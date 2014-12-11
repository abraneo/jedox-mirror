////////////////////////////////////////////////////////////////////////////////
/// @brief
///
/// @file
///
/// Copyright (C) 2006-2014 Jedox AG
///
/// This program is free software; you can redistribute it and/or modify it
/// under the terms of the GNU General Public License (Version 2) as published
/// by the Free Software Foundation at http://www.gnu.org/copyleft/gpl.html.
///
/// This program is distributed in the hope that it will be useful, but WITHOUT
/// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
/// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
/// more details.
///
/// You should have received a copy of the GNU General Public License along with
/// this program; if not, write to the Free Software Foundation, Inc., 59 Temple
/// Place, Suite 330, Boston, MA 02111-1307 USA
/// 
/// If you are developing and distributing open source applications under the
/// GPL License, then you are free to use Palo under the GPL License.  For OEMs,
/// ISVs, and VARs who distribute Palo with their products, and do not license
/// and distribute their source code under the GPL, Jedox provides a flexible
/// OEM Commercial License.
///
/// @author
////////////////////////////////////////////////////////////////////////////////

#define NOMINMAX

#include <algorithm>
#include <functional>
#include <unordered_set>

#include <WTypes.h>
#include <WinNls.h>

#include "../libpalo_ng/source/include/libpalo_ng/Palo/types.h"
#include "../libpalo_ng/source/include/libpalo_ng/Palo/Database.h"
#include "../libpalo_ng/source/include/libpalo_ng/Palo/Dimension.h"
#include "../libpalo_ng/source/include/libpalo_ng/Network/SocketException.h"

#include "libpalo2.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;
using namespace System::Reflection;
using namespace System::Reflection::Emit;

#include <vcclr.h>

#include "Connection.h"

using namespace Jedox::Palo::Comm;

#define ARRAY_MARSHAL_CONVERT(dest_identifier, src_identifier) \
	struct arg_str_array_w dest_identifier; \
	{ \
		dest_identifier.len = (src_identifier)->Length; \
		if (dest_identifier.len > 0) { \
			dest_identifier.a = (wchar_t**)calloc(dest_identifier.len, sizeof(wchar_t*)); \
			if (dest_identifier.a == NULL) { \
				throw gcnew System::OutOfMemoryException(); \
			} \
			for(unsigned long i = 0; i < dest_identifier.len; i++) { \
				dest_identifier.a[i] = reinterpret_cast<wchar_t*>(Marshal::StringToHGlobalUni((src_identifier)[i]).ToPointer()); \
			} \
		} \
	}

#define ARRAY_MARSHAL_CLEANUP(dest_identifier) \
	{ \
		if (dest_identifier.len > 0) { \
			for(unsigned long i = 0; i < dest_identifier.len; i++) { \
				Marshal::FreeHGlobal(IntPtr(dest_identifier.a[i])); \
			} \
			free(dest_identifier.a); \
		} \
	}

#define SIZE_T_TO_MAX_INT(s) (((s)>INT_MAX && (s)!=(size_t)-1) ? INT_MAX : (int)(s))

static const std::string CmpOpLtStr = "%3C";
static const std::string CmpOpLteStr = "%3C%3D";
static const std::string CmpOpGtStr = "%3E";
static const std::string CmpOpGteStr = "%3E%3D";
static const std::string CmpOpEqStr = "%3D%3D";
static const std::string CmpOpNeqStr = "%21%3D";
static const std::string BoolOpAndStr = "%20and%20";
static const std::string BoolOpOrStr = "%20or%20";
static const std::string BoolOpXorStr = "%20or%20";


static void FreeConverted_arg_palo_value_w(struct arg_palo_value_w &pv) {
	if (pv.type == argPaloValueTypeStr) {
		Marshal::FreeHGlobal(IntPtr(pv.val.s));
		pv.val.s = NULL;
	}
}


static struct arg_palo_value_w CellValue2arg_palo_value_w(CellValue &v) {
	struct arg_palo_value_w pv;

	pv.type = argPaloValueTypeError;
	pv.val.err.code = 0;

	if (v.Type == CellValueType::CellValueTypeString) {
		pv.type = argPaloValueTypeStr;
		pv.val.s = reinterpret_cast<wchar_t*>(Marshal::StringToHGlobalUni(v.Value.StrValue).ToPointer());
	} else {
		if (v.Type == CellValueType::CellValueTypeDouble) {
			pv.type = argPaloValueTypeDouble;
			pv.val.d = v.Value.DblValue;
		}
	}

	return pv;
}


static void FreeConverted_arg_str_array_w(struct arg_str_array_w &sa) {
	if ((sa.len > 0) && (sa.a != NULL)) {
		for (unsigned long i = 0; i < sa.len; i++) {
			Marshal::FreeHGlobal(IntPtr(sa.a[i]));
		}
		free(sa.a);
		sa.a = NULL;
		sa.len = 0;
	}
}


static struct arg_str_array_w ManagedStrArray2arg_str_array_w(array<String^>^ a) {
	struct arg_str_array_w sa;

	sa.len = a->Length;
	if (sa.len > 0) {
		sa.a = (wchar_t**)calloc(sa.len, sizeof(wchar_t*));
		if (sa.a == NULL) {
			throw gcnew System::OutOfMemoryException();
		}
	}
	unsigned long i = 0;
	try {
		for (i = 0; i < sa.len; i++) {
			sa.a[i] = reinterpret_cast<wchar_t*>(Marshal::StringToHGlobalUni(a[i]).ToPointer());
		}
	} catch (const Exception^ e) {
		while (i-- > 0) {
			Marshal::FreeHGlobal(IntPtr(sa.a[i]));
		}
		if ((sa.len > 0) && (sa.a != NULL)) {
			free(sa.a);
		}
		throw e;
	}

	return sa;
}


static void FreeConverted_arg_str_array_array_w(struct arg_str_array_array_w &sa) {
	if ((sa.len > 0) && (sa.a != NULL)) {
		for (unsigned long i = 0; i < sa.len; i++) {
			FreeConverted_arg_str_array_w(sa.a[i]);
		}
		free(sa.a);
		sa.a = NULL;
		sa.len = 0;
	}
}


static struct arg_str_array_array_w ManagedStringArrayArray2arg_str_array_array_w(array<array<String^>^>^ a) {
	struct arg_str_array_array_w sa;

	sa.len = a->Length;
	if (sa.len > 0) {
		sa.a = (struct arg_str_array_w*)calloc(sa.len, sizeof(struct arg_str_array_w));
		if (sa.a == NULL) {
			throw gcnew System::OutOfMemoryException();
		}
	} else {
		sa.a = NULL;
	}

	unsigned long i = 0;
	try {
		for (i = 0; i < sa.len; i++) {
			sa.a[i] = ManagedStrArray2arg_str_array_w(Helper::ManagedJaggedArrayMemberToStringArray(a[i]));
		}
	} catch (const Exception^ e) {
		while (i-- > 0) {
			FreeConverted_arg_str_array_w(sa.a[i]);
		}
		if ((sa.len > 0) && (sa.a != NULL)) {
			free(sa.a);
		}
		throw e;
	}

	return sa;
}


static void FreeConverted_arg_str_array_2d_w(struct arg_str_array_2d_w &sa) {
	unsigned long i;
	size_t len = sa.rows * sa.cols;
	if ((len > 0) && (sa.a != NULL)) {
		for (i = 0; i < len; i++) {
			if (sa.a[i] != NULL) {
				Marshal::FreeHGlobal(IntPtr(sa.a[i]));
			}
		}
		free(sa.a);
		sa.a = NULL;
		sa.cols = 0;
		sa.rows = 0;
	}
}


static struct arg_str_array_2d_w ManagedStringArrayArray2arg_str_array_2d_w(array<array<String^>^>^ a) {
	struct arg_str_array_2d_w sa;

	sa.rows = a->Length;
	sa.cols = (sa.rows > 0) ? a[0]->Length : 0;

	unsigned long i, j;
	size_t len =  sa.rows * sa.cols, len2;
	if (len > 0) {
		sa.a = (wchar_t **)calloc(len , sizeof(wchar_t*));
		if (sa.a == NULL) {
			throw gcnew System::OutOfMemoryException();
		}
	} else {
		sa.a = NULL;
	}

	try {
		for (i = 0; i < sa.rows; i++) {
			len2 = a[i]->Length;
			if (len2 > sa.cols) {
				len2 = sa.cols;
			}
			for (j=0; j < len2; j++) {
				sa.a[i*sa.cols+j] =  reinterpret_cast<wchar_t*>(Marshal::StringToHGlobalUni(reinterpret_cast<array<String^>^ >(a[i])[j]).ToPointer());
			}
		}
	} catch (const Exception^ e) {
		FreeConverted_arg_str_array_2d_w(sa);
		throw e;
	}

	return sa;
}


static inline int PositionComparison(ElementInfo^ lhs, ElementInfo^ rhs) {
	return lhs->Position.CompareTo(rhs->Position);
}


inline RuleInfo^ getRuleInfo(arg_rule_info_w &ruleinfo) {
	RuleInfo^ ri = gcnew RuleInfo();

	ri->id = ruleinfo.identifier;
	ri->timestamp = ruleinfo.timestamp;
	ri->definition = gcnew String(ruleinfo.definition);
	ri->extern_id = gcnew String(ruleinfo.extern_id);
	ri->comment = gcnew String(ruleinfo.comment);
	ri->activated = (ruleinfo.activated != 0);
	ri->position = ruleinfo.position;

	return ri;
}


inline RuleInfo^ getRuleInfoAndFree_arg_rule_info_w(arg_rule_info_w &ruleinfo) {
	RuleInfo^ ri = getRuleInfo(ruleinfo);
	free_arg_rule_info_contents_w(&ruleinfo);
	return ri;
};


inline LockInfo^ getLockInfo(arg_lock_info_w &lockinfo) {
	LockInfo^ li = gcnew LockInfo();
	long i, j, size1 = (long)lockinfo.area.len, size2;

	li->id = lockinfo.identifier;
	li->user = gcnew String(lockinfo.user);
	li->steps = lockinfo.steps;

	li->area = gcnew array<array<String^>^>(size1);
	for (i=0; i < size1; i++) {
		size2 = (long)lockinfo.area.a[i].len;
		li->area[i] = gcnew array<String^>(size2);
		for (j=0; j < size2; j++) {
			li->area[i][j] = gcnew String(lockinfo.area.a[i].a[j]);
		}
	}
	return li;
};


// Start Export related
// for direct libpalo_ng version
void arg_add2ConditionString(const std::string& BoolOpStr, const CompareOp& Cmp, const CellValue& Val, std::string& Condition) {
	std::string CmpOp;

	switch (Cmp) {
		case CompareOp::CompareOpGT:
			// >
			CmpOp = CmpOpGtStr;
			break;

		case CompareOp::CompareOpLT:
			// <
			CmpOp = CmpOpLtStr;
			break;

		case CompareOp::CompareOpGTE:
			// >=
			CmpOp = CmpOpGteStr;
			break;

		case CompareOp::CompareOpLTE:
			// <=
			CmpOp = CmpOpLteStr;
			break;

		case CompareOp::CompareOpEQ:
			// ==
			CmpOp = CmpOpEqStr;
			break;

		case CompareOp::CompareOpNEQ:
			// !=
			CmpOp = CmpOpNeqStr;
			break;
	}

	if (!CmpOp.empty()) {
		std::stringstream helper;
		if (Val.Type == CellValueType::CellValueTypeString) {
			std::string strvalue;
			Helper::ConvertStringToUTF8(strvalue, Val.Value.StrValue);
			helper << jedox::util::StringUtils::URLencode(jedox::util::StringUtils::CSVencode(strvalue));
		} else {
			helper.setf(std::ios_base::fixed , std::ios_base::floatfield);
			helper.precision(PRECISION);

			helper << Val.Value.DblValue;
		}

		Condition += BoolOpStr;
		Condition += CmpOp;
		Condition += helper.str();
	}
}


void arg_getConditionString(const ValueFilter& Filter, std::string& Condition) {
	std::string BoolOpStr;

	arg_add2ConditionString(BoolOpStr, Filter.CmpOp1, Filter.Value1, Condition);

	if (!Condition.empty() && (Filter.CmpOp1 != CompareOp::CompareOpTRUE)) {

		switch (Filter.AndOr12) {
			case BoolOp::BoolOpAND:
				BoolOpStr = BoolOpAndStr;
				break;

			case BoolOp::BoolOpOR:
				BoolOpStr = BoolOpOrStr;
				break;

			case BoolOp::BoolOpXOR:
				BoolOpStr = BoolOpXorStr;
				break;

			default:
				break;
		}
	}

	arg_add2ConditionString(BoolOpStr, Filter.CmpOp2, Filter.Value2, Condition);
}
// End Export related


// Start Connection Methods
void Connection::throwException(libpalo_err err) {
	throw gcnew PaloException(err);
}


void Connection::throwException(String^ s) {
	throw gcnew PaloCommException(s);
}


void Connection::DoInit(String^ hostname, String^ service, String^ username, String^ password, String^ trust_file) {
	unsigned short isgpu = 0;
	so = (struct sock_obj*)calloc(1, sizeof(struct sock_obj));
	if (so == NULL) {
		throw gcnew System::OutOfMemoryException();
	}

	libpalo_err err;

	if (!LIBPALO_SUCCESSFUL(libpalo_init_r(&err, NULL))) {
		throwException(err);
	}

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> phostname = PtrToStringChars(hostname);
	pin_ptr<const wchar_t> pservice = PtrToStringChars(service);

	if (!LIBPALO_SUCCESSFUL(palo_connect_w_r(&err, NULL, phostname, pservice, pso))) {
		throwException(err);
	}

	pin_ptr<const wchar_t> pusername = PtrToStringChars(username);
	pin_ptr<const wchar_t> ppassword = PtrToStringChars(password);
	pin_ptr<const wchar_t> ptrustfile = PtrToStringChars(trust_file);

	if (!LIBPALO_SUCCESSFUL(palo_auth_ssl_w_r(&err, pso, NULL, pusername, ppassword, ptrustfile))) {
		palo_disconnect2(so, 1);
		throwException(err);
	}

	palo_is_gpu_server_w_r(&err, &isgpu, pso, NULL);

	IsGpu = (isgpu != 0);
}


Connection::Connection(String^ hostname, String^ service, String^ username, String^ password) {
	DoInit(hostname, service, username, password, TrustFile);
}


Connection::Connection(String^ hostname, String^ service, String^ username, String^ password, String^ trust_file) {
	DoInit(hostname, service, username, password, trust_file);
}


Connection::~Connection(void) {
	pin_ptr<struct sock_obj> pso = so;
	palo_disconnect2(pso, 1);
	free(so);
	so = NULL;
}


// Begin Lib related
ApiInfo Connection::ApiInformation() {
	libpalo_err err;
	struct api_info apiinfo;
	ApiInfo ai;

	if (!LIBPALO_SUCCESSFUL(palo_api_version_w_r(&err, &apiinfo))) {
		throwException(err);
	}

	ai.MajorVersion = apiinfo.major_version;
	ai.MinorVersion = apiinfo.minor_version;
	ai.BugfixVersion = apiinfo.bugfix_version;
	ai.BuildNumber = apiinfo.build_number;

	return ai;
}


void Connection::SetClientVersion(String^ client_version) {
	libpalo_err err;

	pin_ptr<const wchar_t> pclient_version = PtrToStringChars(client_version);

	if (!LIBPALO_SUCCESSFUL(palo_set_client_version_w_r(&err, NULL, pclient_version))) {
		throwException(err);
	}
}


void Connection::InitSSL(String^ argTrustFile) {
	TrustFile = argTrustFile;
	
	libpalo_err err;

	pin_ptr<const wchar_t> ptrustfile = PtrToStringChars(TrustFile);

	if (!LIBPALO_SUCCESSFUL(palo_init_ssl_w(&err, ptrustfile))) {
		throwException(err);
	}
}
// End Lib related


// Start Server related
void Connection::TestConnection(String^ hostname, unsigned int port, String^ username, String^ pw_hash, String^ trust_file) {
	libpalo_err err;

	pin_ptr<const wchar_t> phostname = PtrToStringChars(hostname);
	pin_ptr<const wchar_t> pusername = PtrToStringChars(username);
	pin_ptr<const wchar_t> ppw_hash = PtrToStringChars(pw_hash);
	pin_ptr<const wchar_t> ptrustfile = PtrToStringChars(trust_file);

	if ( !LIBPALO_SUCCESSFUL( palo_test_connection_w_r(&err, NULL, phostname, port, pusername, ppw_hash, ptrustfile ) ) ) {
		throwException( err );
	}
}


void Connection::TestConnection(String^ hostname, unsigned int port, String^ username, String^ pw_hash) {
	TestConnection(hostname, port, username, pw_hash, TrustFile);
}


void Connection::CheckValidity() {
	libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;

	if (!LIBPALO_SUCCESSFUL(palo_check_validity_w_r(&err, pso, NULL))) {
		throwException(err);
	}
}


void Connection::PrepareLogout() {
}


bool Connection::IsAdminUser() {
	libpalo_err err;
	unsigned short result;

	pin_ptr<struct sock_obj> pso = so;

	if (!LIBPALO_SUCCESSFUL(palo_is_admin_user_w_r(&err, &result, pso, NULL))) {
		throwException(err);
	}

	return (result != 0);
}


