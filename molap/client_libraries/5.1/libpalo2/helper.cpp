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

#if defined(WIN32) || defined(WIN64)
#pragma warning(disable : 4996)
#endif

#include <set>

#ifdef __UNIX__
#include <string.h>
#include <strings.h>
#endif 

#include <libpalo_ng/Network/NetInitialisation.h>

#include "libpalo2.h"
#include "helper.h"
#include "strtools.h"
#include "stdaliases.h"

std::set<std::string> trustfiles;
bool InitSSLok = false;

static const std::string PermissionArtStr[] = {	"user",
												"password", 
												"group",
												"database",
												"cube",
												"dimension",
												"dimension element",
												"cell data",
												"rights",
												"system operations",
												"event processor",
												"sub-set view",
												"user info",
												"rule",
												"drillthrough",
												"ste_reports",
												"ste_files",
												"ste_palo",
												"ste_users",
												"ste_etl",
												"ste_conns",
												"ste_scheduler",
												"ste_logs",
												"ste_licenses",
												"ste_mobile",
												"ste_analyzer",
												"ste_sessions",
												"ste_settings"};


palo_err number2types(long ntype, de_type *type) {
	palo_err result = PALO_SUCCESS;

	switch (ntype) {
		case numeric_element:
			*type = de_n;
			break;
		case string_element:
			*type = de_s;
			break;
		case consolidated_element:
			*type = de_c;
			break;
		default:
			*type = de_unknown;
			result = PALO_ERR_INV_ARG;
			break;
	}
	return result;
}


palo_err types2number(de_type type, ELEMENT_INFO::TYPE *ntype) {
	palo_err result = PALO_SUCCESS;

	switch (type) {
		case de_n:
			*ntype = ELEMENT_INFO::NUMERIC;
			break;
		case de_s:
			*ntype = ELEMENT_INFO::STRING;
			break;
		case de_c:
			*ntype = ELEMENT_INFO::CONSOLIDATED;
			break;
		default:
			*ntype = ELEMENT_INFO::UNKNOWN;
			result = PALO_ERR_INV_ARG;
			break;
	}
	return result;
}


palo_err number2valtypes(long ntype, arg_palo_value_type *type) {
	palo_err result = PALO_SUCCESS;

	switch (ntype) {
		case PaloValueTypeNumeric:
			*type = argPaloValueTypeDouble;
			break;
		case PaloValueTypeStr:
			*type = argPaloValueTypeStr;
			break;
		case PaloValueTypeError:
			*type = argPaloValueTypeError;
			break;
		default:
			*type = argPaloValueTypeError;
			result = PALO_ERR_INV_ARG;
			break;
	}
	return result;
}


palo_err splasmode2number(splash_mode mode, SPLASH_MODE *nmode) {
	palo_err result = PALO_SUCCESS;

	switch (mode) {
		case splash_mode_disable:
			*nmode = MODE_SPLASH_NONE;
			break;
		case splash_mode_default:
			*nmode = MODE_SPLASH_DEFAULT;
			break;
		case splash_mode_base_set:
			*nmode = MODE_SPLASH_SET;
			break;
		case splash_mode_base_add:
			*nmode = MODE_SPLASH_ADD;
			break;
		case splash_mode_unknown:
			*nmode = MODE_SPLASH_UNKNOWN;
			break;
		default:
			*nmode = MODE_SPLASH_UNKNOWN;
			result = PALO_ERR_INV_ARG;
			break;
	}
	return result;
}

unsigned short palo_bool2number(palo_bool value) {
	return (value == PALO_TRUE) ? 1 : 0;
}

palo_bool FindDimElementIndex(vector<ELEMENT_INFO> &elems, char *utf8_elem, size_t *index) {
	palo_bool retvalue = PALO_FALSE;
	size_t i, num;

	num = elems.size();
	for (i = 0; i < num;i++) {
		if (stricmp(elems[i].nelement.c_str(), utf8_elem) == 0) {
			*index = i;
			retvalue = PALO_TRUE;
			break;
		}
	}

	return retvalue;
}


palo_bool FindDimElementIndexById(vector<ELEMENT_INFO> &elems, long id, size_t *index) {
	size_t i, num;

	num = elems.size();
	for (i = 0; i < num;i++) {
		if (elems[i].element == id) {
			*index = i;
			return PALO_TRUE;
		}
	}

	return PALO_FALSE;
}