bool Connection::IsGpuServer() {
	return IsGpu;
}


ServerInfo Connection::ServerInformation() {
	libpalo_err err;
	struct server_info srvinfo;
	ServerInfo si;

	pin_ptr<struct sock_obj> pso = so;

	if (!LIBPALO_SUCCESSFUL(palo_server_info_w_r(&err, pso, NULL, &srvinfo))) {
		throwException(err);
	}

	si.MajorVersion = srvinfo.major_version;
	si.MinorVersion = srvinfo.minor_version;
	si.BugfixVersion = srvinfo.bugfix_version;
	si.BuildNumber = srvinfo.build_number;
	si.Encryption = srvinfo.encryption;
	si.HttpsPort = srvinfo.https_port;
	si.DataSequenceNumber = srvinfo.data_sequence_number;

	return si;
}


SuperVisionServerInfo Connection::SVSInformation(){
    libpalo_err err;
	struct arg_svs_info_w result;
	SuperVisionServerInfo svsinfo;

	pin_ptr<struct sock_obj> pso = so;

	palo_get_svs_info_w_r(&err, &result, pso, NULL);

    if (!LIBPALO_SUCCESSFUL(err.result)) {
		throwException(err);
    }

	svsinfo.CubeWorkerActive = (result.cube_worker_active != 0);
	svsinfo.DrillThroughEnabled = (result.drill_through_enabled != 0);
	svsinfo.LoginMode = SVSLoginMode(result.login_mode);
	svsinfo.SVSActive = (result.svs_active != 0);
	svsinfo.WindowsSsoEnabled = (result.windows_sso_enabled != 0);

	return svsinfo;
}


void Connection::Ping() {
	libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;

	if (!LIBPALO_SUCCESSFUL(palo_ping_w_r(&err, pso, NULL))) {
		throwException(err);
	}
}


void Connection::ServerShutdown() {
	libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;

	if (!LIBPALO_SUCCESSFUL(palo_server_shutdown_w_r(&err, pso, NULL))) {
		throwException(err);
	}
}


void Connection::SetUserPassword(String^ user, String^ password) {
	libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> puser = PtrToStringChars(user);
	pin_ptr<const wchar_t> ppassword = PtrToStringChars(password);

	if (!LIBPALO_SUCCESSFUL(palo_set_user_password_w_r(&err, pso, NULL, puser, ppassword))) {
		throwException(err);
	}
}


void Connection::ChangePassword(String^ oldpassword, String^ newpassword) {
	libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> poldpassword = PtrToStringChars(oldpassword);
	pin_ptr<const wchar_t> pnewpassword = PtrToStringChars(newpassword);

	if (!LIBPALO_SUCCESSFUL(palo_change_password_w_r(&err, pso, NULL, poldpassword, pnewpassword))) {
		throwException(err);
	}
}


String^ Connection::ListRuleFunctions() {
	libpalo_err err;
	wchar_t *functions;

	pin_ptr<struct sock_obj> pso = so;

	palo_list_rulefunctions_w_r(&err, &functions, pso, NULL);

	if (!LIBPALO_SUCCESSFUL(err.result)) {
		throwException(err);
	}

	return Helper::CharPtrToStringFree(functions);
};


PermissionType Connection::GetRights(PermissionArt permissionart) {
	libpalo_err err;
	permission_type result;

	pin_ptr<struct sock_obj> pso = so;

	palo_get_rights_w_r(&err, &result, pso, NULL, (permission_art)permissionart);

	if (!LIBPALO_SUCCESSFUL(err.result)) {
		throwException(err);
	}

	return (PermissionType)result;
};
// End Server related


// Start Database related
void Connection::DatabaseLoad(String^ database) {
	libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);

	if (!LIBPALO_SUCCESSFUL(palo_database_load_w_r(&err, pso, NULL, pdatabase))) {
		throwException(err);
	}
}


void Connection::UnloadDatabase(String^ database) {
	libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);

	if (!LIBPALO_SUCCESSFUL(palo_database_unload_w_r(&err, pso, NULL, pdatabase))) {
		throwException(err);
	}
}


void Connection::DatabaseSave(String^ database) {
	libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);

	if (!LIBPALO_SUCCESSFUL(palo_database_save_w_r(&err, pso, NULL, pdatabase))) {
		throwException(err);
	}
}


void Connection::AddDatabase(String^ database) {
	libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);

	if (!LIBPALO_SUCCESSFUL(palo_root_add_database_w_r(&err, pso, NULL, pdatabase))) {
		throwException(err);
	}
}

void Connection::DeleteDatabase(String^ database) {
	libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);

	if (!LIBPALO_SUCCESSFUL(palo_database_delete_w_r(&err, pso, NULL, pdatabase))) {
		throwException(err);
	}
}


array<String^>^ Connection::RootListDatabases() {
    libpalo_err err;
    struct arg_str_array_w databases;

	pin_ptr<struct sock_obj> pso = so;

    if (!LIBPALO_SUCCESSFUL(palo_root_list_databases_w_r(&err, &databases, pso, NULL))) {
		throwException(err);
    }

    return Helper::ArgStringArrayToManagedFree(databases);
}


array<String^>^ Connection::RootListDatabases(DatabaseType Type) {
    libpalo_err err;
    struct arg_str_array_w databases;

	pin_ptr<struct sock_obj> pso = so;

    if (!LIBPALO_SUCCESSFUL(palo_root_list_databases2_w_r(&err, &databases, pso, NULL, (unsigned int)Type))) {
		throwException(err);
    }

    return Helper::ArgStringArrayToManagedFree(databases);
}


DatabaseType Connection::GetDatabaseTypeHelper(unsigned long type) {
	DatabaseType retval = DatabaseType::NormalDatabase;

	switch (type) {
		case normal_db:
			retval = DatabaseType::NormalDatabase;
			break;

		case system_db:
			retval = DatabaseType::SystemDatabase;
			break;

		case user_info_db:
			retval  = DatabaseType::UserinfoDatabase;
			break;

		case unknown_db_type:
			retval  = DatabaseType::UnknownDatabaseType;
			break;
	}

	return retval;

}


DatabaseType Connection::GetDatabaseType(String^ database) {
	libpalo_err err;
	unsigned int result = normal_db;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);

	if (!LIBPALO_SUCCESSFUL(palo_get_database_type_w_r(&err, &result, pso, NULL, pdatabase))) {
		throwException(err);
	}

	return GetDatabaseTypeHelper(result);
}


DatabaseInfo Connection::DatabaseInformation(String^ database) {
	libpalo_err err;
	struct arg_db_info_w dbinfo;
	DatabaseInfo di;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);

	if (!LIBPALO_SUCCESSFUL(palo_database_info_w_r(&err, &dbinfo, pso, NULL, pdatabase))) {
		throwException(err);
	}

	di.id = dbinfo.id;
	di.Name = gcnew String(dbinfo.name);
	di.NumberDimensions = dbinfo.number_dimensions;
	di.NumberCubes = dbinfo.number_cubes;

	switch (dbinfo.status) {
		case unloaded_db:
			di.Status = DatabaseStatus::UnloadedDatabase;
			break;

		case loaded_db:
			di.Status = DatabaseStatus::LoadedDatabase;
			break;

		case changed_db:
			di.Status = DatabaseStatus::ChangedDatabase;
			break;

		case loading_db:
			di.Status = DatabaseStatus::LoadingDatabase;
			break;

		case unknown_db_status:
			di.Status = DatabaseStatus::UnknownDatabaseStatus;
			break;
	}

	di.Type = GetDatabaseTypeHelper(dbinfo.type);

	free_arg_db_info_contents_w(&dbinfo);

	return di;
}
// End Database related


// Start Dimension related
String^ Connection::GetDimensionNameFromID(String^ database, long id) {
    ErrorInformation^ ErrorInfo = gcnew ErrorInformation();
    String^ dimension = nullptr;

	pin_ptr<struct sock_obj> pso = so;
	std::string sdatabase;

	Helper::ConvertStringToUTF8(sdatabase, database);

	try {
		dimension = Helper::ConvertUTF8ToString((*(pso->myServer))[sdatabase].dimension[id].getCacheData().ndimension);
	} catch (const SocketException& exp) {
		ErrorInfo->ProcessSocketError(exp.what());
	} catch (const PaloServerException& exp) {
		ErrorInfo->ProcessError(exp.code(), exp.Description());
	} catch (const std::exception& exp) {
		ErrorInfo->ProcessStandardError(exp.what());
	} catch (const System::Exception^) {
		throw;
#ifndef _DEBUG
	} catch (...) {
		ErrorInfo->ProcessUnknownError();
#endif
	}

	ErrorInfo->Check();

    return dimension;
}


void Connection::DatabaseAddDimension(String^ database, String^ dimension, DimensionType type) {
	libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);

	if (!LIBPALO_SUCCESSFUL(palo_database_add_dimension2_w_r(&err, pso, NULL, pdatabase, pdimension, (dim_type)type))) {
		throwException(err);
	}
}

void Connection::DatabaseAddDimension(String^ database, String^ dimension) {
	DatabaseAddDimension(database, dimension, DimensionType::NormalDimension);
}


void Connection::DatabaseRenameDimension(String^ database, String^ dimension_oldname, String^ dimension_newname) {
	libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pdimension_oldname = PtrToStringChars(dimension_oldname);
	pin_ptr<const wchar_t> pdimension_newname = PtrToStringChars(dimension_newname);

	if (!LIBPALO_SUCCESSFUL(palo_database_rename_dimension_w_r(&err, pso, NULL, pdatabase, pdimension_oldname, pdimension_newname))) {
		throwException(err);
	}
}


void Connection::DeleteDimension(String^ database, String^ dimension) {
	libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);

	if (!LIBPALO_SUCCESSFUL(palo_dimension_delete_w_r(&err, pso, NULL, pdatabase, pdimension))) {
		throwException(err);
	}
}


void Connection::ClearDimension(String^ database, String^ dimension) {
	libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);

	if (!LIBPALO_SUCCESSFUL(palo_dimension_clear_w_r(&err, pso, NULL, pdatabase, pdimension))) {
		throwException(err);
	}
}


DimensionType Connection::GetDimensionTypeHelper(unsigned long type) {
	DimensionType retval = DimensionType:: UnknownDimensionType;

	switch (type) {
		case normal_dim:
			retval = DimensionType::NormalDimension;
			break;

		case system_dim:
			retval = DimensionType::SystemDimension;
			break;

		case attribut_dim:
			retval = DimensionType::AttributeDimension;
			break;

		case user_info_dim:
			retval = DimensionType::UserInfoDimension;
			break;

		case system_id_dim:
			retval = DimensionType::SystemIdDimension;
			break;

	}

	return retval;
}


DimensionType Connection::GetDimensionType(String^ database, String^ dimension) {
	libpalo_err err;
	unsigned int result = normal_dim;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);

	if (!LIBPALO_SUCCESSFUL(palo_get_dimension_type_w_r(&err, &result, pso, NULL, pdatabase, pdimension))) {
		throwException(err);
	}

	return GetDimensionTypeHelper(result);
}


DimensionInfoSimple Connection::DimensionInformationSimple(String^ database, String^ dimension) {
	libpalo_err err;
	struct arg_dim_info_simple_w dimensioninfo;

	DimensionInfoSimple di;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);

	if (!LIBPALO_SUCCESSFUL(palo_dimension_info_simple_w_r(&err, &dimensioninfo, pso, NULL, pdatabase, pdimension))) {
		throwException(err);
	}

	di.id = dimensioninfo.id;
	di.Name = gcnew String(dimensioninfo.name);
	di.AssocDimensionId = dimensioninfo.assoc_dimension_id;
	di.AttributeCubeId = dimensioninfo.attribut_cube_id;
	di.RightsCubeId = dimensioninfo.rights_cube_id;
	di.NumberElements = dimensioninfo.number_elements;
	di.MaximumLevel = dimensioninfo.maximum_level;
	di.MaximumIndent = dimensioninfo.maximum_indent;
	di.MaximumDepth = dimensioninfo.maximum_depth;

	di.Type = GetDimensionTypeHelper(dimensioninfo.type);

	free_arg_dim_info_simple_contents_w(&dimensioninfo);

	return di;
}


String^ Connection::GetAttributeDimension(String^ database, String^ dimension) {
	libpalo_err err;
	wchar_t *result;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);

	if (!LIBPALO_SUCCESSFUL(palo_get_attribute_dimension_w_r(&err, &result, pso, NULL, pdatabase, pdimension))) {
		throwException(err);
	}

	return Helper::CharPtrToStringFree(result);
}


String^ Connection::ElementDimension(String^ database, array<String^>^ dimension_elements, bool should_be_unique) {
	wchar_t *str;
	libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);

	ARRAY_MARSHAL_CONVERT(adimension_elements, dimension_elements)
	palo_edimension_w_r(&err, &str, pso, NULL, pdatabase, adimension_elements, should_be_unique ? PALO_TRUE : PALO_FALSE);
	ARRAY_MARSHAL_CLEANUP(adimension_elements)

	if (!LIBPALO_SUCCESSFUL(err.result)) {
		throwException(err);
	}

	return Helper::CharPtrToStringFree(str);
}


String^ Connection::ElementCubeDimension(String^ database, String^ cube, array<String^>^ dimension_elements, bool should_be_unique) {
	wchar_t *str;
	libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pcube = PtrToStringChars(cube);

	ARRAY_MARSHAL_CONVERT(adimension_elements, dimension_elements)
	palo_ecubedimension_w_r(&err, &str, pso, NULL, pdatabase, pcube, adimension_elements, should_be_unique ? PALO_TRUE : PALO_FALSE);
	ARRAY_MARSHAL_CLEANUP(adimension_elements)

	if (!LIBPALO_SUCCESSFUL(err.result)) {
		throwException(err);
	}

	return Helper::CharPtrToStringFree(str);
}


array<String^>^ Connection::DatabaseListDimensions(String^ database) {
    libpalo_err err;
    struct arg_str_array_w result;

	pin_ptr<struct sock_obj> pso = so;
    pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);

    if (!LIBPALO_SUCCESSFUL(palo_database_list_dimensions_w_r(&err, &result, pso, NULL, pdatabase))) {
		throwException(err);
    }

    return Helper::ArgStringArrayToManagedFree(result);
}


array<String^>^ Connection::DatabaseListDimensions(String^ database, DimensionType Type) {
    libpalo_err err;
    struct arg_str_array_w result;

	pin_ptr<struct sock_obj> pso = so;
    pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);

    if (!LIBPALO_SUCCESSFUL(palo_database_list_dimensions2_w_r(&err, &result, pso, NULL, pdatabase, (unsigned int)Type))) {
		throwException(err);
    }

    return Helper::ArgStringArrayToManagedFree(result);
}
// End Dimension related


// Start Element related
String^ Connection::GetElementNameFromID(String^ database, String^ dimension, long id) {
    ErrorInformation^ ErrorInfo = gcnew ErrorInformation();
    String^ element = nullptr;

	pin_ptr<struct sock_obj> pso = so;
	std::string sdatabase, sdimension;

	Helper::ConvertStringToUTF8(sdatabase, database);
	Helper::ConvertStringToUTF8(sdimension, dimension);

	try {
		element = Helper::ConvertUTF8ToString((*(pso->myServer))[sdatabase].dimension[sdimension][id].getCacheData().nelement);
	} catch (const SocketException& exp) {
		ErrorInfo->ProcessSocketError(exp.what());
	} catch (const PaloServerException& exp) {
		ErrorInfo->ProcessError(exp.code(), exp.Description());
	} catch (const std::exception& exp) {
		ErrorInfo->ProcessStandardError(exp.what());
	} catch (const System::Exception^) {
		throw;
#ifndef _DEBUG
	} catch (...) {
		ErrorInfo->ProcessUnknownError();
#endif
	}

	ErrorInfo->Check();

    return element;
}


DimElementType Connection::GetElementTypeFromID(String^database, String^ dimension, long id) {
    ErrorInformation^ ErrorInfo = gcnew ErrorInformation();
    DimElementType type;

    pin_ptr<struct sock_obj>pso = so;
	std::string sdatabase, sdimension;

	Helper::ConvertStringToUTF8(sdatabase, database);
	Helper::ConvertStringToUTF8(sdimension, dimension);

	try {
		type = (DimElementType)(*(pso->myServer))[sdatabase].dimension[sdimension][id].getCacheData().type;
	} catch (const SocketException& exp) {
		ErrorInfo->ProcessSocketError(exp.what());
	} catch (const PaloServerException& exp) {
		ErrorInfo->ProcessError(exp.code(), exp.Description());
	} catch (const std::exception& exp) {
		ErrorInfo->ProcessStandardError(exp.what());
	} catch (const System::Exception^) {
		throw;
#ifndef _DEBUG
	} catch (...) {
		ErrorInfo->ProcessUnknownError();
#endif
	}

	ErrorInfo->Check();

    return type;
}


unsigned int Connection::GetElementPositionFromID(String^ database, String^ dimension, long id) {
	ErrorInformation^ ErrorInfo = gcnew ErrorInformation();
	unsigned int position = 0;

	pin_ptr<struct sock_obj> pso = so;

	std::string sdatabase, sdimension;

	Helper::ConvertStringToUTF8(sdatabase, database);
	Helper::ConvertStringToUTF8(sdimension, dimension);

	try {
		position = (*(pso->myServer))[sdatabase].dimension[sdimension][id].getCacheData().position;
	} catch (const SocketException& exp) {
		ErrorInfo->ProcessSocketError(exp.what());
	} catch (const PaloServerException& exp) {
		ErrorInfo->ProcessError(exp.code(), exp.Description());
	} catch (const std::exception& exp) {
		ErrorInfo->ProcessStandardError(exp.what());
	} catch (const System::Exception^) {
		throw;
#ifndef _DEBUG
	} catch (...) {
		ErrorInfo->ProcessUnknownError();
#endif
	}

	ErrorInfo->Check();

	return position;
}

long Connection::GetElementIDFromName(String^ database, String^ dimension, String^ element) {
    ErrorInformation^ ErrorInfo = gcnew ErrorInformation();
    long id = -1;

	pin_ptr<struct sock_obj> pso = so;
	std::string sdatabase, sdimension, selement;

	Helper::ConvertStringToUTF8(sdatabase, database);
	Helper::ConvertStringToUTF8(sdimension, dimension);
	Helper::ConvertStringToUTF8(selement, element);

	try {
		id = (*(pso->myServer))[sdatabase].dimension[sdimension][selement].getCacheData().element;
	} catch (const SocketException& exp) {
		ErrorInfo->ProcessSocketError(exp.what());
	} catch (const PaloServerException& exp) {
		ErrorInfo->ProcessError(exp.code(), exp.Description());
	} catch (const std::exception& exp) {
		ErrorInfo->ProcessStandardError(exp.what());
	} catch (const System::Exception^) {
		throw;
#ifndef _DEBUG
	} catch (...) {
		ErrorInfo->ProcessUnknownError();
#endif
	}

	ErrorInfo->Check();

    return id;
}


DimElementType Connection::getType(de_type type) {
	DimElementType Type = DimElementType::DimElementTypeRule;

	switch (type) {
		case de_n:
			Type = DimElementType::DimElementTypeNumeric;
			break;

		case de_s:
			Type = DimElementType::DimElementTypeString;
			break;

		case de_c:
			Type = DimElementType::DimElementTypeConsolidated;
			break;

		case de_r:
			Type = DimElementType::DimElementTypeRule;
			break;

		default:
			throwException("Unknown dimension element type");
	}

	return Type;

}


void Connection::FillElementInfo(ElementInfo^ ei, arg_dim_element_info2_raw_w &elementinfo) {
    long j, len;

	if (elementinfo.name != NULL) {
		ei->Identifier = elementinfo.identifier;
		ei->Name = gcnew String(elementinfo.name);
	} else {
		ei->Identifier = -1;
		ei->Name = nullptr;
	}

	ei->Type = getType(elementinfo.type);
	ei->Position = elementinfo.position;
	ei->Level = elementinfo.level;
	ei->Indent = elementinfo.indent;
	ei->Depth = elementinfo.depth;
	ei->NumberParents = elementinfo.num_parents;
	len = (long)ei->NumberParents; 
	ei->Parents = gcnew array<ParentInfoRaw>(len);

	for (j = 0;j < len; j++) {
		ei->Parents[j].Identifier = elementinfo.parents[j].identifier;
	}

	ei->NumberChildren = elementinfo.num_children;
	len = (long)ei->NumberChildren; 
	ei->Children = gcnew array<ChildInfoRaw>(len);

	for (j = 0;j < len; j++) {
		ei->Children[j].Identifier = elementinfo.children[j].identifier;
		ei->Children[j].Factor = elementinfo.children[j].factor;
	}
}


ElementInfo^ Connection::ElementCreate(String^ database, String^ dimension, String^ element, DimElementType type, array<ConsolidationInfo>^ ci) {
	ElementInfo^ ei;
	libpalo_err err;

	struct arg_dim_element_info2_raw_w result;

	struct arg_consolidation_element_info_array_w ceia;
	ceia.len = ci->Length;
	ceia.a = (struct arg_consolidation_element_info_w*)calloc(ceia.len, sizeof(struct arg_consolidation_element_info_w));

	for (long i = 0; i < ci->Length; i++) {
		ceia.a[i].factor = ci[i].Factor;
		ceia.a[i].name = reinterpret_cast<wchar_t*>(Marshal::StringToHGlobalUni(ci[i].Name).ToPointer());
	}

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);
	pin_ptr<const wchar_t> pelement = PtrToStringChars(element);

	palo_element_create_w_r(&err, &result, pso, NULL, pdatabase, pdimension, pelement, (de_type)type, &ceia);

	for (long i = 0; i < ci->Length; i++) {
		Marshal::FreeHGlobal(IntPtr(ceia.a[i].name));
	}

	free(ceia.a);

	if (!LIBPALO_SUCCESSFUL(err.result)) {
		throwException(err);
	}

	ei = gcnew ElementInfo();

	FillElementInfo(ei, result);

	free_arg_dim_element_info2_raw_contents_w(&result);

	return ei;
}


ElementInfo^ Connection::ElementCreate(String^ database, String^ dimension, String^ element) {
	return ElementCreate(database, dimension, element, DimElementType::DimElementTypeNumeric, gcnew array<ConsolidationInfo>(0));
}

void Connection::ElementCreateMulti(String^ database, String^ dimension, array<String^>^ elements , int type, array<array<String^>^>^ children, array<array<double>^>^ weights) {
	ErrorInformation^ ErrorInfo = gcnew ErrorInformation();
	
	pin_ptr<struct sock_obj> pso = so;
	std::string sdatabase, sdimension;
	std::vector<std::string> selements;
	std::vector<std::vector<std::string>> schildren;
	std::vector < std::vector<double> > sweights;
	
	Helper::ConvertStringToUTF8(sdatabase, database);
	Helper::ConvertStringToUTF8(sdimension, dimension);
	Helper::ConvertStringArrayToUTF8(selements, elements);
	Helper::ConvertStringArrayArrayToUTF8(schildren,children);
	Helper::ConvertDArrayArrayToDVector(sweights, weights);
	
	try {
		(*(pso->myServer))[sdatabase].dimension[sdimension].bulkCreateElements(selements, (ELEMENT_INFO::TYPE)type, schildren, sweights);
	} catch (const SocketException& exp) {
		ErrorInfo->ProcessSocketError(exp.what());
	} catch (const PaloServerException& exp) {
		ErrorInfo->ProcessError(exp.code(), exp.Description());
	} catch (const std::exception& exp) {
		ErrorInfo->ProcessStandardError(exp.what());
	} catch (const System::Exception^) {
		throw;
#ifndef _DEBUG
	} catch (...) {
		ErrorInfo->ProcessUnknownError();
#endif
	}

	ErrorInfo->Check();
}


bool Connection::ElementExists(String^ database, String^ dimension, String^ element)
{
    ErrorInformation^ ErrorInfo = gcnew ErrorInformation();
    bool exists = false;


	pin_ptr<struct sock_obj> pso = so;
	std::string sdatabase, sdimension, selement;

	Helper::ConvertStringToUTF8(sdatabase, database);
	Helper::ConvertStringToUTF8(sdimension, dimension);
	Helper::ConvertStringToUTF8(selement, element);

	try {
		exists = (*(pso->myServer ))[sdatabase].dimension[sdimension].Exists(selement);
	} catch ( const SocketException& exp ) {
		ErrorInfo->ProcessSocketError(exp.what());
	} catch ( const PaloServerException& exp ) {
		ErrorInfo->ProcessError(exp.code(), exp.Description());
	} catch ( const std::exception& exp ) {
		ErrorInfo->ProcessStandardError(exp.what());
	} catch ( const System::Exception^ ) {
		throw;
#ifndef _DEBUG
	} catch (...) {
		ErrorInfo->ProcessUnknownError();
#endif
	}

	ErrorInfo->Check();

    return exists;
}

void Connection::DimensionAddOrUpdateElement(String^ database, String^ dimension, String^ element, AddOrUpdateElementMode mode, DimElementType type, array<ConsolidationInfo>^ ci, bool append_c) {
	libpalo_err err;

	struct arg_consolidation_element_info_array_w ceia;
	ceia.len = ci->Length;
	ceia.a = (struct arg_consolidation_element_info_w*)calloc(ceia.len, sizeof(struct arg_consolidation_element_info_w));

	for (long i = 0; i < ci->Length; i++) {
		ceia.a[i].factor = ci[i].Factor;
		ceia.a[i].name = reinterpret_cast<wchar_t*>(Marshal::StringToHGlobalUni(ci[i].Name).ToPointer());
	}

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);
	pin_ptr<const wchar_t> pelement = PtrToStringChars(element);

	palo_eadd_or_update_w_r(&err, pso, NULL, pdatabase, pdimension, pelement, (dimension_add_or_update_element_mode)mode, (de_type)type, ceia, append_c == true ? PALO_TRUE : PALO_FALSE);

	for (long i = 0; i < ci->Length; i++) {
		Marshal::FreeHGlobal(IntPtr(ceia.a[i].name));
	}

	free(ceia.a);

	if (!LIBPALO_SUCCESSFUL(err.result)) {
		throwException(err);
	}
}


void Connection::DimensionAddOrUpdateElement(String^ database, String^ dimension, String^ element, DimElementType type, array<ConsolidationInfo>^ ci) {
	DimensionAddOrUpdateElement(database, dimension, element, AddOrUpdateElementMode::AddOrUpdateElementModeAddOrUpdate, type, ci, false);
}


void Connection::DimensionAddOrUpdateElement(String^ database, String^ dimension, String^ element, AddOrUpdateElementMode mode, DimElementType type) {
	DimensionAddOrUpdateElement(database, dimension, element, mode, type, gcnew array<ConsolidationInfo>(0) , false);
}


void Connection::DimensionAddOrUpdateElement(String^ database, String^ dimension, String^ element) {
	DimensionAddOrUpdateElement(database, dimension, element, DimElementType::DimElementTypeNumeric, gcnew array<ConsolidationInfo>(0));
}


void Connection::DimElementRename(String^ database, String^ dimension, String^ de_oldname, String^ de_newname) {
	libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);
	pin_ptr<const wchar_t> pde_oldname = PtrToStringChars(de_oldname);
	pin_ptr<const wchar_t> pde_newname = PtrToStringChars(de_newname);

	if (!LIBPALO_SUCCESSFUL(palo_erename_w_r(&err, pso, NULL, pdatabase, pdimension, pde_oldname, pde_newname))) {
		throwException(err);
	}
}


void Connection::DimElementDelete(String^ database, String^ dimension, String^ dimension_element) {
	libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);
	pin_ptr<const wchar_t> pdimension_element = PtrToStringChars(dimension_element);

	if (!LIBPALO_SUCCESSFUL(palo_edelete_w_r(&err, pso, NULL, pdatabase, pdimension, pdimension_element))) {
		throwException(err);
	}
}


// use libpalo_ng directly
void Connection::DimElementDeleteMulti(String^ database, String^ dimension, array<String^>^ elements) {
    ErrorInformation^ ErrorInfo = gcnew ErrorInformation();

	pin_ptr<struct sock_obj> pso = so;
	std::string sdatabase, sdimension;
	std::vector<std::string> selements;

	Helper::ConvertStringToUTF8(sdatabase, database);
	Helper::ConvertStringToUTF8(sdimension, dimension);
	Helper::ConvertStringArrayToUTF8(selements, elements);

	try {
		(*(pso->myServer))[sdatabase].dimension[sdimension].bulkDeleteElements(selements);
	} catch (const SocketException& exp) {
		ErrorInfo->ProcessSocketError(exp.what());
	} catch (const PaloServerException& exp) {
		ErrorInfo->ProcessError(exp.code(), exp.Description());
	} catch (const std::exception& exp) {
		ErrorInfo->ProcessStandardError(exp.what());
	} catch (const System::Exception^) {
		throw;
#ifndef _DEBUG
	} catch (...) {
		ErrorInfo->ProcessUnknownError();
#endif
	}

	ErrorInfo->Check();
}


void Connection::DimElementMove(String^ database, String^ dimension, String^ dimension_element, unsigned long new_position) {
	libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);
	pin_ptr<const wchar_t> pdimension_element = PtrToStringChars(dimension_element);

	if (!LIBPALO_SUCCESSFUL(palo_emove_w_r(&err, pso, NULL, pdatabase, pdimension, pdimension_element, new_position))) {
		throwException(err);
	}
}


DimElementType Connection::ElementType(String^ database, String^ dimension, String^ dimension_element) {
	libpalo_err err;
	de_type result;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);
	pin_ptr<const wchar_t> pdimension_element = PtrToStringChars(dimension_element);

	if (!LIBPALO_SUCCESSFUL(palo_etype_w_r(&err, &result, pso, NULL, pdatabase, pdimension, pdimension_element))) {
		throwException(err);
	}

	return getType(result);
}


ElementInfo^ Connection::ElementInformationSimple(String^ database, String^ dimension, String^ element) {
	libpalo_err err;
	struct arg_dim_element_info2_raw_w elementinfo;

	ElementInfo^ ei;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);
	pin_ptr<const wchar_t> pelement = PtrToStringChars(element);

	if (!LIBPALO_SUCCESSFUL(palo_element_info_simple_w_r(&err, &elementinfo, pso, NULL, pdatabase, pdimension, pelement))) {
		throwException(err);
	}

	ei = gcnew ElementInfo();

	FillElementInfo(ei, elementinfo);

	free_arg_dim_element_info2_raw_contents_w(&elementinfo);

	return ei;
}


String^ Connection::ElementName(String^ database, String^ dimension, unsigned long n) {
	libpalo_err err;
	wchar_t *result;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);

	if (!LIBPALO_SUCCESSFUL(palo_ename_w_r(&err, &result, pso, NULL, pdatabase, pdimension, n))) {
		throwException(err);
	}

	return Helper::CharPtrToStringFree(result);
}


bool Connection::ElementIsChild(String^ database, String^ dimension, String^ de_parent, String^ de_child) {
	libpalo_err err;
	palo_bool result;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);
	pin_ptr<const wchar_t> pde_parent = PtrToStringChars(de_parent);
	pin_ptr<const wchar_t> pde_child = PtrToStringChars(de_child);

	if (!LIBPALO_SUCCESSFUL(palo_eischild_w_r(&err, &result, pso, NULL, pdatabase, pdimension, pde_parent, pde_child))) {
		throwException(err);
	}

	return result ? true : false;
}


unsigned int Connection::ElementParentCount(String^ database, String^ dimension, String^ dimension_element) {
	libpalo_err err;
	unsigned int result;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);
	pin_ptr<const wchar_t> pdimension_element = PtrToStringChars(dimension_element);

	if (!LIBPALO_SUCCESSFUL(palo_eparentcount_w_r(&err, &result, pso, NULL, pdatabase, pdimension, pdimension_element))) {
		throwException(err);
	}

	return result;
}


unsigned int Connection::ElementChildCount(String^ database, String^ dimension, String^ dimension_element) {
	libpalo_err err;
	unsigned int result;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);
	pin_ptr<const wchar_t> pdimension_element = PtrToStringChars(dimension_element);

	if (!LIBPALO_SUCCESSFUL(palo_echildcount_w_r(&err, &result, pso, NULL, pdatabase, pdimension, pdimension_element))) {
		throwException(err);
	}

	return result;
}


double Connection::ElementWeight(String^ database, String^ dimension, String^ de_parent, String^ de_child) {
	libpalo_err err;
	double result;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);
	pin_ptr<const wchar_t> pde_parent = PtrToStringChars(de_parent);
	pin_ptr<const wchar_t> pde_child = PtrToStringChars(de_child);

	if (!LIBPALO_SUCCESSFUL(palo_eweight_w_r(&err, &result, pso, NULL, pdatabase, pdimension, pde_parent, pde_child))) {
		throwException(err);
	}

	return result;
}


String^ Connection::ElementParentName(String^ database, String^ dimension, String^ dimension_element, unsigned long n) {
	libpalo_err err;
	wchar_t *result;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);
	pin_ptr<const wchar_t> pdimension_element = PtrToStringChars(dimension_element);

	if (!LIBPALO_SUCCESSFUL(palo_eparentname_w_r(&err, &result, pso, NULL, pdatabase, pdimension, pdimension_element, n))) {
		throwException(err);
	}

	return Helper::CharPtrToStringFree(result);
}


String^ Connection::ElementChildName(String^ database, String^ dimension, String^ dimension_element, unsigned long n) {
	libpalo_err err;
	wchar_t *result;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);
	pin_ptr<const wchar_t> pdimension_element = PtrToStringChars(dimension_element);

	if (!LIBPALO_SUCCESSFUL(palo_echildname_w_r(&err, &result, pso, NULL, pdatabase, pdimension, pdimension_element, n))) {
		throwException(err);
	}

	return Helper::CharPtrToStringFree(result);
}


unsigned int Connection::ElementIndex(String^ database, String^ dimension, String^ dimension_element) {
	libpalo_err err;
	unsigned int result;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);
	pin_ptr<const wchar_t> pdimension_element = PtrToStringChars(dimension_element);

	if (!LIBPALO_SUCCESSFUL(palo_eindex_w_r(&err, &result, pso, NULL, pdatabase, pdimension, pdimension_element))) {
		throwException(err);
	}

	return result;
}