palo_err getDimElementSiblingIndex(wchar_t **errmsg, size_t *siblingindex, vector<ELEMENT_INFO> &elems,  size_t elemindex, int offset) {
	size_t i, vsize, vsize2, esize;
	size_t j, n;
	int skipped;

	*errmsg = wcsdup(L"hierarchy corrupted - this should never happen");

	vsize = elems[elemindex].parents.size();
	esize = elems.size();

	if (offset == 0) {
		*siblingindex = elemindex;
		return PALO_SUCCESS;
	}

	// root element
	if (vsize == 0) {
		n = elems[elemindex].position;
		skipped = 0;
		while (offset < 0 ? n > 1 : n < esize) {
			if (offset == skipped) {
				*siblingindex = n;
				return PALO_SUCCESS;
			}
			if (offset < 0) {
				n--;
				if ((n != -1) && elems[n].type == ELEMENT_INFO::CONSOLIDATED) {
					skipped--;
				}
			}					 /// offset>0
			else {
				n++;
				if ((n < esize)  && (elems[n].type == ELEMENT_INFO::CONSOLIDATED)) {
					skipped++;
				}
			}
		}
		*errmsg = wcsdup(L"invalid offset");
		return PALO_ERR_INV_ARG;
	}

	/// others
	skipped = 0;
	size_t pindex;
	for (i = 0; (i < vsize && offset > 0) || i == 0; i++) {

		if (FindDimElementIndexById(elems, elems[elemindex].parents[i], &pindex) == PALO_FALSE) {
			return PALO_ERR_DIM_ELEMENT_NOT_FOUND;
		}
		if (elems[pindex].type != ELEMENT_INFO::CONSOLIDATED) {
			return PALO_ERR_INV_ARG;
		}

		vsize2 = elems[pindex].children.size();
		if (i == 0) {

			for (j = 0; j < vsize2; j++) {
				if (elems[pindex].children[j] == elems[elemindex].element) {
					break;
				}
			}

			if (j >= vsize2 && vsize2 != 0) {
				*errmsg = NULL;
				return PALO_ERR_INV_ARG;
			}
		} else {
			j = 0;
		}

		while (j < vsize2 && j + 1 > 0) {
			if (elems[pindex].children[j] != elems[elemindex].element) {
				if (offset >= 0) {
					skipped++;
				} else {
					skipped--;
				}
				if (skipped == offset) {
					if (FindDimElementIndexById(elems, elems[pindex].children[j], siblingindex) == PALO_FALSE) {
						return PALO_ERR_DIM_ELEMENT_NOT_FOUND;
					}
					return PALO_SUCCESS;
				}
			}
			if (offset >= 0) {
				j++;
			} else {
				j--;
			}
		}
	}

	*errmsg = wcsdup(L"invalid offset");
	return PALO_ERR_INV_ARG;
}


void add2ConditionString2(const std::string& BoolOp, const std::string& CmpOp, struct arg_palo_value_m *Val, std::string& conditionstring) {

}


void add2ConditionString(const std::string& BoolOp, const compare_op& cmp, struct arg_palo_value_m *Val, std::string& conditionstring) {

	static const std::string CmpOpLtStr("%3C");
	static const std::string CmpOpLteStr("%3C%3D");
	static const std::string CmpOpGtStr("%3E");
	static const std::string CmpOpGteStr("%3E%3D");
	static const std::string CmpOpEqStr("%3D%3D");
	static const std::string CmpOpNeqStr("%21%3D");

	std::string CmpOp;

	switch (cmp) {
		case compare_op_lt:
			// <
			CmpOp = CmpOpLtStr;
			break;
		case compare_op_gt:
			// >
			CmpOp = CmpOpGtStr;
			break;
		case compare_op_lte:
			// <=
			CmpOp = CmpOpLteStr;
			break;
		case compare_op_gte:
			// >=
			CmpOp = CmpOpGteStr;
			break;
		case compare_op_eq:
			// ==
			CmpOp = CmpOpEqStr;
			break;
		case compare_op_neq:
			// !=
			CmpOp = CmpOpNeqStr;
			break;
	}

	if (!CmpOp.empty()) {
		std::stringstream helper;
		if (Val->type == argPaloValueTypeStr) {
			helper << jedox::util::StringUtils::URLencode(jedox::util::StringUtils::CSVencode(Val->val.s));
		} else {
			helper.setf(std::ios_base::fixed , std::ios_base::floatfield);
			helper.precision(PRECISION);

			helper << Val->val.d;
		}

		conditionstring += BoolOp;
		conditionstring += CmpOp;
		conditionstring += helper.str();
	}
}


void getConditionString(struct arg_getdata_export_options_filter_m *Filter, std::string& conditionstring) {
	static const std::string BoolOpAndStr("%20and%20");
	static const std::string BoolOpOrStr("%20or%20");
	static const std::string BoolOpXorStr("%20or%20");

	std::string BoolOp;

	add2ConditionString(BoolOp, Filter->cmp1, &Filter->val1, conditionstring);

	if (!conditionstring.empty() && (Filter->cmp2 != compare_op_true) && (Filter->cmp2 != compare_op_unknown)) {
		switch (Filter->andor12) {
			case bool_op_and:
				BoolOp = BoolOpAndStr;
				break;
			case bool_op_or:
				BoolOp = BoolOpOrStr;
				break;
			case bool_op_xor:
				BoolOp = BoolOpXorStr;
				break;
			default:
				break;
		}
	}

	add2ConditionString(BoolOp, Filter->cmp2, &Filter->val2, conditionstring);
}


void initso(struct sock_obj *so) {
	so->hostname = NULL;
	so->username = NULL;
	so->pw = NULL;
	so->key = NULL;
	so->version = (int*)calloc(sizeof(int),1);
	so->myServer = NULL;

	so->socket = rand();
	so->port = 0;
}

void updateso(struct sock_obj *so, std::string &key) {
	if ((so != NULL) && (so->myServer != NULL)) {
		SERVER_INFO si = (*(so->myServer)).getCacheDataCopy();
		*(so->version) = si.major_version *100 + si.minor_version;
		so->key = strdup(key.c_str());
	}
}

void api_version(struct VERSION_INFO &api_version_info){
	api_version_info = libpalo_ng_getversion();
};

void checkedInitSSL(std::string &TrustFile) {
	bool isempty = TrustFile.empty();
	if (!InitSSLok || (!isempty && (trustfiles.find(TrustFile) == trustfiles.end()))) {
		NetInitialisation::instance().initSSL(TrustFile);
		InitSSLok = true;
		if (!isempty) {
			trustfiles.insert(TrustFile);
		}
	}
}

const std::string& getPermissionArtString(permission_art pa) {
	return PermissionArtStr[pa];
}

const string CheckForSpecialCubes::PrefixPropertyCube("#_CELL_PROPERTIES_");
const size_t CheckForSpecialCubes::LengthPrefixPropertyCube = CheckForSpecialCubes::PrefixPropertyCube.length();