unsigned int Connection::ElementLevel(String^ database, String^ dimension, String^ dimension_element) {
	libpalo_err err;
	unsigned int result;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);
	pin_ptr<const wchar_t> pdimension_element = PtrToStringChars(dimension_element);

	if (!LIBPALO_SUCCESSFUL(palo_elevel_w_r(&err, &result, pso, NULL, pdatabase, pdimension, pdimension_element))) {
		throwException(err);
	}

	return result;
}


unsigned int Connection::ElementIndent(String^ database, String^ dimension, String^ dimension_element) {
	libpalo_err err;
	unsigned int result;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);
	pin_ptr<const wchar_t> pdimension_element = PtrToStringChars(dimension_element);

	if (!LIBPALO_SUCCESSFUL(palo_eindent_w_r(&err, &result, pso, NULL, pdatabase, pdimension, pdimension_element))) {
		throwException(err);
	}

	return result;
}


String^ Connection::ElementSibling(String^ database, String^ dimension, String^ dimension_element, long n) {
	libpalo_err err;
	wchar_t *result;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);
	pin_ptr<const wchar_t> pdimension_element = PtrToStringChars(dimension_element);

	if (!LIBPALO_SUCCESSFUL(palo_esibling_w_r(&err, &result, pso, NULL, pdatabase, pdimension, pdimension_element, n))) {
		throwException(err);
	}

	return Helper::CharPtrToStringFree(result);
}


String^ Connection::ElementFirst(String^ database, String^ dimension) {
	libpalo_err err;
	wchar_t *result;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);

	if (!LIBPALO_SUCCESSFUL(palo_efirst_w_r(&err, &result, pso, NULL, pdatabase, pdimension))) {
		throwException(err);
	}

	return Helper::CharPtrToStringFree(result);
}


String^ Connection::ElementNext(String^ database, String^ dimension, String^ dimension_element) {
	libpalo_err err;
	wchar_t *result;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);
	pin_ptr<const wchar_t> pdimension_element = PtrToStringChars(dimension_element);

	if (!LIBPALO_SUCCESSFUL(palo_enext_w_r(&err, &result, pso, NULL, pdatabase, pdimension, pdimension_element))) {
		throwException(err);
	}

	return Helper::CharPtrToStringFree(result);
}


String^ Connection::ElementPrev(String^ database, String^ dimension, String^ dimension_element) {
	libpalo_err err;
	wchar_t *result;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);
	pin_ptr<const wchar_t> pdimension_element = PtrToStringChars(dimension_element);

	if (!LIBPALO_SUCCESSFUL(palo_eprev_w_r(&err, &result, pso, NULL, pdatabase, pdimension, pdimension_element))) {
		throwException(err);
	}

	return Helper::CharPtrToStringFree(result);
}


unsigned int Connection::ElementCount(String^ database, String^ dimension) {
	libpalo_err err;
	unsigned int result;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);

	if (!LIBPALO_SUCCESSFUL(palo_ecount_w_r(&err, &result, pso, NULL, pdatabase, pdimension))) {
		throwException(err);
	}

	return result;
}


unsigned int Connection::ElementTopLevel(String^ database, String^ dimension) {
	libpalo_err err;
	unsigned int result;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);

	if (!LIBPALO_SUCCESSFUL(palo_etoplevel_w_r(&err, &result, pso, NULL, pdatabase, pdimension))) {
		throwException(err);
	}

	return result;
}


// use libpalo_ng directly
array<ElementInfo^>^ Connection::DimensionListElements(String^ database, String^ dimension) {
    ErrorInformation^ ErrorInfo = gcnew ErrorInformation();
    array<ElementInfo^>^ eia;

	long i = 0, dimlen, len = 0, current_index = 0;

	pin_ptr<struct sock_obj> pso = so;
	std::string sdatabase, sdimension;

	Helper::ConvertStringToUTF8(sdatabase, database);
	Helper::ConvertStringToUTF8(sdimension, dimension);

	try {
		Dimension& dim = (*(pso->myServer))[sdatabase].dimension[sdimension];
		std::unique_ptr<DimensionCache::CacheIterator> it = dim.getIterator(); 
		dimlen = dim.getCacheData().number_elements;
		eia = gcnew array<ElementInfo^>(dimlen);
		std::unordered_set<IdentifierType> AlreadyUsed;
		IdentifierType id;


		for (; !(*it).end(); ++current_index, ++*it) {
			const jedox::palo::ELEMENT_INFO& ei = (**it);

			// sanity check
			if (current_index >= dimlen) {
				System::Array::Resize(eia, ++dimlen);
			}

			ElementInfo^ current_ei = eia[current_index] = gcnew ElementInfo();

			id = ei.element;

			if ((AlreadyUsed.find(id) == AlreadyUsed.end()) && !ei.nelement.empty()) {
				AlreadyUsed.insert(id);
				current_ei->Identifier = id;
				current_ei->Name = Helper::ConvertUTF8ToString(ei.nelement);
		
				switch (ei.type) {
					case ELEMENT_INFO::NUMERIC:
						current_ei->Type = DimElementType::DimElementTypeNumeric;
						break;

					case ELEMENT_INFO::STRING:
						current_ei->Type = DimElementType::DimElementTypeString;
						break;

					case ELEMENT_INFO::CONSOLIDATED:
						current_ei->Type = DimElementType::DimElementTypeConsolidated;
						break;

					default:
						current_ei->Type = DimElementType::DimElementTypeRule;
						break;
				}

				current_ei->Position = ei.position;
				current_ei->Level = ei.level;
				current_ei->Indent = ei.indent;
				current_ei->Depth = ei.depth;

				len = ei.number_parents;
				current_ei->NumberParents = len;
				current_ei->Parents = gcnew array<ParentInfoRaw>(len);

				for (i = 0; i < len; i++) {
					current_ei->Parents[i].Identifier = ei.parents[i];
				}

				len = ei.number_children;
				current_ei->NumberChildren = len;
				current_ei->Children = gcnew array<ChildInfoRaw>(len);

				for (i = 0; i < len; i++) {
					current_ei->Children[i].Identifier = ei.children[i];
					current_ei->Children[i].Factor = ei.weights[i];
				}
			} else {
				current_index--;
			}

		}

		if (current_index != dimlen) {
			System::Array::Resize(eia, current_index);
		}
		if (current_index > 0) {
			System::Array::Sort(eia, gcnew System::Comparison<ElementInfo^>(PositionComparison));
		}
	
	} catch (const SocketException& exp) {
		ErrorInfo->ProcessSocketError(exp.what());
	} catch (const PaloServerException& exp) {
		ErrorInfo->ProcessError(exp.code(), exp.Description());
	} catch (const std::exception& exp) {
		ErrorInfo->ProcessStandardError(exp.what());
	} catch (const System::Exception^) {
		throw;
#ifndef _DEBUG
	} catch (...) {
		ErrorInfo->ProcessUnknownError();
#endif
	}

	ErrorInfo->Check();

    return eia;
}


size_t Connection::DimensionFlatCount(String^ database, String^ dimension) {
    libpalo_err err;

    size_t result;

	pin_ptr<struct sock_obj> pso = so;
    pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
    pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);

    if (!LIBPALO_SUCCESSFUL(palo_dimension_flat_count_w_r(&err, &result, pso, NULL, pdatabase, pdimension))) {
		throwException(err);
    }

	return result;
}


// use directly libpalo_ng
array<ElementInfo^>^ Connection::RestrictedList(String^ database, String^ dimension, String^ element, long parentid, long %start, long limit) {
    ErrorInformation^ ErrorInfo = gcnew ErrorInformation();
    array<ElementInfo^>^ eia;

	if (*(so->version) < 302) {
		if (parentid == -2) {
			return DimensionListElements(database, dimension);
		} else {
			ErrorInfo->ProcessError(PALO_ERR_NOT_IMPLEMENTED_ID, "This server doesn't support this call since it is too old.");
			ErrorInfo->Check();
		}
	}

	pin_ptr<struct sock_obj> pso = so;
	std::string sdatabase, sdimension, selement;

	Helper::ConvertStringToUTF8(sdatabase, database);
	Helper::ConvertStringToUTF8(sdimension, dimension);
	if (element != nullptr) {
		Helper::ConvertStringToUTF8(selement, element);
	}

	long pos = 0;
	IdentifierType itstart = start;
	try {
		std::list<ELEMENT_INFO> elems = (*(pso->myServer))[sdatabase].dimension[sdimension].getElements(parentid, itstart, selement, limit);
		std::list<ELEMENT_INFO>::iterator it = elems.begin();
		std::list<ELEMENT_INFO>::iterator endlist = elems.end();

		long i, len = (long)elems.size(), len2;
		eia = gcnew array<ElementInfo^>(len);

		ElementInfo^ current_ei;

		while ((pos < len) && (it != endlist)) {
			const jedox::palo::ELEMENT_INFO& ei = (*it);
			current_ei = eia[pos] = gcnew ElementInfo();

			if (!ei.nelement.empty()) {
				current_ei->Identifier = ei.element;
				current_ei->Name = Helper::ConvertUTF8ToString(ei.nelement);
			}
		
			switch (ei.type) {
				case ELEMENT_INFO::NUMERIC:
					current_ei->Type = DimElementType::DimElementTypeNumeric;
					break;

				case ELEMENT_INFO::STRING:
					current_ei->Type = DimElementType::DimElementTypeString;
					break;

				case ELEMENT_INFO::CONSOLIDATED:
					current_ei->Type = DimElementType::DimElementTypeConsolidated;
					break;

				default:
					current_ei->Type = DimElementType::DimElementTypeRule;
					break;
			}

			current_ei->Position = ei.position;
			current_ei->Level = ei.level;
			current_ei->Indent = ei.indent;
			current_ei->Depth = ei.depth;

			len2 = ei.number_parents;
			current_ei->NumberParents = len2;
			current_ei->Parents = gcnew array<ParentInfoRaw>(len2);
			for (i = 0; i < len2; i++) {
				current_ei->Parents[i].Identifier = ei.parents[i];
			}

			len2 = ei.number_children;
			current_ei->NumberChildren = len2;
			current_ei->Children = gcnew array<ChildInfoRaw>(len2);
			for (i = 0; i < len2; i++) {
				current_ei->Children[i].Identifier = ei.children[i];
				current_ei->Children[i].Factor = ei.weights[i];
			}

			pos++;
			++it;
		}
		start = itstart;
	} catch (const SocketException& exp) {
		ErrorInfo->ProcessSocketError(exp.what());
	} catch (const PaloServerException& exp) {
		ErrorInfo->ProcessError(exp.code(), exp.Description());
	} catch (const std::exception& exp) {
		ErrorInfo->ProcessStandardError(exp.what());
	} catch (const System::Exception^) {
		throw;
#ifndef _DEBUG
	} catch (...) {
		ErrorInfo->ProcessUnknownError();
#endif
	}

	ErrorInfo->Check();

	System::Array::Resize<ElementInfo^>(eia, pos);

    return eia;
}


array<ElementInfo^>^ Connection::DimensionRestrictedFlatListDimElements(String^ database, String^ dimension, long start, long limit) {
	return RestrictedList(database, dimension, nullptr, -2, start, limit);
}


array<ElementInfo^>^ Connection::DimensionRestrictedTopListDimElements(String^ database, String^ dimension, long start, long limit) {
	return RestrictedList(database, dimension, nullptr, -1, start, limit);
}


array<ElementInfo^>^ Connection::DimensionRestrictedChildrenListDimElements(String^ database, String^ dimension, long elementidentifier, long start, long limit) {
	return RestrictedList(database, dimension, nullptr, elementidentifier, start, limit);
}


array<ElementInfo^>^ Connection::DimensionRestrictedFlatListFindElement(String^ database, String^ dimension, String^ element, long %start, long limit) {
	return RestrictedList(database, dimension, element, -2, start, limit);
}


// use directly libpalo_ng
array<ConsolidationInfo>^ Connection::DimElementListConsolidated(String^ database, String^ dimension, String^ element) {
	ErrorInformation^ ErrorInfo = gcnew ErrorInformation();
    array<ConsolidationInfo>^ cia;

	long i, id, len, pos = 0;

	pin_ptr<struct sock_obj> pso = so;
	std::string sdatabase, sdimension, selement;

	Helper::ConvertStringToUTF8(sdatabase, database);
	Helper::ConvertStringToUTF8(sdimension, dimension);
	Helper::ConvertStringToUTF8(selement, element);

	try {
		Dimension& dim = (*(pso->myServer))[sdatabase].dimension[sdimension];
		const ELEMENT_INFO &dei = dim[selement].getCacheData();
		ELEMENT_LIST children = dei.children;
		ELEMENT_WEIGHT_LIST weights = dei.weights;

		len = (long)children.size();
		cia = gcnew array<ConsolidationInfo>(len);

		for (i = 0; i < len; i++) {
			id = children[i];
			if (dim.Exists(id)) {
				const jedox::palo::ELEMENT_INFO& ei = dim[id].getCacheData();
				cia[pos].Name = Helper::ConvertUTF8ToString(ei.nelement);

				switch (ei.type) {
					case ELEMENT_INFO::NUMERIC:
						cia[pos].Type = DimElementType::DimElementTypeNumeric;
						break;

					case ELEMENT_INFO::STRING:
						cia[pos].Type = DimElementType::DimElementTypeString;
						break;
					
					case ELEMENT_INFO::CONSOLIDATED:
						cia[pos].Type = DimElementType::DimElementTypeConsolidated;
						break;
					
					case ELEMENT_INFO::UNKNOWN:
						cia[pos].Type = DimElementType::DimElementTypeRule;
						break;
				}
				cia[pos++].Factor = weights[i];
			}
		}
	} catch (const SocketException& exp) {
		ErrorInfo->ProcessSocketError(exp.what());
	} catch (const PaloServerException& exp) {
		ErrorInfo->ProcessError(exp.code(), exp.Description());
	} catch (const std::exception& exp) {
		ErrorInfo->ProcessStandardError(exp.what());
	} catch (const System::Exception^) {
		throw;
#ifndef _DEBUG
	} catch (...) {
		ErrorInfo->ProcessUnknownError();
#endif
	}

	ErrorInfo->Check();
	System::Array::Resize<ConsolidationInfo>(cia, pos);

    return cia;
}


// use directly libpalo_ng
array<ConsolidationExtendedInfo>^ Connection::ElementListConsolidated(String^ database, String^ dimension, String^ element) {
	ErrorInformation^ ErrorInfo = gcnew ErrorInformation();
    array<ConsolidationExtendedInfo>^ cia;

	long i = 0, len = 0, pos = 0, id;

	pin_ptr<struct sock_obj> pso = so;
	std::string sdatabase, sdimension, selement;

	Helper::ConvertStringToUTF8(sdatabase, database);
	Helper::ConvertStringToUTF8(sdimension, dimension);
	Helper::ConvertStringToUTF8(selement, element);

	try {
		Dimension& dim = (*(pso->myServer))[sdatabase].dimension[sdimension];
		const ELEMENT_INFO &dei = dim[selement].getCacheData();
		ELEMENT_LIST children = dei.children;
		ELEMENT_WEIGHT_LIST weights = dei.weights;

		len = (long)children.size();
		cia = gcnew array<ConsolidationExtendedInfo>(len);

		for (i = 0; i < len; i++) {
			id = children[i];
			if (dim.Exists(id)) {
				const jedox::palo::ELEMENT_INFO& ei = dim[id].getCacheData();
				cia[pos].Identifier = id;
				cia[pos].Name = Helper::ConvertUTF8ToString(ei.nelement);

				switch (ei.type) {
					case ELEMENT_INFO::NUMERIC:
						cia[pos].Type = DimElementType::DimElementTypeNumeric;
						break;

					case ELEMENT_INFO::STRING:
						cia[pos].Type = DimElementType::DimElementTypeString;
						break;
					
					case ELEMENT_INFO::CONSOLIDATED:
						cia[pos].Type = DimElementType::DimElementTypeConsolidated;
						break;
					
					case ELEMENT_INFO::UNKNOWN:
						cia[pos].Type = DimElementType::DimElementTypeRule;
						break;
				}
				cia[pos++].Factor = weights[i];
			}
		}
	} catch (const SocketException& exp) {
		ErrorInfo->ProcessSocketError(exp.what());
	} catch (const PaloServerException& exp) {
		ErrorInfo->ProcessError(exp.code(), exp.Description());
	} catch (const std::exception& exp) {
		ErrorInfo->ProcessStandardError(exp.what());
	} catch (const System::Exception^) {
		throw;
#ifndef _DEBUG
	} catch (...) {
		ErrorInfo->ProcessUnknownError();
#endif
	}

	ErrorInfo->Check();
	System::Array::Resize<ConsolidationExtendedInfo>(cia, pos);

    return cia;
}


// use directly libpalo_ng
array<SubsetResult>^ Connection::DimensionSubset(	String^ database, String^ dimension, long indent,
													ArgAliasFilterSettings alias,
													ArgFieldFilterSettings field,
													ArgBasicFilterSettings basic,
													ArgDataFilterSettings data,
													ArgSortingFilterSettings sorting,
													ArgStructuralFilterSettings structural,
													ArgTextFilterSettings text) {
    ErrorInformation^ ErrorInfo = gcnew ErrorInformation();
	array<SubsetResult>^ sra = nullptr;

	pin_ptr<struct sock_obj> pso = so;
	std::string sdatabase, sdimension;
	AliasFilterSettings m_alias;
	FieldFilterSettings m_field;
	BasicFilterSettings m_basic;
	DataFilterSettings m_data;
	SortingFilterSettings m_sorting;
	StructuralFilterSettings m_structural;
	TextFilterSettings m_text;

	Helper::ConvertStringToUTF8(sdatabase, database);
	Helper::ConvertStringToUTF8(sdimension, dimension);

	m_alias.active = alias.Active;

	if (alias.Active) {
		m_alias.flags = (unsigned int)alias.Flags;
		m_alias.indent = indent;
		Helper::ConvertStringToUTF8(m_alias.attribute1, alias.Attribute1);
		Helper::ConvertStringToUTF8(m_alias.attribute2, alias.Attribute2);
	}


	m_field.active = field.Active;

	if (field.Active) {
		m_field.flags = (unsigned int)field.Flags;
		m_field.indent = indent;
		Helper::ConvertStringArrayArrayToUTF8(m_field.advanced, field.Advanced);
	}


	m_basic.active = basic.Active;

	if (basic.Active) {
		m_basic.flags = (unsigned int)basic.Flags;
		m_basic.indent = indent;
		m_basic.manual_subset_set = basic.ManualSubsetSet;
		Helper::ConvertStringArrayToUTF8(m_basic.manual_subset, basic.ManualSubset);
	}


	m_data.active = data.Active;

	if (data.Active) {
		m_data.flags = (unsigned int)data.Flags;
		m_data.indent = indent;
		m_data.cmp.par1d = data.Cmp.Par1d;
		m_data.cmp.par2d = data.Cmp.Par2d;
		m_data.cmp.use_strings = data.Cmp.UseStrings;
		m_data.lower_percentage_set = data.LowerPercentageSet;
		m_data.lower_percentage = data.LowerPercentage;
		m_data.upper_percentage_set = data.UpperPercentageSet;
		m_data.upper_percentage = data.UpperPercentage;
		m_data.top = data.Top;
		m_data.coords_set = data.CoordsSet;

		Helper::ConvertStringToUTF8(m_data.cube, data.Cube);
		Helper::ConvertStringToUTF8(m_data.cmp.op1, data.Cmp.Op1);
		Helper::ConvertStringToUTF8(m_data.cmp.op2, data.Cmp.Op2);
		Helper::ConvertStringToUTF8(m_data.cmp.par1s, data.Cmp.Par1s);
		Helper::ConvertStringToUTF8(m_data.cmp.par2s, data.Cmp.Par2s);

		long i, len = data.Coords->Length;
		std::vector<std::string> elems;

		m_data.coords.resize(len);

		for (i = 0; i < len; i++) {
			Helper::ConvertStringArrayToUTF8(elems, data.Coords[i].Str);
			m_data.coords[i] = std::make_pair<bool, std::vector<std::string> >(data.Coords[i].BoolVal, elems);
		}
	}


	m_sorting.active = sorting.Active;

	if (sorting.Active) {
		m_sorting.flags = (unsigned int)sorting.Flags;
		m_sorting.indent = indent;
		m_sorting.level = sorting.Level;
		m_sorting.limit_count = sorting.Limit_count;
		m_sorting.limit_start = sorting.Limit_start;
		Helper::ConvertStringToUTF8(m_sorting.attribute, sorting.Attribute);
	}

	m_structural.active = structural.Active;

	if (structural.Active) {
		m_structural.flags = (unsigned int)structural.Flags;
		m_structural.indent = indent;
		m_structural.level = structural.Level;
		m_structural.level_start = structural.LevelStart;
		m_structural.level_end = structural.LevelEnd;
		m_structural.revolve = structural.Revolve;
		m_structural.revolve_count = structural.RevolveCount;
		Helper::ConvertStringToUTF8(m_structural.bound, structural.Bound);
		Helper::ConvertStringToUTF8(m_structural.revolve_elem, structural.RevolveElement);
	}


	m_text.active = text.Active;

	if (text.Active) {
		m_text.flags = (unsigned int)text.Flags;
		m_text.indent = indent;
		Helper::ConvertStringArrayToUTF8(m_text.regexps, text.RegularExpressions);
	}


	try {
		ElementExList el;
		(*(pso->myServer))[sdatabase].dimension[sdimension].subset(el, pso->myServer, m_alias, m_field, m_basic, m_data, m_sorting, m_structural, m_text);

		long i, len = (long)el.size();

		sra = gcnew array<SubsetResult>(len);

		ElementExList::iterator it, elb = el.begin(), ele = el.end();
		for (it = elb, i = 0; it != ele; ++it, ++i) {
			const jedox::palo::ELEMENT_INFO_EXT& eie = (*it);
			sra[i].Identifier = eie.m_einf.element;
			sra[i].Name = Helper::ConvertUTF8ToString(eie.get_name());
			sra[i].Alias = Helper::ConvertUTF8ToString(eie.get_alias());
			sra[i].Path = Helper::ConvertUTF8ToString(eie.path);

			sra[i].Index = eie.get_idx(indent);
			sra[i].Depth = eie.m_einf.depth;
			sra[i].HasChildren = (eie.m_einf.children.size() > 0);
		}
	} catch (const SocketException& exp) {
		ErrorInfo->ProcessSocketError(exp.what());
	} catch (const PaloServerException& exp) {
		ErrorInfo->ProcessError(exp.code(), exp.Description());
	} catch (const std::exception& exp) {
		ErrorInfo->ProcessStandardError(exp.what());
	} catch (const System::Exception^) {
		throw;
#ifndef _DEBUG
	} catch (...) {
		ErrorInfo->ProcessUnknownError();
#endif
	}

	ErrorInfo->Check();

    return sra;
}


// use directly libpalo_ng
Dictionary<long, String^>^ Connection::GetAttributeValues(String^ database, String^ dimension, array<long>^ elements, String^ attribute, long lastelement, unsigned long NumDatasets){
    ErrorInformation^ ErrorInfo = gcnew ErrorInformation();
	Dictionary<long, String^>^ result = nullptr;

	pin_ptr<struct sock_obj> pso = so;
	std::string sdatabase, sdimension, sattribute;

	Helper::ConvertStringToUTF8(sdatabase, database);
	Helper::ConvertStringToUTF8(sdimension, dimension);
	Helper::ConvertStringToUTF8(sattribute, attribute);

	try {
		long i, vsize = elements->Length, vsize2 = (lastelement > -1) ? 2 : 0;
		Database& db = (*(pso->myServer))[sdatabase];
		const DIMENSION_INFO& dim = db.dimension[sdimension].getCacheData();
		long attribute_cube = dim.attribute_cube;
		long attribute_dim = dim.assoc_dimension;
		unsigned long maxsize = (vsize == 0) ? dim.number_elements : vsize;
		if ((NumDatasets > 0) && (NumDatasets < maxsize)) {
			maxsize = NumDatasets;
		}

		ELEMENT_LIST v0(1), v1(vsize), start(vsize2);
		std::vector<ELEMENT_LIST > area(2);
		v0[0] = db.dimension[attribute_dim][sattribute].getCacheData().element;
		for (i = 0; i < vsize; i++) {
			v1[i] = elements[i];
		}
		if (vsize2 > 0) {
			start[0] = v0[0];
			start[1] = lastelement;
		}
		area[0] = v0;
		area[1] = v1;

		std::vector<CELL_VALUE_EXPORTED> expresult;
		db.cube[attribute_cube].CellExport(expresult, area, maxsize, start, "", 0, 0, 0);
		vsize = (long)(expresult.size() - 1);
		result = gcnew Dictionary<long, String^>(vsize);
		std::stringstream tmpstr;
		long id;
	
		for (i = 0; i < vsize; i++) {
			id = expresult[i].cvp.path[1];
			if (!result->ContainsKey(id))
			{
				if (expresult[i].cvp.type == CELL_VALUE_PATH_EXPORTED::NUMERIC) {
					tmpstr.str("");
					tmpstr << expresult[i].cvp.val.d;
					result->Add(id, Helper::ConvertUTF8ToString(tmpstr.str()));
				} else {
					result->Add(id, Helper::ConvertUTF8ToString(expresult[i].cvp.val.s));
				}
			}
		}
	} catch (const SocketException& exp) {
		ErrorInfo->ProcessSocketError(exp.what());
	} catch (const PaloServerException& exp) {
		ErrorInfo->ProcessError(exp.code(), exp.Description());
	} catch (const std::exception& exp) {
		ErrorInfo->ProcessStandardError(exp.what());
	} catch (const System::Exception^) {
		throw;
#ifndef _DEBUG
	} catch (...) {
		ErrorInfo->ProcessUnknownError();
#endif
	}

	ErrorInfo->Check();

    return result;
}

Dictionary<long, String^>^ Connection::GetAttributeValues(String^ database, String^ dimension, array<long>^ elements, String^ attribute) {
	return GetAttributeValues(database, dimension, elements, attribute, -1, 0);
}


// use directly libpalo_ng
Dictionary<long, array<String^>^ >^ Connection::GetAttributeValues(String^ database, String^ dimension, array<long>^ elements, array<long>^ attributes){
    ErrorInformation^ ErrorInfo = gcnew ErrorInformation();
	Dictionary<long, array<String^>^ >^ result = nullptr;

	pin_ptr<struct sock_obj> pso = so;
	std::string sdatabase, sdimension;

	Helper::ConvertStringToUTF8(sdatabase, database);
	Helper::ConvertStringToUTF8(sdimension, dimension);

	try {
		long i, j, vsize = elements->Length, vsize0 = attributes->Length, vsize2;
		Database& db = (*(pso->myServer))[sdatabase];
		const DIMENSION_INFO& dim = db.dimension[sdimension].getCacheData();
		long attribute_cube = dim.attribute_cube;
		if (vsize0 > 0) {
			unsigned long maxsize = (vsize == 0) ? dim.number_elements : vsize;
			maxsize *= vsize0;
			ELEMENT_LIST v0(vsize0), v1(vsize), start(0);
			std::vector<ELEMENT_LIST > area(2);
		
			for (i = 0; i < vsize0; i++) {
				v0[i] = attributes[i];
			}
			for (i = 0; i < vsize; i++) {
				v1[i] = elements[i];
			}

			area[0] = v0;
			area[1] = v1;

			std::vector<CELL_VALUE_EXPORTED> expresult;
			db.cube[attribute_cube].CellExport(expresult, area, maxsize, start, "", 0, 0, 0);
			vsize2 = (long)(expresult.size() - 1);
	
			result = gcnew Dictionary<long, array<String^>^>(vsize);
			std::stringstream tmpstr;
			long id;

			for (i = 0; i < vsize2; i++) {
				j = i / vsize;
				id = expresult[i].cvp.path[1];

				if (i < vsize && !result->ContainsKey(id)) {
					result->Add(id, gcnew array<String^>(vsize0));
				}

				if (expresult[i].cvp.type == CELL_VALUE_PATH_EXPORTED::NUMERIC) {
					tmpstr.str("");
					tmpstr << expresult[i].cvp.val.d;
					result[id][j] = Helper::ConvertUTF8ToString(tmpstr.str());
				} else {
					result[id][j] = Helper::ConvertUTF8ToString(expresult[i].cvp.val.s);
				}
			}
		}
	} catch (const SocketException& exp) {
		ErrorInfo->ProcessSocketError(exp.what());
	} catch (const PaloServerException& exp) {
		ErrorInfo->ProcessError(exp.code(), exp.Description());
	} catch (const std::exception& exp) {
		ErrorInfo->ProcessStandardError(exp.what());
	} catch (const System::Exception^) {
		throw;
#ifndef _DEBUG
	} catch (...) {
		ErrorInfo->ProcessUnknownError();
#endif
	}

	ErrorInfo->Check();

    return result;
}

// End Element related


// Start Cube related
void Connection::CubeLoad(String^ database, String^ cube) {
	libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pcube = PtrToStringChars(cube);

	if (!LIBPALO_SUCCESSFUL(palo_cube_load_w_r(&err, pso, NULL, pdatabase, pcube))) {
		throwException(err);
	}
}


String^ Connection::GetCubeNameFromID(String^ database, long id) {
    ErrorInformation^ ErrorInfo = gcnew ErrorInformation();
    String^ cube = nullptr;

	pin_ptr<struct sock_obj> pso = so;
	std::string sdatabase;

	Helper::ConvertStringToUTF8(sdatabase, database);

	try {
		cube = Helper::ConvertUTF8ToString((*(pso->myServer))[sdatabase].cube[id].getCacheData().ncube);
	} catch (const SocketException& exp) {
		ErrorInfo->ProcessSocketError(exp.what());
	} catch (const PaloServerException& exp) {
		ErrorInfo->ProcessError(exp.code(), exp.Description());
	} catch (const std::exception& exp) {
		ErrorInfo->ProcessStandardError(exp.what());
	} catch (const System::Exception^) {
		throw;
#ifndef _DEBUG
	} catch (...) {
		ErrorInfo->ProcessUnknownError();
#endif
	}

	ErrorInfo->Check();

    return cube;
}


void Connection::UnloadCube(String^ database, String^ cube) {
	libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pcube = PtrToStringChars(cube);

	if (!LIBPALO_SUCCESSFUL(palo_cube_unload_w_r(&err, pso, NULL, pdatabase, pcube))) {
		throwException(err);
	}
}


void Connection::CubeCommitLog(String^ database, String^ cube) {
	libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pcube = PtrToStringChars(cube);

	if (!LIBPALO_SUCCESSFUL(palo_cube_commit_log_w_r(&err, pso, NULL, pdatabase, pcube))) {
		throwException(err);
	}
}


void Connection::DatabaseAddCube(String^ database, String^ cube, array<String^>^ dimensions, CubeType type) {
	libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pcube = PtrToStringChars(cube);

	ARRAY_MARSHAL_CONVERT(adimensions, dimensions)
	palo_database_add_cube2_w_r(&err, pso, NULL, pdatabase, pcube, adimensions, (cube_type)type);
	ARRAY_MARSHAL_CLEANUP(adimensions);

	if (!LIBPALO_SUCCESSFUL(err.result)) {
		throwException(err);
	}
}


void Connection::DatabaseAddCube(String^ database, String^ cube, array<String^>^ dimensions) {
	DatabaseAddCube(database, cube, dimensions, CubeType::NormalCube);
}


void Connection::DatabaseRenameCube(String^ database, String^ cube_oldname, String^ cube_newname) {
	libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pcube_oldname = PtrToStringChars(cube_oldname);
	pin_ptr<const wchar_t> pcube_newname = PtrToStringChars(cube_newname);

	if (!LIBPALO_SUCCESSFUL(palo_database_rename_cube_w_r(&err, pso, NULL, pdatabase, pcube_oldname, pcube_newname))) {
		throwException(err);
	}
}


void Connection::DeleteCube(String^ database, String^ cube) {
	libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pcube = PtrToStringChars(cube);

	if (!LIBPALO_SUCCESSFUL(palo_cube_delete_w_r(&err, pso, NULL, pdatabase, pcube))) {
		throwException(err);
	}
}


void Connection::CubeClear(String^ database, String^ cube, array<array<String^>^>^ elements) {
	libpalo_err err;

	/* convert elements */
	struct arg_str_array_array_w lib_elements;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pcube = PtrToStringChars(cube);
	bool Complete = (elements == nullptr);

	if (!Complete) {
		lib_elements = ManagedStringArrayArray2arg_str_array_array_w(elements);
	}

	palo_cube_clear_w_r(&err, pso, NULL, pdatabase, pcube, (Complete) ? 1 : 0 , lib_elements);  

	if (!Complete) {
		/* cleanup */
		FreeConverted_arg_str_array_array_w(lib_elements);
	}

	if (!LIBPALO_SUCCESSFUL(err.result)) {
		throwException(err);
	}

	return;
}


void Connection::CubeClear(String^ database, String^ cube) {
	CubeClear(database, cube, nullptr);
}


void Connection::CubeConvert(String^ database, String^ cube, CubeType newType){
    libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
    pin_ptr<const wchar_t> pcube = PtrToStringChars(cube);

    palo_cube_convert_w_r(&err, pso, NULL, pdatabase, pcube, cube_type(newType));

    if (!LIBPALO_SUCCESSFUL(err.result)) {
		throwException(err);
    }
}


CubeType Connection::GetCubeTypeHelper(unsigned long type) {
	CubeType retval = CubeType::NormalCube;

	switch (type) {
		case normal_cube:
			retval = CubeType::NormalCube;
			break;

		case system_cube:
			retval = CubeType::SystemCube;
			break;

		case attribut_cube:
			retval = CubeType::AttributeCube;
			break;

		case user_info_cube:
			retval = CubeType::UserInfoCube;
			break;

		case gpu_cube:
			retval = CubeType::GpuCube;
			break;
	}

	return retval;
}


CubeType Connection::GetCubeType(String^ database, String^ cube) {
	libpalo_err err;
	unsigned int result = normal_cube;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pcube = PtrToStringChars(cube);

	if (!LIBPALO_SUCCESSFUL(palo_get_cube_type_w_r(&err, &result, pso, NULL, pdatabase, pcube))) {
		throwException(err);
	}

	return GetCubeTypeHelper(result);
}


CubeInfo Connection::CubeInformation(String^ database, String^ cube) {
	libpalo_err err;
	struct arg_cube_info_w cubeinfo;
	CubeInfo ci;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pcube = PtrToStringChars(cube);

	if (!LIBPALO_SUCCESSFUL(palo_cube_info_w_r(&err, &cubeinfo, pso, NULL, pdatabase, pcube))) {
		throwException(err);
	}

	ci.id = cubeinfo.id;
	ci.Name = gcnew String(cubeinfo.name);
	ci.NumberDimensions = cubeinfo.number_dimensions;
	long i, len = (long)cubeinfo.dimensions.len;
	ci.Dimensions = gcnew array<String^>(len);

	for (i = 0; i < len; i++) {
		ci.Dimensions[i] = gcnew String(cubeinfo.dimensions.a[i]);
	}

	ci.NumberCells = cubeinfo.number_cells;
	ci.NumberFilledCells = cubeinfo.number_filled_cells;

	switch (cubeinfo.status) {
		case unloaded_cube:
			ci.Status = CubeStatus::UnloadedCube;
			break;

		case loaded_cube:
			ci.Status = CubeStatus::LoadedCube;
			break;

		case changed_cube:
			ci.Status = CubeStatus::ChangedCube;
			break;

		case unknown_cube_status:
			ci.Status = CubeStatus::UnknownCubeStatus;
			break;
	}

	ci.Type = GetCubeTypeHelper(cubeinfo.type);

	free_arg_cube_info_contents_w(&cubeinfo);

	return ci;
}


String^ Connection::GetAttributeCube(String^ database, String^ dimension) {
	libpalo_err err;
	wchar_t *result;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);

	if (!LIBPALO_SUCCESSFUL(palo_get_attribute_cube_w_r(&err, &result, pso, NULL, pdatabase, pdimension))) {
		throwException(err);
	}

	return Helper::CharPtrToStringFree(result);
}


String^ Connection::GetRightsCube(String^ database, String^ dimension) {
	libpalo_err err;
	wchar_t *result;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);

	if (!LIBPALO_SUCCESSFUL(palo_get_rights_cube_w_r(&err, &result, pso, NULL, pdatabase, pdimension))) {
		throwException(err);
	}

	return Helper::CharPtrToStringFree(result);
}

array<String^>^ Connection::DatabaseListCubes(String^ database) {
    libpalo_err err;
    struct arg_str_array_w result;

	pin_ptr<struct sock_obj> pso = so;
    pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);

    if (!LIBPALO_SUCCESSFUL(palo_database_list_cubes_w_r(&err, &result, pso, NULL, pdatabase))) {
		throwException(err);
    }

    return Helper::ArgStringArrayToManagedFree(result);
}


array<String^>^ Connection::DatabaseListCubes(String^ database, CubeType type, bool OnlyCubesWithCells) {
    libpalo_err err;
    struct arg_str_array_w result;

	pin_ptr<struct sock_obj> pso = so;
    pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);

    if (!LIBPALO_SUCCESSFUL(palo_database_list_cubes2_w_r(&err, &result, pso, NULL, pdatabase, (unsigned int)type, (OnlyCubesWithCells) ? 1 : 0))) {
		throwException(err);
    }

    return Helper::ArgStringArrayToManagedFree(result);
}


array<String^>^ Connection::DatabaseListCubes(String^ database, CubeType type){
    return DatabaseListCubes(database, type, false);
}


array<String^>^ Connection::CubeListDimensions(String^ database, String^ cube) {
    libpalo_err err;
    struct arg_str_array_w dimensions;

	pin_ptr<struct sock_obj> pso = so;
    pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
    pin_ptr<const wchar_t> pcube = PtrToStringChars(cube);

    if (!LIBPALO_SUCCESSFUL(palo_cube_list_dimensions_w_r(&err, &dimensions, pso, NULL, pdatabase, pcube))) {
		throwException(err);
    }

    return Helper::ArgStringArrayToManagedFree(dimensions);
}


array<String^>^ Connection::DimensionListCubes(String^ database, String^ dimension) {
    libpalo_err err;
    struct arg_str_array_w cubes;

	pin_ptr<struct sock_obj> pso = so;
    pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
    pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);

    if (!LIBPALO_SUCCESSFUL(palo_dimension_list_cubes_w_r(&err, &cubes, pso, NULL, pdatabase, pdimension))) {
		throwException(err);
    }

    return Helper::ArgStringArrayToManagedFree(cubes);
}


array<String^>^ Connection::DimensionListCubes(String^ database, String^ dimension, CubeType Type, bool OnlyCubesWithCells) {
    libpalo_err err;
    struct arg_str_array_w cubes;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
    pin_ptr<const wchar_t> pdimension = PtrToStringChars(dimension);

    if (!LIBPALO_SUCCESSFUL(palo_dimension_list_cubes2_w_r(&err, &cubes, pso, NULL, pdatabase, pdimension, (unsigned int)Type, (OnlyCubesWithCells) ? 1 : 0))) {
		throwException(err);
    }

    return Helper::ArgStringArrayToManagedFree(cubes);
}


array<String^>^ Connection::DimensionListCubes(String^ database, String^ dimension, CubeType Type) {
    return DimensionListCubes(database, dimension, Type, false);
}


RuleInfo^ Connection::RuleCreate(String^ database, String^ cube, String^ definition, String^ ExternId, String^ Comment, bool Activate, double Position) {
	libpalo_err err;
	arg_rule_info_w ruleinfo;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pcube = PtrToStringChars(cube);
	pin_ptr<const wchar_t> pdefinition = PtrToStringChars(definition);
	pin_ptr<const wchar_t> pExternId = nullptr;
	pin_ptr<const wchar_t> pComment = nullptr;

	if (ExternId != nullptr) {
		pExternId = PtrToStringChars(ExternId);
	}

	if (Comment != nullptr) {
		pComment = PtrToStringChars(Comment);
	}

	palo_rule_add_w_r(&err, &ruleinfo, pso, NULL, pdatabase, pcube, pdefinition, 0, pExternId, pComment, (Activate) ? 1 : 0, Position);

	if (!LIBPALO_SUCCESSFUL(err.result)) {
		throwException(err);
	}

	return getRuleInfoAndFree_arg_rule_info_w(ruleinfo);

};


RuleInfo^ Connection::RuleCreate(String^ database, String^ cube, String^ definition, String^ ExternId, String^ Comment, bool Activate) {
	return RuleCreate(database, cube, definition, ExternId, Comment, Activate, 0);
}

	
RuleInfo^ Connection::RuleCreate(String^ database, String^ cube, String^ definition, bool Activate) {
	return RuleCreate(database, cube, definition, nullptr, nullptr, Activate, 0);
};


RuleInfo^ Connection::RuleModify(String^ database, String^ cube, long id, String^ definition, String^ ExternId, String^ Comment, bool Activate, double Position) {
	libpalo_err err;
	arg_rule_info_w ruleinfo;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pcube = PtrToStringChars(cube);
	pin_ptr<const wchar_t> pdefinition = nullptr;
	pin_ptr<const wchar_t> pExternId = nullptr;
	pin_ptr<const wchar_t> pComment = nullptr;

	if (ExternId != nullptr) {
		pExternId = PtrToStringChars(ExternId);
	}

	if (Comment != nullptr) {
		pComment = PtrToStringChars(Comment);
	}

	if (definition != nullptr) {
		pdefinition = PtrToStringChars(definition);
	}

	palo_rule_modify_w_r(&err, &ruleinfo, pso, NULL, pdatabase, pcube, id, pdefinition, 0, pExternId, pComment,  (Activate) ? 1 : 0, Position);

	if (!LIBPALO_SUCCESSFUL(err.result)) {
		throwException(err);
	}

	return getRuleInfoAndFree_arg_rule_info_w(ruleinfo);

};


RuleInfo^ Connection::RuleModify(String^ database, String^ cube, long id, String^ definition, String^ ExternId, String^ Comment, bool Activate) {
	return RuleModify(database, cube, id, definition, ExternId, Comment, Activate, 0);
};


RuleInfo^ Connection::RuleModify(String^ database, String^ cube, long id, String^ definition, bool Activate) {
	return RuleModify(database, cube, id, definition, nullptr, nullptr, Activate, 0);
};


array<RuleInfo^>^ Connection::RulesMove(String^ database, String^ cube, array<long>^ ids, double startPosition, double belowPosition) {
    libpalo_err err;
    arg_rule_info_array_w ruleinfoa;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
    pin_ptr<const wchar_t> pcube = PtrToStringChars(cube);

	long i, vsize = ids->Length, *pids = NULL;

	if (vsize > 0) {
		pids = (long *)calloc(vsize, sizeof(long));
		if (pids == NULL) {
			throw gcnew System::OutOfMemoryException();
		}
		for (i = 0; i < vsize; i++) {
			*(pids + i) = ids[i];	
		}
	}

	palo_rules_move_w_r(&err, &ruleinfoa, pso, NULL, pdatabase, pcube, vsize, pids, startPosition, belowPosition );

	free(pids);

	if (!LIBPALO_SUCCESSFUL(err.result ) ) {
		throwException(err );
	}

	long len = (long)ruleinfoa.len;

    array<RuleInfo^>^ ria = gcnew array<RuleInfo^>(len);

    for (i = 0; i < len; i++) {
		ria[i] = getRuleInfo(*(ruleinfoa.a + i));
    }

	free_arg_rule_info_array_contents_w(&ruleinfoa);

	return ria;
};


array<RuleInfo^>^ Connection::RulesActivate(String^ database, String^ cube, array<long>^ ids, RuleActivationMode mode ) {
	libpalo_err err;
    arg_rule_info_array_w ruleinfoa;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
    pin_ptr<const wchar_t> pcube = PtrToStringChars(cube);

	long i, vsize = ids->Length, *pids = NULL;
	
	if (vsize > 0) {
		pids = (long *)calloc(vsize, sizeof(long));
		if (pids == NULL) {
			throw gcnew System::OutOfMemoryException();
		}
		for (i = 0; i < vsize; i++) {
			*(pids + i) = ids[i];	
		}
	}

	palo_rules_activate_w_r(&err, &ruleinfoa, pso, NULL, pdatabase, pcube, vsize, pids, (rule_activation_mode)mode);

	free(pids);

	if (!LIBPALO_SUCCESSFUL(err.result ) ) {
		throwException(err );
	}

	long len = (long)ruleinfoa.len;

    array<RuleInfo^>^ ria = gcnew array<RuleInfo^>(len);

    for (i = 0; i < len; i++) {
		ria[i] = getRuleInfo(*(ruleinfoa.a + i));
    }

	free_arg_rule_info_array_contents_w(&ruleinfoa);

	return ria;

};


RuleInfo^ Connection::RuleInformation(String^ database, String^ cube, long id) {
	libpalo_err err;
	arg_rule_info_w ruleinfo;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pcube = PtrToStringChars(cube);

	palo_rule_info_w_r(&err, &ruleinfo, pso, NULL, pdatabase, pcube, id, 0);

	if (!LIBPALO_SUCCESSFUL(err.result)) {
		throwException(err);
	}

	return getRuleInfoAndFree_arg_rule_info_w(ruleinfo);
};


RuleInfo^ Connection::CellRuleInformation(String^ database, String^ cube, array<String^>^ coordinates) {
	libpalo_err err;
	arg_rule_info_w ruleinfo;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pcube = PtrToStringChars(cube);

	ARRAY_MARSHAL_CONVERT(acoordinates, coordinates)
	palo_cell_rule_info_w_r(&err, &ruleinfo, pso, NULL, pdatabase, pcube, acoordinates);
	ARRAY_MARSHAL_CLEANUP(acoordinates);

	if (!LIBPALO_SUCCESSFUL(err.result)) {
		throwException(err);
	}

	return getRuleInfoAndFree_arg_rule_info_w(ruleinfo);

}


String^ Connection::RuleParse(String^ database, String^ cube, String^ definition) {
	libpalo_err err;
	wchar_t *xmldef;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pcube = PtrToStringChars(cube);
	pin_ptr<const wchar_t> pdefinition = PtrToStringChars(definition);

	palo_rule_parse_w_r(&err, &xmldef, pso, NULL, pdatabase, pcube, pdefinition);

	if (!LIBPALO_SUCCESSFUL(err.result)) {
		throwException(err);
	}

	return Helper::CharPtrToStringFree(xmldef);

};


void Connection::RuleDelete(String^ database, String^ cube, long id) {
	libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pcube = PtrToStringChars(cube);

	palo_rule_delete_w_r(&err, pso, NULL, pdatabase, pcube, id);

	if (!LIBPALO_SUCCESSFUL(err.result)) {
		throwException(err);
	}

};


void Connection::RulesDelete(String^ database, String^ cube, array<long>^ ids) {
    libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
    pin_ptr<const wchar_t> pcube = PtrToStringChars(cube);

	long i, vsize = ids->Length, *pids = NULL;

	if (vsize > 0) {
		pids = (long *)calloc(vsize, sizeof(long));
		if (pids == NULL) {
			throw gcnew System::OutOfMemoryException();
		}
		for (i = 0; i < vsize; i++) {
			*(pids + i) = ids[i];	
		}
	}

	palo_rules_delete_w_r(&err, pso, NULL, pdatabase, pcube, vsize, pids );

	free(pids);

	if (!LIBPALO_SUCCESSFUL(err.result ) ) {
		throwException(err );
	}

};


array<RuleInfo^>^ Connection::ListRules(String^ database, String^ cube) {
    libpalo_err err;
    arg_rule_info_array_w ruleinfoa;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
    pin_ptr<const wchar_t> pcube = PtrToStringChars(cube);

    palo_list_rules_w_r(&err, &ruleinfoa, pso, NULL, pdatabase, pcube, 0);

    if (!LIBPALO_SUCCESSFUL(err.result)) {
		throwException(err);
    }

	long i = 0, len = (long)ruleinfoa.len;

	array<RuleInfo^>^ ria = gcnew array<RuleInfo^>(len);

    for (i = 0; i < len; i++) {
		ria[i] = getRuleInfo(*(ruleinfoa.a + i));
    }

	free_arg_rule_info_array_contents_w(&ruleinfoa);

	return ria;
};
// End Cube related


// Start Cell related
void Connection::GetData(String^ %str_result, double %dbl_result, String^ database, String^ cube, array<String^>^ coordinates) { 
	libpalo_err err;
	struct arg_palo_value_w val;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pcube = PtrToStringChars(cube);

	ARRAY_MARSHAL_CONVERT(acoordinates, coordinates)
	palo_getdata_w_r(&err, &val, pso, NULL, pdatabase, pcube, acoordinates);
	ARRAY_MARSHAL_CLEANUP(acoordinates);

	if (!LIBPALO_SUCCESSFUL(err.result)) {
		throwException(err);
	}

	if (val.type == argPaloValueTypeDouble) {
		dbl_result = val.val.d;
		str_result = nullptr;
	} else {
		if (val.type == argPaloValueTypeStr) {
			str_result = Helper::CharPtrToStringFree(val.val.s);
		} else {
			throwException("returned value is of invalid type.");
		}
	}
}


array<CellValue>^ Connection::GetDataMulti(String^ database, String^ cube, array<array<String^>^>^ elements) {
    libpalo_err err;
    struct arg_palo_value_array_w result;
    
	pin_ptr<struct sock_obj> pso = so;
    pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
    pin_ptr<const wchar_t> pcube = PtrToStringChars(cube);

    // convert elements
    struct arg_str_array_2d_w lib_elements = ManagedStringArrayArray2arg_str_array_2d_w(elements);

    palo_getdata_multi_w_r(&err, &result, pso, NULL, pdatabase, pcube, lib_elements);

	// cleanup
    FreeConverted_arg_str_array_2d_w(lib_elements);

    if (!LIBPALO_SUCCESSFUL(err.result)) {
		throwException(err);
	}

	// convert return value
	array<CellValue>^ cva;
    try {
	    long i, len = (long)result.len;
	    cva = gcnew array<CellValue>(len);

		for (i = 0; i < len; i++) {
		    cva[i] = Helper::ArgPaloValueToCellValue(result.a[i]);
	    }
    } catch (const Exception^ e) {
		free_arg_palo_value_array_contents_w(&result);
	    throw e;
    }

    free_arg_palo_value_array_contents_w(&result);

	return cva;
}


array<CellValue>^ Connection::GetDataArea(String^ database, String^ cube, array<array<String^>^>^ elements) {
    libpalo_err err;
    struct arg_palo_value_array_w result;
    
	pin_ptr<struct sock_obj> pso = so;
    pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
    pin_ptr<const wchar_t> pcube = PtrToStringChars(cube);

    // convert elements
    struct arg_str_array_array_w lib_elements = ManagedStringArrayArray2arg_str_array_array_w(elements);

	palo_getdata_area_w_r(&err, &result, pso, NULL, pdatabase, pcube, lib_elements, PALO_TRUE);

	// cleanup
    FreeConverted_arg_str_array_array_w(lib_elements);

    if (!LIBPALO_SUCCESSFUL(err.result)) {
		throwException(err);
	}

	// convert return value
	array<CellValue>^ cva;
    try {
	    long i, len = (long)result.len;
	    cva = gcnew array<CellValue>(len);

		for (i = 0; i < len; i++) {
		    cva[i] = Helper::ArgPaloValueToCellValue(result.a[i]);
	    }
    } catch (const Exception^ e) {
		free_arg_palo_value_array_contents_w(&result);
	    throw e;
    }

    free_arg_palo_value_array_contents_w(&result);

	return cva;
}


void Connection::SetData(String^ database, String^ cube, array<String^>^ coordinates, String^ str_value, double dbl_value, SplashMode mode, bool add, bool eventprocessor, array<array<String^>^>^ locked_elements) {
	libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pcube = PtrToStringChars(cube);
	pin_ptr<const wchar_t> pstr_value = (str_value != nullptr) ? PtrToStringChars(str_value) : nullptr;

	bool LockedCells = (locked_elements != nullptr);
	struct arg_str_array_2d_w lib_locked_elements;

	if (LockedCells) {
		lib_locked_elements = ManagedStringArrayArray2arg_str_array_2d_w(locked_elements);
	}
	
	ARRAY_MARSHAL_CONVERT(acoordinates, coordinates)
	palo_setdata_extended_w_r(&err, pso, NULL, pdatabase, pcube, libpalo_make_arg_palo_dataset_w(acoordinates, libpalo_make_arg_palo_value_w(pstr_value, dbl_value)), (splash_mode)mode, add, eventprocessor, (LockedCells) ? &lib_locked_elements : NULL);
	ARRAY_MARSHAL_CLEANUP(acoordinates)

	if (LockedCells) {
		FreeConverted_arg_str_array_2d_w(lib_locked_elements);
	}

	if (!LIBPALO_SUCCESSFUL(err.result)) {
		throwException(err);
	}
}


void Connection::SetData(String^ database, String^ cube, array<String^>^ coordinates, String^ str_value, double dbl_value, SplashMode mode) {
	SetData(database, cube, coordinates, str_value, dbl_value, mode, false, true, nullptr);
}


void Connection::SetData(String^ database, String^ cube, array<String^>^ coordinates, String^ str_value) {
	SetData(database, cube, coordinates, str_value, 0.0, SplashMode::SplashModeDisable);
}


void Connection::SetData(String^ database, String^ cube, array<String^>^ coordinates, double dbl_value, SplashMode mode) {
	SetData(database, cube, coordinates, nullptr, dbl_value, mode);
}


void Connection::SetData(String^ database, String^ cube, array<String^>^ coordinates, double dbl_value) {
	SetData(database, cube, coordinates, dbl_value, SplashMode::SplashModeDisable);
}


void Connection::SetDataMulti(String^ database, String^ cube, array<DataSetMulti>^ dsa, SplashMode mode, bool Add, bool EventProccessor, array<array<String^>^>^ locked_elements) {
	libpalo_err err;
	struct arg_palo_dataset_array_w palo_dsa;
	unsigned long i;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pcube = PtrToStringChars(cube);

	palo_dsa.len = dsa->Length;

	if (palo_dsa.len > 0) {

		palo_dsa.a = (struct arg_palo_dataset_w*)calloc(palo_dsa.len , sizeof(struct arg_palo_dataset_w));

		if (palo_dsa.a == NULL) {
			throw gcnew System::OutOfMemoryException();
		}

		for (i = 0; i < palo_dsa.len; i++) {
			*(palo_dsa.a + i) = libpalo_make_arg_palo_dataset_w(ManagedStrArray2arg_str_array_w(dsa[i].Coordinates) , libpalo_make_arg_palo_value_w(reinterpret_cast<wchar_t*>(Marshal::StringToHGlobalUni(dsa[i].Value.s).ToPointer()), dsa[i].Value.d));
		}

		bool LockedCells = (locked_elements != nullptr);
		struct arg_str_array_2d_w lib_locked_elements;

		if (LockedCells) {
			lib_locked_elements = ManagedStringArrayArray2arg_str_array_2d_w(locked_elements);
		}

		palo_setdata_multi_extended_w_r(&err, pso, NULL, pdatabase, pcube, palo_dsa, (splash_mode)mode, Add, EventProccessor, (LockedCells) ? &lib_locked_elements : NULL);

		if (LockedCells) {
			FreeConverted_arg_str_array_2d_w(lib_locked_elements);
		}

		for (i = 0; i < palo_dsa.len; i++) {
			FreeConverted_arg_str_array_w((palo_dsa.a + i)->coordinates);
			if((palo_dsa.a + i)->value.type == PaloValueTypeStr){
				Marshal::FreeHGlobal(IntPtr((palo_dsa.a + i)->value.val.s));
			}
		}

		free(palo_dsa.a);


		if (!LIBPALO_SUCCESSFUL(err.result)) {
			throwException(err);
		}
	}
};

void Connection::SetDataMulti(String^ database, String^ cube, array<DataSetMulti>^ dsa, SplashMode mode, bool Add, bool EventProccessor) {
	SetDataMulti(database, cube, dsa, mode, Add, EventProccessor, nullptr);
}

void Connection::CopyCell(CopyCellMode mode, String^ database, String^ cube, array<String^>^ from, array<String^>^ to, array<array<String^>^>^ predict_area, array<array<String^>^>^ locked_elements, double^ dbl_value, bool use_rules) {
	libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pcube = PtrToStringChars(cube);

	bool LockedCells = (locked_elements != nullptr);
	bool isDefault = (mode == CopyCellMode::CopyCellDefault);
	bool FromUsed = (isDefault && (from != nullptr));
	bool PredictUsed = (!isDefault && (predict_area != nullptr));  
	bool ValueUsed = (isDefault && (dbl_value != nullptr));
	struct arg_str_array_2d_w lib_locked_elements;
	struct arg_str_array_array_w lib_predict_area;
	double val = (ValueUsed) ? *dbl_value : 0;

	if (LockedCells) {
		lib_locked_elements = ManagedStringArrayArray2arg_str_array_2d_w(locked_elements);
	}

	if (PredictUsed) {
		lib_predict_area = ManagedStringArrayArray2arg_str_array_array_w(predict_area);
	}

	if (from == nullptr) {
		from = gcnew array<String^>(0);
	}

	ARRAY_MARSHAL_CONVERT(afrom, from)
	ARRAY_MARSHAL_CONVERT(ato, to)
	palo_cell_copy_extended_w_r(&err, pso, NULL, (cell_copy_mode)mode , pdatabase, pcube, (FromUsed) ? &afrom : NULL, &ato, (PredictUsed) ? &lib_predict_area : NULL, (LockedCells) ? &lib_locked_elements : NULL, (ValueUsed) ? &val : NULL , use_rules);
	ARRAY_MARSHAL_CLEANUP(afrom)
	ARRAY_MARSHAL_CLEANUP(ato)

	if (LockedCells) {
		FreeConverted_arg_str_array_2d_w(lib_locked_elements);
	}

	if (PredictUsed) {
		FreeConverted_arg_str_array_array_w(lib_predict_area);
	}

	if (!LIBPALO_SUCCESSFUL(err.result)) {
		throwException(err);
	}
}


void Connection::CopyCell(String^ database, String^ cube, array<String^>^ from, array<String^>^ to) {
	CopyCell(CopyCellMode::CopyCellDefault, database, cube, from, to, nullptr, nullptr, nullptr, false);
}


void Connection::CopyCell(String^ database, String^ cube, array<String^>^ from, array<String^>^ to, double dbl_value) {
	CopyCell(CopyCellMode::CopyCellDefault, database, cube, from, to, nullptr, nullptr, dbl_value, false);
}


void Connection::GoalSeek(String^ database, String^ cube, array<String^>^ path, double dbl_value, GoalSeekMode mode, array<array<String^>^>^ goalseek_area) {
	libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
	pin_ptr<const wchar_t> pcube = PtrToStringChars(cube);
	bool AreaNotNull = (goalseek_area != nullptr);
	struct arg_str_array_array_w lib_goalseek_area;

	if (AreaNotNull) {
		lib_goalseek_area = ManagedStringArrayArray2arg_str_array_array_w(goalseek_area);
	}

	ARRAY_MARSHAL_CONVERT(apath, path)
	palo_cell_goalseek_extended_w_r(&err, pso, NULL, pdatabase, pcube, &apath, dbl_value, (goalseek_mode)mode, (AreaNotNull) ? &lib_goalseek_area : NULL);
	ARRAY_MARSHAL_CLEANUP(apath)

	if (AreaNotNull) {
		FreeConverted_arg_str_array_array_w(lib_goalseek_area);
	}

	if (!LIBPALO_SUCCESSFUL(err.result)) {
		throwException(err);
	}
}


void Connection::GoalSeek(String^ database, String^ cube, array<String^>^ path, double dbl_value) {
	GoalSeek(database, cube, path, dbl_value, GoalSeekMode::GoalSeekComplete, nullptr);
}


LockInfo^ Connection::CubeLock(String^ database, String^ cube, array<array<String^>^>^ area) {
    libpalo_err err;
    struct arg_lock_info_w result;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
    pin_ptr<const wchar_t> pcube = PtrToStringChars(cube);

    // convert elements
    struct arg_str_array_array_w lib_area = ManagedStringArrayArray2arg_str_array_array_w(area);

    palo_cube_lock_w_r(&err, &result, pso, NULL, pdatabase, pcube, lib_area);

    // cleanup
    FreeConverted_arg_str_array_array_w(lib_area);

    if (!LIBPALO_SUCCESSFUL(err.result)) {
		throwException(err);
    }

	LockInfo^ li = getLockInfo(result);
	
    free_arg_lock_info_contents_w(&result);

    return li;
}


array<LockInfo^>^ Connection::CubeLocks(String^ database, String^ cube) {
    libpalo_err err;
	struct arg_lock_info_array_w result;
	long i, lsize;

	pin_ptr<struct sock_obj> pso = so;
    pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
    pin_ptr<const wchar_t> pcube = PtrToStringChars(cube);

    palo_cube_locks_w_r(&err, &result, pso, NULL, pdatabase, pcube);

    if (!LIBPALO_SUCCESSFUL(err.result)) {
		throwException(err);
    }

	lsize = (long)result.len;

	array<LockInfo^>^ lia = gcnew array<LockInfo^>(lsize);
	for (i=0; i < lsize; i++) {
		lia[i] = getLockInfo(*(result.a+i));
	}
	
    free_arg_lock_info_array_contents_w(&result);

    return lia;
}


void Connection::CubeRollback(String^ database, String^ cube, long lockid, long steps){
    libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
    pin_ptr<const wchar_t> pcube = PtrToStringChars(cube);

    palo_cube_rollback_w_r(&err, pso, NULL, pdatabase, pcube, lockid, steps);

    if (!LIBPALO_SUCCESSFUL(err.result)) {
		throwException(err);
    }
}	


void Connection::CubeRollback(String^ database, String^ cube, long lockid){
	CubeRollback(database, cube, lockid, -1);
}	


void Connection::CubeCommit(String^ database, String^ cube, long lockid){
    libpalo_err err;

	pin_ptr<struct sock_obj> pso = so;
	pin_ptr<const wchar_t> pdatabase = PtrToStringChars(database);
    pin_ptr<const wchar_t> pcube = PtrToStringChars(cube);

    palo_cube_commit_w_r(&err, pso, NULL, pdatabase, pcube, lockid);

    if (!LIBPALO_SUCCESSFUL(err.result)) {
		throwException(err);
    }
}	


// use directly libpalo_ng
array<DrillThroughSet>^ Connection::FastCellDrillThrough(String^ database, String^ cube, array<String^>^ coordinates , DrillThroughType mode) {
	ErrorInformation^ ErrorInfo = gcnew ErrorInformation();
    array<DrillThroughSet>^ dtsa = nullptr;

	pin_ptr<struct sock_obj> pso = so;
	std::string sdatabase, scube;
	std::vector<std::string> scoordinates;

	Helper::ConvertStringToUTF8(sdatabase, database);
	Helper::ConvertStringToUTF8(scube, cube);
	Helper::ConvertStringArrayToUTF8(scoordinates, coordinates);

	try {
		std::vector<DRILLTHROUGH_INFO> cdt;
		(*(pso->myServer))[sdatabase].cube[scube].CellDrillThrough(cdt, scoordinates, Cube::DRILLTHROUGH_TYPE(mode)); 
		scoordinates.clear();

		// commented clear because of 19306

		long i, len = (long)cdt.size();
	    dtsa = gcnew array<DrillThroughSet>(len);
		for (i = 0; i < len; i++) { 
			dtsa[i].Items = Helper::ConvertUTF8ToStringArray(cdt[i].line);
//			cdt[i].line.clear();
		}
//		cdt.clear();
	} catch (const SocketException& exp) {
		ErrorInfo->ProcessSocketError(exp.what());
	} catch (const PaloServerException& exp) {
		ErrorInfo->ProcessError(exp.code(), exp.Description());
	} catch (const std::exception& exp) {
		ErrorInfo->ProcessStandardError(exp.what());
	} catch (const System::Exception^) {
		throw;
#ifndef _DEBUG
	} catch (...) {
		ErrorInfo->ProcessUnknownError();
#endif
	}

	ErrorInfo->Check();

    return dtsa;
}


// use directly libpalo_ng
array<Dataset>^ Connection::GetDataExport(String^ database, String^ cube, array<array<String^>^>^ area, GetDataExportOptions opts, bool useRules, double %progress) {

	ErrorInformation^ ErrorInfo = gcnew ErrorInformation();
	array<Dataset>^ dsa = nullptr;
	progress = 0.0;

	pin_ptr<struct sock_obj> pso = so;
	std::string sdatabase, scube, scondition;
	std::vector<std::string> slast;
	std::vector<std::vector<std::string> > sarea;

	Helper::ConvertStringToUTF8(sdatabase, database);
	Helper::ConvertStringToUTF8(scube, cube);
	Helper::ConvertStringArrayToUTF8(slast, opts.LastCoordinates);
	Helper::ConvertStringArrayArrayToUTF8(sarea, area);
	
	arg_getConditionString(opts.Filter, scondition);

	try {
		long dbid = (*(pso->myServer))[sdatabase].getCacheData().database;
		long cubeid = (*(pso->myServer))[dbid].cube[scube].getCacheData().cube;
		std::vector<CELL_VALUE_EXPORTED> cve;
		(*(pso->myServer))[dbid].cube[cubeid].CellExport(cve, sarea, opts.NumDatasets, slast, scondition, opts.BaseElementsOnly, opts.IngoreEmptyCells, useRules);

		long i, j, len = (long)cve.size(), len2;

		if (len > 0) {
			len = len - 1;
			long double	denominator = cve[len].exportinfo.allcells;
			if (denominator == 0) {
				denominator = 1;
			}
			long double	counter = cve[len].exportinfo.usedcells;
			progress = counter / denominator;
			if (progress > 1) {
				progress = 1;
			}
		}

		DIMENSION_LIST cubedims = (*(pso->myServer))[dbid].cube[cubeid].getCacheData().dimensions;
	    dsa = gcnew array<Dataset>(len);

	    for (i = 0; i < len; i++) {
			len2 = (long)cve[i].cvp.path.size();
			dsa[i].Coordinates = gcnew array<String^>(len2);
			
			for (j = 0; j < len2; j++) {
				dsa[i].Coordinates[j] = Helper::ConvertUTF8ToString((*(pso->myServer))[dbid].dimension[cubedims[j]][cve[i].cvp.path[j]].getCacheData().nelement);
			}

			switch (cve[i].cvp.type) {
				case CELL_VALUE::NUMERIC:
					dsa[i].Value.Type = CellValueType::CellValueTypeDouble;
					dsa[i].Value.Value.StrValue = nullptr;
					dsa[i].Value.Value.DblValue = cve[i].cvp.val.d;
					break;

				case CELL_VALUE::STRING:
					dsa[i].Value.Type = CellValueType::CellValueTypeString;
					dsa[i].Value.Value.DblValue = 0;
					dsa[i].Value.Value.StrValue = Helper::ConvertUTF8ToString(cve[i].cvp.val.s);
					break;

				case CELL_VALUE::ERROR:
					dsa[i].Value.Type = CellValueType::CellValueTypeError;
					dsa[i].Value.Value.DblValue = 0;
					dsa[i].Value.Value.StrValue = Helper::ConvertUTF8ToString(cve[i].cvp.val.s);
					break;

				default:
					LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_CELLVALUEINFOTYPE);
			}
	    }
	} catch (const SocketException& exp) {
		ErrorInfo->ProcessSocketError(exp.what());
	} catch (const PaloServerException& exp) {
		ErrorInfo->ProcessError(exp.code(), exp.Description());
	} catch (const std::exception& exp) {
		ErrorInfo->ProcessStandardError(exp.what());
	} catch (const System::Exception^) {
		throw;
#ifndef _DEBUG
	} catch (...) {
		ErrorInfo->ProcessUnknownError();
#endif
	}

	ErrorInfo->Check();

    return dsa;
}


array<Dataset>^ Connection::GetDataExport(String^ database, String^ cube, array<array<String^>^>^ area, GetDataExportOptions opts, double %progress) {
    return GetDataExport(database, cube, area, opts, false, progress);
}


array<Dataset>^ Connection::GetDataExport(String^ database, String^ cube, array<array<String^>^>^ area, GetDataExportOptions opts) {
    double d;
    return GetDataExport(database, cube, area, opts, false, d);
}
// End Cell related
// End Connection Methods


// Start Helper Methods
String^ Helper::CharPtrToStringFree(wchar_t *s) {
	String^ S = gcnew String(s);
	free_arg_str_w(s);
	return S;
}


array<String^>^ Helper::ArgStringArrayToManaged(struct arg_str_array_w a) {
	long i, len = (long)a.len;
    array<String^>^ sa = gcnew array<String^>(len);

    for (i = 0; i < len; i++) {
		sa[i] = gcnew String(a.a[i]);
    }

    return sa;
}


array<String^>^ Helper::ArgStringArrayToManagedFree(struct arg_str_array_w a) {
    array<String^>^ sa = ArgStringArrayToManaged(a);

    free_arg_str_array_contents_w(&a);

    return sa;
}


void Helper::ManagedToArgStringArray(array<String^>^ src, struct arg_str_array_w &dest) {
	dest.len = src->Length;
	if (dest.len > 0) { 
		dest.a = (wchar_t**)calloc(dest.len, sizeof(wchar_t*));

		if (dest.a == NULL) {
			throw gcnew System::OutOfMemoryException();
		}

		for (unsigned long i = 0; i < dest.len; i++) {
			dest.a[i] = reinterpret_cast<wchar_t*>(Marshal::StringToHGlobalUni(src[i]).ToPointer());
		}
	}
}


void Helper::FreeConvertedArgStringArray(struct arg_str_array_w &tofree) {
	if (tofree.len > 0) { 
		for (unsigned long i = 0; i < tofree.len; i++) {
			Marshal::FreeHGlobal(IntPtr(tofree.a[i])); 
		}
		free(tofree.a);
		tofree.a = NULL;
		tofree.len = 0;
	}
}


/* needed because Managed C++ seems not to support jagged arrays (why?) */
array<String^>^ Helper::ManagedJaggedArrayMemberToStringArray(array<String^>^ a) {
    array<String^>^ sa = gcnew array<String^>(a->Length);

    for (long i = 0; i < a->Length; i++) {
		sa[i] = static_cast<String^>(a->GetValue(i));
    }

    return sa;
}


CellValue Helper::ArgPaloValueToCellValue(struct arg_palo_value_w pv) {
	CellValue v;

	switch (pv.type) {
		case argPaloValueTypeDouble:
			v.Type = CellValueType::CellValueTypeDouble;
			v.Value.DblValue = pv.val.d;
			break;

		case argPaloValueTypeStr:
			v.Type = CellValueType::CellValueTypeString;
			v.Value.DblValue = pv.val.d;
			v.Value.StrValue = gcnew String(pv.val.s);
			break;

		case argPaloValueTypeError:
			v.Type = CellValueType::CellValueTypeError;
			v.Value.DblValue = pv.val.d;
			v.Value.StrValue = gcnew String(pv.val.s);
			break;

	}

	return v;
}


String^ Helper::GetErrorMessage(libpalo_err err) {
	wchar_t *errstr = libpalo_err_get_string_w(&err);

	return (errstr == NULL) ? "could not get error information." : CharPtrToStringFree(errstr);
}


void Helper::HandleConversionError(bool ToUtf8) {
	DWORD LastError = GetLastError();
	if (LastError != ERROR_INSUFFICIENT_BUFFER) {
		if (ToUtf8) {
			throw gcnew System::SystemException("WideCharToMultiByte failed with Error=" + LastError);
		} else {
			throw gcnew System::SystemException("MultiByteToWideChar failed with Error=" + LastError);
		}

	} else {
		throw gcnew System::OutOfMemoryException;
	}
}


void Helper::wcs2utf8(char **utf8_str, const wchar_t *s, size_t len) {
	size_t _len = WideCharToMultiByte(CP_UTF8, 0, s, (int)len, NULL, 0, NULL, NULL);

	if (_len == 0) {
		HandleConversionError(true);
	}

	/* len in bytes */
	*utf8_str = (char*)malloc(_len + 1);
	if (*utf8_str == NULL) {
		throw gcnew System::OutOfMemoryException;
	}
	
	if (WideCharToMultiByte(CP_UTF8, 0, s, (int)len, *utf8_str, SIZE_T_TO_MAX_INT(_len), NULL, NULL) == 0) {
		free(*utf8_str);
		HandleConversionError(true);
	}

	(*utf8_str)[_len] = '\0';
}


void Helper::ConvertStringToUTF8(std::string& To, String^ From) {
	if (String::IsNullOrEmpty(From)) {
		To.clear();
	} else {
		char * pTo = NULL;
		pin_ptr<const wchar_t> wFrom = PtrToStringChars(From);

		wcs2utf8(&pTo, wFrom, wcslen(wFrom));
		To = pTo;
		free(pTo);
	}
}


void Helper::ConvertStringArrayToUTF8(std::vector<std::string>& To, array<String^>^ From) {
	long i, len = From->Length;

	To.clear();
	To.resize(len);
	for (i = 0; i < len; i++) {
		ConvertStringToUTF8(To[i], From[i]);
	}
}


void Helper::ConvertStringArrayArrayToUTF8(std::vector<std::vector<std::string> >& To, array<array<String^>^>^ From) {
	long i, len = From->Length;

	To.clear();
	To.resize(len);
	for (i = 0; i < len; i++) {
		ConvertStringArrayToUTF8(To[i], From[i]);
	}
}


void Helper::utf82wcs(wchar_t **wcs, const char *utf8_str) {
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, NULL, 0);

	if ((len = MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, NULL, 0)) == 0) {
		HandleConversionError(false);
	}

	/* len in wchar's */
	*wcs = (wchar_t*)malloc(len*sizeof(wchar_t));
	if (*wcs == NULL) {
		throw gcnew System::OutOfMemoryException;
	}

	if (MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, *wcs, SIZE_T_TO_MAX_INT(len)) == 0) {
		free(*wcs);
		HandleConversionError(false);
	}
}


String^ Helper::ConvertUTF8ToString(const std::string& From) {
	String^ To = nullptr;
	if (From.size() == 0) {
		To = gcnew String(L"");
	} else {
		wchar_t *wFrom = NULL;

		utf82wcs(&wFrom, From.c_str());
		To = gcnew String(wFrom);
		free(wFrom);
	}

	return To;
}


array<String^>^ Helper::ConvertUTF8ToStringArray(const std::vector<std::string>& From) {
	long i, len = (long)From.size();
	array<String^>^ To = gcnew array<String^>(len);

	for (i = 0; i < len; i++) {
		To[i] = ConvertUTF8ToString(From[i]);
	}
	return To;
}

void Jedox::Palo::Comm::Helper::ConvertDArrayArrayToDVector(std::vector< std::vector<double> >& To, array<array<double>^>^ From){
	size_t i, j, len = From->Length, len2;
	std::vector<double> _tmp;
	To.reserve(len);
	for (i = 0; i < len; ++i) {
		len2 = From[i]->Length;
		_tmp.reserve(len2);
		for (j = 0; j < len2; ++j) {
			_tmp.push_back(From[i][j]);
		}
		To.push_back(std::move(_tmp));
	}
}
// End Helper Methods


// Start ErrorInformation Methods
int ErrorInformation::GetCode() {
	return ErrorCode;
}


int ErrorInformation::GetOriginalCode() {
	return OriginalErrorCode;
}


String^ ErrorInformation::GetMessage() {
	return ErrorMessage;
}


void ErrorInformation::Check() {
	if (ErrorCode != 0) {
		throw gcnew PaloException(ErrorMessage, ErrorCode, OriginalErrorCode);
	}
}


void ErrorInformation::ProcessErrorCode() {
	switch (OriginalErrorCode) {
		case PaloExceptionFactory::ERROR_ID_NOT_FOUND:
			ErrorCode = PALO_ERR_ID_NOT_FOUND_ID;
			break;

		case PaloExceptionFactory::ERROR_INVALID_FILENAME:
			ErrorCode = PALO_ERR_INVALID_FILENAME_ID;
			break;

		case PaloExceptionFactory::ERROR_MKDIR_FAILED:
			ErrorCode = PALO_ERR_MKDIR_ID;
			break;

		case PaloExceptionFactory::ERROR_RENAME_FAILED:
			ErrorCode = PALO_ERR_FILE_RENAME_ID;
			break;

		case PaloExceptionFactory::ERROR_AUTHORIZATION_FAILED:
			ErrorCode = PALO_ERR_AUTH_ID;
			break;

		case PaloExceptionFactory::ERROR_INVALID_TYPE:
			ErrorCode = PALO_ERR_TYPE_ID;
			break;

		case PaloExceptionFactory::ERROR_INVALID_COORDINATES:
			ErrorCode = PALO_ERR_INV_ARG_ID;
			break;

		case PaloExceptionFactory::ERROR_FILE_NOT_FOUND_ERROR:
			ErrorCode = PALO_ERR_FILE_ID;
			break;

		case PaloExceptionFactory::ERROR_NOT_AUTHORIZED:
			ErrorCode = PALO_ERR_AUTH_ID;
			break;

		case PaloExceptionFactory::ERROR_CORRUPT_FILE:
			ErrorCode = PALO_ERR_FILE_ID;
			break;

		case PaloExceptionFactory::ERROR_PARAMETER_MISSING:
			ErrorCode = PALO_ERR_INV_ARG_ID;
			break;

		case PaloExceptionFactory::ERROR_INVALID_SPLASH_MODE:
			ErrorCode = PALO_ERR_INV_ARG_ID;
			break;

		case PaloExceptionFactory::ERROR_API_CALL_NOT_IMPLEMENTED:
			ErrorCode = PALO_ERR_NOT_IMPLEMENTED_ID;
			break;

		case PaloExceptionFactory::ERROR_DATABASE_NOT_FOUND:
			ErrorCode = PALO_ERR_DATABASE_NOT_FOUND_ID;
			break;

		case PaloExceptionFactory::ERROR_DATABASE_STILL_LOADED:
			ErrorCode = PALO_ERR_ALREADY_LOADED_ID;
			break;

		case PaloExceptionFactory::ERROR_INVALID_DATABASE_NAME:
			ErrorCode = PALO_ERR_INV_DATABASE_NAME_ID;
			break;

		case PaloExceptionFactory::ERROR_DATABASE_NAME_IN_USE:
			ErrorCode = PALO_ERR_IN_USE_ID;
			break;

		case PaloExceptionFactory::ERROR_DIMENSION_NOT_FOUND:
			ErrorCode = PALO_ERR_DIMENSION_NOT_FOUND_ID;
			break;

		case PaloExceptionFactory::ERROR_DIMENSION_EMPTY:
			ErrorCode = PALO_ERR_DIM_EMPTY_ID;
			break;

		case PaloExceptionFactory::ERROR_DIMENSION_EXISTS:
			ErrorCode = PALO_ERR_DIMENSION_EXISTS_ID;
			break;

		case PaloExceptionFactory::ERROR_INVALID_DIMENSION_NAME:
			ErrorCode = PALO_ERR_INV_DIMENSION_NAME_ID;
			break;

		case PaloExceptionFactory::ERROR_DIMENSION_NAME_IN_USE:
			ErrorCode = PALO_ERR_IN_USE_ID;
			break;

		case PaloExceptionFactory::ERROR_DIMENSION_IN_USE:
			ErrorCode = PALO_ERR_IN_USE_ID;
			break;

		case PaloExceptionFactory::ERROR_ELEMENT_NOT_FOUND:
			ErrorCode = PALO_ERR_DIM_ELEMENT_NOT_FOUND_ID;
			break;

		case PaloExceptionFactory::ERROR_ELEMENT_EXISTS:
			ErrorCode = PALO_ERR_DIMENSION_ELEMENT_EXISTS_ID;
			break;

		case PaloExceptionFactory::ERROR_ELEMENT_CIRCULAR_REFERENCE:
			ErrorCode = PALO_ERR_CIRCULAR_REF_ID;
			break;

		case PaloExceptionFactory::ERROR_ELEMENT_NAME_IN_USE:
			ErrorCode = PALO_ERR_IN_USE_ID;
			break;

		case PaloExceptionFactory::ERROR_ELEMENT_NAME_NOT_UNIQUE:
			ErrorCode = PALO_ERR_NAME_NOT_UNIQUE_ID;
			break;

		case PaloExceptionFactory::ERROR_ELEMENT_NO_CHILD_OF:
			ErrorCode = PALO_ERR_DIM_ELEMENT_NOT_CHILD_ID;
			break;

		case PaloExceptionFactory::ERROR_INVALID_ELEMENT_NAME:
			ErrorCode = PALO_ERR_INV_DIMENSION_ELEMENT_NAME_ID;
			break;

		case PaloExceptionFactory::ERROR_INVALID_OFFSET:
			ErrorCode = PALO_ERR_INV_OFFSET_ID;
			break;

		case PaloExceptionFactory::ERROR_INVALID_ELEMENT_TYPE:
			ErrorCode = PALO_ERR_DIM_ELEMENT_INV_TYPE_ID;
			break;

		case PaloExceptionFactory::ERROR_INVALID_POSITION:
			ErrorCode = PALO_ERR_INV_OFFSET_ID;
			break;

		case PaloExceptionFactory::ERROR_CUBE_NOT_FOUND:
			ErrorCode = PALO_ERR_CUBE_NOT_FOUND_ID;
			break;

		case PaloExceptionFactory::ERROR_INVALID_CUBE_NAME:
			ErrorCode = PALO_ERR_INV_CUBE_NAME_ID;
			break;

		case PaloExceptionFactory::ERROR_CUBE_EMPTY:
			ErrorCode = PALO_ERR_EMPTY_CUBE_ID;
			break;

		case PaloExceptionFactory::ERROR_CUBE_NAME_IN_USE:
			ErrorCode = PALO_ERR_IN_USE_ID;
			break;

		case PaloExceptionFactory::ERROR_NET_ARG:
			ErrorCode = PALO_ERR_NET_ARG_ID;
			break;

		case PaloExceptionFactory::ERROR_INV_CMD:
			ErrorCode = PALO_ERR_INV_CMD_ID;
			break;

		case PaloExceptionFactory::ERROR_INV_CMD_CTL:
			ErrorCode = PALO_ERR_INV_CMD_CTL_ID;
			break;

		case PaloExceptionFactory::ERROR_NET_SEND:
			ErrorCode = PALO_ERR_NET_SEND_ID;
			break;

		case PaloExceptionFactory::ERROR_NET_CONN_TERM:
			ErrorCode = PALO_ERR_NET_CONN_TERM_ID;
			break;

		case PaloExceptionFactory::ERROR_NET_RECV:
			ErrorCode = PALO_ERR_NET_RECV_ID;
			break;

		case PaloExceptionFactory::ERROR_NET_HS_HALLO:
			ErrorCode = PALO_ERR_NET_HS_HELLO_ID;
			break;

		case PaloExceptionFactory::ERROR_NET_HS_PROTO:
			ErrorCode = PALO_ERR_NET_HS_PROTO_ID;
			break;

		case PaloExceptionFactory::ERROR_INV_ARG_COUNT:
			ErrorCode = PALO_ERR_INV_ARG_COUNT_ID;
			break;

		case PaloExceptionFactory::ERROR_INV_ARG_TYPE:
			ErrorCode = PALO_ERR_INV_ARG_TYPE_ID;
			break;

		case PaloExceptionFactory::ERROR_CLIENT_INV_NET_REPLY:
			ErrorCode = PALO_ERR_CLIENT_INV_NET_REPLY_ID;
			break;

		default:
			ErrorCode = OriginalErrorCode;
			break;
	}
}


void ErrorInformation::ProcessError(int OriginalError, const std::string& Message) {
	OriginalErrorCode = OriginalError;
	ProcessErrorCode();
	std::stringstream tmpstr;
	tmpstr << Message << " (Error " << ErrorCode << ")"; 
	ErrorMessage = Helper::ConvertUTF8ToString(tmpstr.str());
}


void ErrorInformation::ProcessSocketError(const std::string& Message) {
	ProcessError(PALO_ERR_NETWORK_ID, Message);
}


void ErrorInformation::ProcessStandardError(const std::string& Message) {
	ProcessError(PALO_ERR_SYSTEM_ID, Message);
}


void ErrorInformation::ProcessUnknownError() {
	ProcessStandardError("exception of unknown type occurred");
}
// End ErrorInformation Methods


// Start PaloCommException Methods
PaloCommException::PaloCommException(String^ message) : ApplicationException(message) {
}
// End PaloCommException Methods


// Start PaloException Methods
Jedox::Palo::Comm::PaloException::PaloException(libpalo_err err) : ApplicationException(Helper::GetErrorMessage(err)) {
	if (LIBPALO_SUCCESSFUL(err.result)) {
		throw gcnew System::Exception("Requested to throw an Exception where no error happened!");
	}

	Set((int)err.palo_error, err.original_error);
	libpalo_err_free_contents(&err);

	PaloMessage = Message;
}


Jedox::Palo::Comm::PaloException::PaloException(String^ Message, int Code) : ApplicationException(Message) {
	Set(Code, Code);
}


Jedox::Palo::Comm::PaloException::PaloException(String^ Message, int Code, int Orginalcode) : ApplicationException(Message) {
	Set(Code, Orginalcode);
}


void Jedox::Palo::Comm::PaloException::Set(int Code, int Originalcode) {
	ErrorCode = Code;
	OriginalErrorCode = Originalcode;
	PaloMessage = Message;
}
// End PaloException Methods
