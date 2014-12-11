////////////////////////////////////////////////////////////////////////////////
/// @brief
///
/// @file
///
/// Copyright (C) 2006-2013 Jedox AG
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

#pragma warning(disable : 4482 )

#include <WTypes.h>
#include <WinNls.h>

#include "../libpalo_ng/source/include/libpalo_ng/Palo/types.h"
#include "../libpalo_ng/source/include/libpalo_ng/Palo/Database.h"
#include "../libpalo_ng/source/include/libpalo_ng/Palo/Dimension.h"
#include "../libpalo_ng/source/include/libpalo_ng/Network/SocketException.h"

#include "libpalo2.h"

#include <limits>

#using <mscorlib.dll>

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
		if(dest_identifier.len > 0) \
		{ \
			dest_identifier.a = (wchar_t**)calloc(dest_identifier.len, sizeof(wchar_t*)); \
			if (dest_identifier.a == NULL) throw new System::OutOfMemoryException(); \
			for(unsigned int i=0; i<dest_identifier.len; i++) \
			dest_identifier.a[i] = reinterpret_cast<wchar_t*>(Marshal::StringToHGlobalUni((src_identifier)[i]).ToPointer()); \
		} \
	}

#define ARRAY_MARSHAL_CLEANUP(dest_identifier) \
	{ \
		if(dest_identifier.len > 0) \
		{ \
			for(unsigned int i=0; i<dest_identifier.len; i++) \
			Marshal::FreeHGlobal(dest_identifier.a[i]); \
			free(dest_identifier.a); \
		} \
	}


#define SIZE_T_TO_MAX_INT(s) (((s)>INT_MAX && (s)!=(size_t)-1) ? INT_MAX : (int)(s))


ConsolidationInfo::ConsolidationInfo( String *name, double factor ) {
	Name = name;
	Factor = factor;
}


ParentInfo::ParentInfo( String *name ) {
	Name = name;
}


ChildInfo::ChildInfo( String *name ) {
	Name = name;
}


DimElementInfo2::DimElementInfo2( String *name ) {
	Name = name;
	NumberParents = 0;
	NumberChildren = 0;
}

ArgAliasFilterSettings::ArgAliasFilterSettings() {
	Active = false;
}
 
ArgFieldFilterSettings::ArgFieldFilterSettings() {
	Active = false;
}

ArgBasicFilterSettings::ArgBasicFilterSettings() {
	Active = false;
}

ArgDataFilterSettings::ArgDataFilterSettings() {
	Active = false;
}

ArgSortingFilterSettings::ArgSortingFilterSettings() {
	Active = false;
}

ArgStructuralFilterSettings::ArgStructuralFilterSettings() {
	Active = false;
}

ArgTextFilterSettings::ArgTextFilterSettings() {
	Active = false;
}

void Connection::DoInit( String *hostname, String *service, String *username, String *pw_hash, String *trust_file ) {
	unsigned short isgpu = 0;
	struct conversions __pin *pconvs = &convs;
	libpalo_err err;

	if ( !LIBPALO_SUCCESSFUL( libpalo_init_r( &err, pconvs ) ) ) {
		throwException( err );
	}

	struct sock_obj __pin *pso = &so;

	const wchar_t __pin *phostname = PtrToStringChars( hostname );
	const wchar_t __pin *pservice = PtrToStringChars( service );

	if ( !LIBPALO_SUCCESSFUL( palo_connect_w_r( &err, pconvs, phostname, pservice, pso ) ) ) {
		libpalo_cleanup_r( pconvs );
		throwException( err );
	}

	const wchar_t __pin *pusername = PtrToStringChars( username );
	const wchar_t __pin *ppw_hash = PtrToStringChars( pw_hash );
	const wchar_t __pin *ptrustfile = PtrToStringChars( trust_file );

	if ( !LIBPALO_SUCCESSFUL( palo_auth_ssl_w_r( &err, pso, pconvs, pusername, ppw_hash, ptrustfile ) ) ) {
		palo_disconnect2( pso, 1 );
		libpalo_cleanup_r( pconvs );
		throwException( err );
	}

	palo_is_gpu_server_w_r(&err, &isgpu, pso, NULL);

	IsGpu = (isgpu != 0);
}

Connection::Connection( String *hostname, String *service, String *username, String *pw_hash ) {
	DoInit( hostname, service, username, pw_hash, TrustFile );
}

Connection::Connection( String *hostname, String *service, String *username, String *pw_hash, String *trust_file ) {
	DoInit( hostname, service, username, pw_hash, trust_file );
}

Connection::~Connection( void ) {
	struct sock_obj __pin *pso = &so;
	palo_disconnect2( pso, 1 );

	struct conversions __pin *pconvs = &convs;
	libpalo_cleanup_r( pconvs );
}


/* Destructor Alias */
void Connection::Dispose( void ) {
	this->~Connection();
}


void Connection::InitSSL( String *argTrustFile ) {
	TrustFile = argTrustFile;
	
	libpalo_err err;

	const wchar_t __pin *ptrustfile = PtrToStringChars( TrustFile );

	if ( !LIBPALO_SUCCESSFUL( palo_init_ssl_w( &err, ptrustfile ) ) ) {
		throwException( err );
	}

}

void Connection::TestConnection(String *hostname, unsigned int port, String *username, String *pw_hash, String *trust_file) {
	libpalo_err err;

	const wchar_t __pin *phostname = PtrToStringChars( hostname );
	const wchar_t __pin *pusername = PtrToStringChars( username );
	const wchar_t __pin *ppw_hash = PtrToStringChars( pw_hash );
	const wchar_t __pin *ptrustfile = PtrToStringChars( trust_file );

	if ( !LIBPALO_SUCCESSFUL( palo_test_connection_w_r(&err, NULL, phostname, port, pusername, ppw_hash, ptrustfile ) ) ) {
		throwException( err );
	}
}


void Connection::TestConnection(String *hostname, unsigned int port, String *username, String *pw_hash) {
	TestConnection(hostname, port, username, pw_hash, TrustFile);
}


void Connection::CheckValidity() {
	libpalo_err err;

	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	if (!LIBPALO_SUCCESSFUL( palo_check_validity_w_r(&err, pso, pconvs))) {
		throwException( err );
	}
}

void Connection::PrepareLogout() {
}


void Connection::throwException( libpalo_err err ) {
	throw new PaloException( err );
}


void Connection::throwException( String *s ) {
	throw new PaloCommException( s );
}

/* 
// use libpalo2
ConsolidationInfo Connection::DimElementListConsolidated( String *database, String *dimension, String *element )[]
{
    libpalo_err err;
    struct arg_consolidation_element_info_array_w ceia;
    struct sock_obj __pin *pso = &so;
    struct conversions __pin *pconvs = &convs;

    const wchar_t __pin *pdatabase = PtrToStringChars( database );
    const wchar_t __pin *pdimension = PtrToStringChars( dimension );
    const wchar_t __pin *pelement = PtrToStringChars( element );

    if ( !LIBPALO_SUCCESSFUL( palo_element_list_consolidation_elements_w_r( &err, &ceia, pso, pconvs, pdatabase, pdimension, pelement ) ) ) {
		throwException( err );
    }

    ConsolidationInfo ci[] = new ConsolidationInfo[ceia.len];

    for ( unsigned int i = 0; i < ceia.len; i++ ) {
		ci[i].Name = new String( ceia.a[i].name );

	    switch ( ceia.a[i].type ) {

		    case de_n:
			    ci[i].Type = DimElementType::DimElementTypeNumeric;
			    break;

		    case de_s:
			    ci[i].Type = DimElementType::DimElementTypeString;
			    break;
		    case de_c:
			    ci[i].Type = DimElementType::DimElementTypeConsolidated;
			    break;
		    case de_r:
			    ci[i].Type = DimElementType::DimElementTypeRule;
			    break;
	    }
	    ci[i].Factor = ceia.a[i].factor;
    }
    free_arg_consolidation_element_info_array_contents_w( &ceia );

    return ci;
}
*/

// use libpalo_ng directly
ConsolidationInfo Connection::DimElementListConsolidated( String *database, String *dimension, String *element )[]
{
	ErrorInformation *ErrorInfo = new ErrorInformation();
    ConsolidationInfo cia[];

	long i, id, len, pos = 0;

	struct sock_obj __pin *pso = &so;
	std::string sdatabase, sdimension, selement;

	Helper::ConvertStringToUTF8(sdatabase, database);
	Helper::ConvertStringToUTF8(sdimension, dimension);
	Helper::ConvertStringToUTF8(selement, element);

	try {
		Dimension& dim = (*(pso->myServer ))[sdatabase].dimension[sdimension];
		const ELEMENT_INFO &dei = dim[selement].getCacheData();
		ELEMENT_LIST children = dei.children;
		ELEMENT_WEIGHT_LIST weights = dei.weights;

		len = (long)children.size();
		cia = new ConsolidationInfo[len];

		for (i = 0; i < len; i++) {
			id = children[i];
			if (dim.Exists(id)) {
				const jedox::palo::ELEMENT_INFO& ei = dim[id].getCacheData();
				cia[pos].Name = Helper::ConvertUTF8ToString(ei.nelement);
				switch ( ei.type ) {
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
	} catch ( const SocketException& exp ) {
		ErrorInfo->ProcessSocketError(exp.what());
	} catch ( const PaloServerException& exp ) {
		ErrorInfo->ProcessError(exp.code(), exp.Description());
	} catch ( const std::exception& exp ) {
		ErrorInfo->ProcessStandardError(exp.what());
	} catch ( const System::Exception* ) {
		throw;
#ifndef _DEBUG
	} catch ( ... ) {
		ErrorInfo->ProcessUnknownError();
#endif
	}

	ErrorInfo->Check();
	System::Array::Resize<ConsolidationInfo>(&cia, pos);

    return cia;
}


ConsolidationExtendedInfo Connection::ElementListConsolidated( String *database, String *dimension, String *element )[]
{
	ErrorInformation *ErrorInfo = new ErrorInformation();
    ConsolidationExtendedInfo cia[];

	long i = 0, len = 0, pos = 0, id;

	struct sock_obj __pin *pso = &so;
	std::string sdatabase, sdimension, selement;

	Helper::ConvertStringToUTF8(sdatabase, database);
	Helper::ConvertStringToUTF8(sdimension, dimension);
	Helper::ConvertStringToUTF8(selement, element);

	try {
		Dimension& dim = (*(pso->myServer ))[sdatabase].dimension[sdimension];
		const ELEMENT_INFO &dei = dim[selement].getCacheData();
		ELEMENT_LIST children = dei.children;
		ELEMENT_WEIGHT_LIST weights = dei.weights;

		len = (long)children.size();
		cia = new ConsolidationExtendedInfo[len];

		for (i = 0; i < len; i++) {
			id = children[i];
			if (dim.Exists(id)) {
				const jedox::palo::ELEMENT_INFO& ei = dim[id].getCacheData();
				cia[pos].Identifier = id;
				cia[pos].Name = Helper::ConvertUTF8ToString(ei.nelement);
				switch ( ei.type ) {
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
	} catch ( const SocketException& exp ) {
		ErrorInfo->ProcessSocketError(exp.what());
	} catch ( const PaloServerException& exp ) {
		ErrorInfo->ProcessError(exp.code(), exp.Description());
	} catch ( const std::exception& exp ) {
		ErrorInfo->ProcessStandardError(exp.what());
	} catch ( const System::Exception* ) {
		throw;
#ifndef _DEBUG
	} catch ( ... ) {
		ErrorInfo->ProcessUnknownError();
#endif
	}

	ErrorInfo->Check();
	System::Array::Resize<ConsolidationExtendedInfo>(&cia, pos);

    return cia;
}

bool Connection::ElementExists( String *database, String *dimension, String *element )
{
	ErrorInformation *ErrorInfo = new ErrorInformation();
    bool exists = false;

	struct sock_obj __pin *pso = &so;
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
	} catch ( const System::Exception* ) {
		throw;
#ifndef _DEBUG
	} catch ( ... ) {
		ErrorInfo->ProcessUnknownError();
#endif
	}

	ErrorInfo->Check();

    return exists;
}


String *Connection::RootListDatabases()[]
{
    libpalo_err err;
    struct arg_str_array_w databases;
    struct sock_obj __pin *pso = &so;
    struct conversions __pin *pconvs = &convs;

    if ( !LIBPALO_SUCCESSFUL( palo_root_list_databases_w_r( &err, &databases, pso, pconvs ) ) ) {
		throwException( err );
    }

    return Helper::ArgStringArrayToManagedFree( databases );
}


String *Connection::RootListDatabases( DatabaseType Type )[]
{
    libpalo_err err;
    struct arg_str_array_w databases;
    struct sock_obj __pin *pso = &so;
    struct conversions __pin *pconvs = &convs;

    if ( !LIBPALO_SUCCESSFUL( palo_root_list_databases2_w_r( &err, &databases, pso, pconvs, Type ) ) ) {
		throwException( err );
    }

    return Helper::ArgStringArrayToManagedFree( databases );
}


void Connection::DeleteDimension( String *database, String *dimension ) {
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );

	if ( !LIBPALO_SUCCESSFUL( palo_dimension_delete_w_r( &err, pso, pconvs, pdatabase, pdimension ) ) ) {
		throwException( err );
	}
}


void Connection::ClearDimension( String *database, String *dimension ) {
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );

	if ( !LIBPALO_SUCCESSFUL( palo_dimension_clear_w_r( &err, pso, pconvs, pdatabase, pdimension ) ) ) {
		throwException( err );
	}
}


String *Connection::DatabaseListCubes( String *database )[]
{
    libpalo_err err;
    struct arg_str_array_w result;
    struct sock_obj __pin *pso = &so;
    struct conversions __pin *pconvs = &convs;

    const wchar_t __pin *pdatabase = PtrToStringChars( database );

    if ( !LIBPALO_SUCCESSFUL( palo_database_list_cubes_w_r( &err, &result, pso, pconvs, pdatabase ) ) ) {
		throwException( err );
    }

    return Helper::ArgStringArrayToManagedFree( result );
}


String *Connection::DatabaseListCubes( String *database, CubeType type, bool OnlyCubesWithCells )[]
{
    libpalo_err err;
    struct arg_str_array_w result;
    struct sock_obj __pin *pso = &so;
    struct conversions __pin *pconvs = &convs;

    const wchar_t __pin *pdatabase = PtrToStringChars( database );

    if ( !LIBPALO_SUCCESSFUL( palo_database_list_cubes2_w_r( &err, &result, pso, pconvs, pdatabase, type, ( OnlyCubesWithCells ) ? 1 : 0 ) ) ) {
		throwException( err );
    }

    return Helper::ArgStringArrayToManagedFree( result );
}


String *Connection::DatabaseListCubes( String *database, CubeType type )[]
{
    return DatabaseListCubes( database, type, false );
}


String *Connection::DatabaseListDimensions( String *database )[]
{
    libpalo_err err;
    struct arg_str_array_w result;
    struct sock_obj __pin *pso = &so;
    struct conversions __pin *pconvs = &convs;

    const wchar_t __pin *pdatabase = PtrToStringChars( database );

    if ( !LIBPALO_SUCCESSFUL( palo_database_list_dimensions_w_r( &err, &result, pso, pconvs, pdatabase ) ) ) {
		throwException( err );
    }

    return Helper::ArgStringArrayToManagedFree( result );
}


String *Connection::DatabaseListDimensions( String *database, DimensionType Type )[]
{
    libpalo_err err;
    struct arg_str_array_w result;
    struct sock_obj __pin *pso = &so;
    struct conversions __pin *pconvs = &convs;

    const wchar_t __pin *pdatabase = PtrToStringChars( database );

    if ( !LIBPALO_SUCCESSFUL( palo_database_list_dimensions2_w_r( &err, &result, pso, pconvs, pdatabase, Type ) ) ) {
		throwException( err );
    }

    return Helper::ArgStringArrayToManagedFree( result );
}


DimElementType Connection::getType( de_type type ) {
	DimElementType Type = DimElementType::DimElementTypeRule;

	switch ( type ) {

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
			throwException( S"Unknown dimension element type" );
	}

	return Type;

}


DimElementInfo2 Connection::getElementInfo2( arg_dim_element_info2_w &elementinfo )
{
    long j, len;
    DimElementInfo2 ei;

	ei.Identifier = elementinfo.identifier;
	ei.Name = new String( elementinfo.name );
	ei.Type = getType( elementinfo.type );
	ei.Position = elementinfo.position;
	ei.Level = elementinfo.level;
	ei.Indent = elementinfo.indent;
	ei.Depth = elementinfo.depth;
	ei.NumberParents = elementinfo.num_parents;
	len = (long)ei.NumberParents;
	ei.Parents = new ParentInfo[len];

	for (j = 0; j < len; j++ ) {
		ei.Parents[j].Identifier = elementinfo.parents[j].identifier;
		ei.Parents[j].Name = new String( elementinfo.parents[j].name );
		ei.Parents[j].Type = getType( elementinfo.parents[j].type );
	}

	ei.NumberChildren = elementinfo.num_children;
	len = (long)ei.NumberChildren;
	ei.Children = new ChildInfo[len];

	for ( j = 0;j < len; j++ ) {
		ei.Children[j].Identifier = elementinfo.children[j].identifier;
		ei.Children[j].Name = new String( elementinfo.children[j].name );
		ei.Children[j].Type = getType( elementinfo.children[j].type );
		ei.Children[j].Factor = elementinfo.children[j].factor;
	}

    return ei;
}

DimElementInfo2 Connection::DimensionListDimElements2( String *database, String *dimension )[]
{
    libpalo_err err;
    long i, len;
    DimElementInfo2 ei[];

    struct arg_dim_element_info2_array_w deia;
    struct sock_obj __pin *pso = &so;
    struct conversions __pin *pconvs = &convs;

    const wchar_t __pin *pdatabase = PtrToStringChars( database );
    const wchar_t __pin *pdimension = PtrToStringChars( dimension );

    deia.len = 0;
    if ( !LIBPALO_SUCCESSFUL( palo_dimension_list_elements2_w_r( &err, &deia, pso, pconvs, pdatabase, pdimension ) ) ) {
		throwException( err );
    }

	len = (long)deia.len;

    ei = new DimElementInfo2[len];

    for ( i = 0; i < len; i++ ) {
		ei[i] = getElementInfo2(deia.a[i]);
    }

	free_arg_dim_element_info2_array_contents_w( &deia );

    return ei;
}


void Connection::FillElementInfo( ElementInfo &ei, arg_dim_element_info2_raw_w &elementinfo )
{
    long j, len;

	if (elementinfo.name != NULL) {
		ei.Identifier = elementinfo.identifier;
		ei.Name = new String( elementinfo.name );
	} else {
		ei.Identifier = -1;
		ei.Name = NULL;
	}
	ei.Type = getType( elementinfo.type );
	ei.Position = elementinfo.position;
	ei.Level = elementinfo.level;
	ei.Indent = elementinfo.indent;
	ei.Depth = elementinfo.depth;
	ei.NumberParents = elementinfo.num_parents;
	len = (long)ei.NumberParents; 
	ei.Parents = new ParentInfoRaw[len];

	for ( j = 0;j < len; j++ ) {
		ei.Parents[j].Identifier = elementinfo.parents[j].identifier;
	}

	ei.NumberChildren = elementinfo.num_children;
	len = (long)ei.NumberChildren; 
	ei.Children = new ChildInfoRaw[len];

	for ( j = 0;j < len; j++ ) {
		ei.Children[j].Identifier = elementinfo.children[j].identifier;
		ei.Children[j].Factor = elementinfo.children[j].factor;
	}
}

/* 
// use libpalo2
ElementInfo Connection::DimensionListElements( String *database, String *dimension )[]
{
    libpalo_err err;
    unsigned long i;
    ElementInfo ei[];

    struct arg_dim_element_info2_raw_array_w deia;
    struct sock_obj __pin *pso = &so;
    struct conversions __pin *pconvs = &convs;

    const wchar_t __pin *pdatabase = PtrToStringChars( database );
    const wchar_t __pin *pdimension = PtrToStringChars( dimension );

    deia.len = 0;
    if ( !LIBPALO_SUCCESSFUL( palo_dimension_list_elements2_raw_w_r( &err, &deia, pso, pconvs, pdatabase, pdimension ) ) ) {
		throwException( err );
    }

    ei = new ElementInfo[deia.len];

    for ( i = 0; i < deia.len; i++ ) {
		 FillElementInfo(ei[i], deia.a[i]);
    }

	free_arg_dim_element_info2_raw_array_contents_w( &deia );

    return ei;
}
*/	
int PositionComparison(ElementInfo* lhs, ElementInfo* rhs)
{
	return lhs->Position.CompareTo(rhs->Position);
}

// use libpalo_ng directly
ElementInfo* Connection::DimensionListElements( String *database, String *dimension )[]
{
    ErrorInformation *ErrorInfo = new ErrorInformation();
    ElementInfo* eia[];

	long i = 0, dimlen = 0, len, current_index = 0;
    struct sock_obj __pin *pso = &so;
	std::string sdatabase, sdimension;

	Helper::ConvertStringToUTF8(sdatabase, database);
	Helper::ConvertStringToUTF8(sdimension, dimension);

	try {
		Dimension& dim = (*(pso->myServer ))[sdatabase].dimension[sdimension];
		std::unique_ptr<DimensionCache::CacheIterator> it = dim.getIterator(); 
		dimlen = dim.getCacheData().number_elements;
		eia = new ElementInfo*[dimlen];

		for ( ; !( *it ).end(); ++current_index, ++*it ) {
			const jedox::palo::ELEMENT_INFO& ei = ( **it );

			// sanity check
			if (current_index >= dimlen) {
				System::Array::Resize(&eia, ++dimlen );
			}

			ElementInfo* current_ei = eia[current_index] = new ElementInfo();

			if (!ei.nelement.empty()) {
				current_ei->Identifier = ei.element;
				current_ei->Name = Helper::ConvertUTF8ToString(ei.nelement);
			}
		
			switch (ei.type) {
				case ELEMENT_INFO::NUMERIC:
					current_ei->Type = DimElementTypeNumeric;
					break;

				case ELEMENT_INFO::STRING:
					current_ei->Type = DimElementTypeString;
					break;

				case ELEMENT_INFO::CONSOLIDATED:
					current_ei->Type = DimElementTypeConsolidated;
					break;

				default:
					current_ei->Type = DimElementTypeRule;
					break;
			}

			current_ei->Position = ei.position;
			current_ei->Level = ei.level;
			current_ei->Indent = ei.indent;
			current_ei->Depth = ei.depth;

			len = ei.number_parents;
			current_ei->NumberParents = len;
			current_ei->Parents = new ParentInfoRaw[len];
			for (i = 0; i < len; i++ ) {
				current_ei->Parents[i].Identifier = ei.parents[i];
			}

			len = ei.number_children;
			current_ei->NumberChildren = len;
			current_ei->Children = new ChildInfoRaw[len];
			for (i = 0; i < len; i++ ) {
				current_ei->Children[i].Identifier = ei.children[i];
				current_ei->Children[i].Factor = ei.weights[i];
			}
		}

		if (current_index != dimlen)
		{
			System::Array::Resize(&eia, current_index );
		}
		if (current_index > 0)
		{
			System::Array::Sort(eia, new System::Comparison<ElementInfo*>(PositionComparison));
		}
	
	} catch ( const SocketException& exp ) {
		ErrorInfo->ProcessSocketError(exp.what());
	} catch ( const PaloServerException& exp ) {
		ErrorInfo->ProcessError(exp.code(), exp.Description());
	} catch ( const std::exception& exp ) {
		ErrorInfo->ProcessStandardError(exp.what());
	} catch ( const System::Exception* ) {
		throw;
#ifndef _DEBUG
	} catch ( ... ) {
		ErrorInfo->ProcessUnknownError();
#endif
	}

	ErrorInfo->Check();

    return eia;
}


String* Connection::GetDimensionNameFromID(String* database, long id) {
    ErrorInformation* ErrorInfo = new ErrorInformation();
    String* dimension = NULL;

    struct sock_obj __pin *pso = &so;
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
	} catch (const System::Exception*) {
		throw;
#ifndef _DEBUG
	} catch (...) {
		ErrorInfo->ProcessUnknownError();
#endif
	}

	ErrorInfo->Check();

    return dimension;
}


String* Connection::GetCubeNameFromID(String* database, long id) {
    ErrorInformation* ErrorInfo = new ErrorInformation();
    String* cube = NULL;

    struct sock_obj __pin *pso = &so;
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
	} catch (const System::Exception*) {
		throw;
#ifndef _DEBUG
	} catch (...) {
		ErrorInfo->ProcessUnknownError();
#endif
	}

	ErrorInfo->Check();

    return cube;
}



String* Connection::GetElementNameFromID( String *database, String *dimension, long id )
{
    ErrorInformation *ErrorInfo = new ErrorInformation();
    String* element = NULL;

    struct sock_obj __pin *pso = &so;
	std::string sdatabase, sdimension;

	Helper::ConvertStringToUTF8(sdatabase, database);
	Helper::ConvertStringToUTF8(sdimension, dimension);

	try {
		element = Helper::ConvertUTF8ToString((*(pso->myServer ))[sdatabase].dimension[sdimension][id].getCacheData().nelement);
	} catch ( const SocketException& exp ) {
		ErrorInfo->ProcessSocketError(exp.what());
	} catch ( const PaloServerException& exp ) {
		ErrorInfo->ProcessError(exp.code(), exp.Description());
	} catch ( const std::exception& exp ) {
		ErrorInfo->ProcessStandardError(exp.what());
	} catch ( const System::Exception* ) {
		throw;
#ifndef _DEBUG
	} catch ( ... ) {
		ErrorInfo->ProcessUnknownError();
#endif
	}

	ErrorInfo->Check();

    return element;
}

DimElementType Connection::GetElementTypeFromID( String *database, String *dimension, long id )
{
    ErrorInformation *ErrorInfo = new ErrorInformation();
    DimElementType type;

    struct sock_obj __pin *pso = &so;
	std::string sdatabase, sdimension;

	Helper::ConvertStringToUTF8(sdatabase, database);
	Helper::ConvertStringToUTF8(sdimension, dimension);

	try {
		type = (DimElementType)(*(pso->myServer ))[sdatabase].dimension[sdimension][id].getCacheData().type;
	} catch ( const SocketException& exp ) {
		ErrorInfo->ProcessSocketError(exp.what());
	} catch ( const PaloServerException& exp ) {
		ErrorInfo->ProcessError(exp.code(), exp.Description());
	} catch ( const std::exception& exp ) {
		ErrorInfo->ProcessStandardError(exp.what());
	} catch ( const System::Exception* ) {
		throw;
#ifndef _DEBUG
	} catch ( ... ) {
		ErrorInfo->ProcessUnknownError();
#endif
	}

	ErrorInfo->Check();

    return type;
}

long Connection::GetElementPositionFromID( String *database, String *dimension, long id )
{
	ErrorInformation *ErrorInfo = new ErrorInformation();
	long position = 0;

	struct sock_obj __pin *pso = &so;
	std::string sdatabase, sdimension;

	Helper::ConvertStringToUTF8(sdatabase, database);
	Helper::ConvertStringToUTF8(sdimension, dimension);

	try {
		position = (*(pso->myServer ))[sdatabase].dimension[sdimension][id].getCacheData().position;
	} catch ( const SocketException& exp ) {
		ErrorInfo->ProcessSocketError(exp.what());
	} catch ( const PaloServerException& exp ) {
		ErrorInfo->ProcessError(exp.code(), exp.Description());
	} catch ( const std::exception& exp ) {
		ErrorInfo->ProcessStandardError(exp.what());
	} catch ( const System::Exception* ) {
		throw;
#ifndef _DEBUG
	} catch ( ... ) {
		ErrorInfo->ProcessUnknownError();
#endif
	}

	ErrorInfo->Check();

	return position;
}

long Connection::GetElementIDFromName( String *database, String *dimension, String *element )
{
    ErrorInformation *ErrorInfo = new ErrorInformation();
    long id = -1;

    struct sock_obj __pin *pso = &so;
	std::string sdatabase, sdimension, selement;

	Helper::ConvertStringToUTF8(sdatabase, database);
	Helper::ConvertStringToUTF8(sdimension, dimension);
	Helper::ConvertStringToUTF8(selement, element);

	try {
		id = (*(pso->myServer ))[sdatabase].dimension[sdimension][selement].getCacheData().element;
	} catch ( const SocketException& exp ) {
		ErrorInfo->ProcessSocketError(exp.what());
	} catch ( const PaloServerException& exp ) {
		ErrorInfo->ProcessError(exp.code(), exp.Description());
	} catch ( const std::exception& exp ) {
		ErrorInfo->ProcessStandardError(exp.what());
	} catch ( const System::Exception* ) {
		throw;
#ifndef _DEBUG
	} catch ( ... ) {
		ErrorInfo->ProcessUnknownError();
#endif
	}

	ErrorInfo->Check();

    return id;
}

String *Connection::CubeListDimensions( String *database, String *cube )[]
{
    libpalo_err err;
    struct arg_str_array_w dimensions;
    struct sock_obj __pin *pso = &so;
    struct conversions __pin *pconvs = &convs;

    const wchar_t __pin *pdatabase = PtrToStringChars( database );
    const wchar_t __pin *pcube = PtrToStringChars( cube );

    if ( !LIBPALO_SUCCESSFUL( palo_cube_list_dimensions_w_r( &err, &dimensions, pso, pconvs, pdatabase, pcube ) ) ) {
		throwException( err );
    }

    return Helper::ArgStringArrayToManagedFree( dimensions );
}


String *Connection::DimensionListCubes( String *database, String *dimension )[]
{
    libpalo_err err;
    struct arg_str_array_w cubes;
    struct sock_obj __pin *pso = &so;
    struct conversions __pin *pconvs = &convs;

    const wchar_t __pin *pdatabase = PtrToStringChars( database );
    const wchar_t __pin *pdimension = PtrToStringChars( dimension );

    if ( !LIBPALO_SUCCESSFUL( palo_dimension_list_cubes_w_r( &err, &cubes, pso, pconvs, pdatabase, pdimension ) ) ) {
		throwException( err );
    }

    return Helper::ArgStringArrayToManagedFree( cubes );
}


String *Connection::DimensionListCubes( String *database, String *dimension, CubeType Type, bool OnlyCubesWithCells )[]
{
    libpalo_err err;
    struct arg_str_array_w cubes;
    struct sock_obj __pin *pso = &so;
    struct conversions __pin *pconvs = &convs;

    const wchar_t __pin *pdatabase = PtrToStringChars( database );
    const wchar_t __pin *pdimension = PtrToStringChars( dimension );

    if ( !LIBPALO_SUCCESSFUL( palo_dimension_list_cubes2_w_r( &err, &cubes, pso, pconvs, pdatabase, pdimension, Type, ( OnlyCubesWithCells ) ? 1 : 0 ) ) ) {
		throwException( err );
    }

    return Helper::ArgStringArrayToManagedFree( cubes );
}


String *Connection::DimensionListCubes( String *database, String *dimension, CubeType Type )[]
{
    return DimensionListCubes( database, dimension, Type, false );
}


void Connection::GetData( String *&str_result, double __gc &dbl_result, String *database, String *cube, String *coordinates[] ) {
	libpalo_err err;
	struct arg_palo_value_w val;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pcube = PtrToStringChars( cube );

	ARRAY_MARSHAL_CONVERT( acoordinates, coordinates )
	palo_getdata_w_r( &err, &val, pso, pconvs, pdatabase, pcube, acoordinates );
	ARRAY_MARSHAL_CLEANUP( acoordinates );

	if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
		throwException( err );
	}

	if ( val.type == argPaloValueTypeDouble ) {
		dbl_result = val.val.d;
		str_result = NULL;
	} else {
		if ( val.type == argPaloValueTypeStr ) {
			str_result = Helper::CharPtrToStringFree( val.val.s );
		} else {
			throwException( S"returned value is of invalid type." );
		}
	}
}

DrillThroughSet Connection::CellDrillThrough( String *database, String *cube, String *coordinates[] , DrillThroughType mode)[] {

	libpalo_err err;
	struct arg_str_array_array_w result;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pcube = PtrToStringChars( cube );

	ARRAY_MARSHAL_CONVERT( acoordinates, coordinates )
	palo_cell_drill_through_w_r( &err, &result, pso, pconvs, pdatabase, pcube, &acoordinates, drillthrough_type(mode) );
	ARRAY_MARSHAL_CLEANUP( acoordinates );

	if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
		throwException( err );
	}

    // convert return value
    DrillThroughSet dtsa[];

    long i, j, vsize = (long)result.len, vsize2;

	try {
	    dtsa = new DrillThroughSet[vsize];

	    for ( i = 0; i < vsize; i++ ) {
			vsize2 = (long)result.a[i].len;
			dtsa[i].Items = new String * __gc[vsize2];

			for (j=0; j < vsize2; j++) {
				dtsa[i].Items[j] = new String(result.a[i].a[j]);
			}
	    }
    } catch (const Exception *e ) {
		free_arg_str_array_array_contents_w( &result );
	    throw e;
    }

    free_arg_str_array_array_contents_w( &result );

    return dtsa;
}

DrillThroughSet Connection::FastCellDrillThrough( String *database, String *cube, String *coordinates[] , DrillThroughType mode)[] {

	ErrorInformation *ErrorInfo = new ErrorInformation();
    DrillThroughSet dtsa[] = NULL;

    struct sock_obj __pin *pso = &so;
	std::string sdatabase, scube, selement;
	long i, j, len = coordinates->Length, len2;
	std::vector<std::string> scoordinates(len);

	Helper::ConvertStringToUTF8(sdatabase, database);
	Helper::ConvertStringToUTF8(scube, cube);
	for (i = 0; i < len; i++) {
		Helper::ConvertStringToUTF8(selement, coordinates[i]);
		scoordinates[i] = selement;
	}

	try {
		std::vector<DRILLTHROUGH_INFO> cdt;

		(*(pso->myServer ))[sdatabase].cube[scube].CellDrillThrough(cdt, scoordinates, Cube::DRILLTHROUGH_TYPE(mode)); 
		scoordinates.clear();
		len = (long)cdt.size();
	    dtsa = new DrillThroughSet[len];
		for (i = 0; i < len; i++) { 
			len2 = (long)cdt[i].line.size();
			dtsa[i].Items = new String * __gc[len2];
			for (j = 0; j < len2; j++) {
				dtsa[i].Items[j] = Helper::ConvertUTF8ToString(cdt[i].line[j]);
			}
			cdt[i].line.clear();
		}
		cdt.clear();
	} catch ( const SocketException& exp ) {
		ErrorInfo->ProcessSocketError(exp.what());
	} catch ( const PaloServerException& exp ) {
		ErrorInfo->ProcessError(exp.code(), exp.Description());
	} catch ( const std::exception& exp ) {
		ErrorInfo->ProcessStandardError(exp.what());
	} catch ( const System::Exception* ) {
		throw;
#ifndef _DEBUG
	} catch ( ... ) {
		ErrorInfo->ProcessUnknownError();
#endif
	}

	ErrorInfo->Check();

    return dtsa;
}


void Connection::DimElementDeleteMulti( String *database, String *dimension, String *dimension_elements[] ) {

	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );

	ARRAY_MARSHAL_CONVERT( adimension_elements, dimension_elements )
	palo_edelete_bulk_w_r( &err, pso, pconvs, pdatabase, pdimension, &adimension_elements );
	ARRAY_MARSHAL_CLEANUP( adimension_elements );

	if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
		throwException( err );
	}
}

String *Connection::ElementDimension( String *database, String *dimension_elements[], bool should_be_unique ) {
	wchar_t *str;
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );

	ARRAY_MARSHAL_CONVERT( adimension_elements, dimension_elements )
	palo_edimension_w_r( &err, &str, pso, pconvs, pdatabase, adimension_elements, should_be_unique ? PALO_TRUE : PALO_FALSE );
	ARRAY_MARSHAL_CLEANUP( adimension_elements )

	if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
		throwException( err );
	}

	return Helper::CharPtrToStringFree( str );
}


String *Connection::ElementCubeDimension( String *database, String *cube, String *dimension_elements[], bool should_be_unique ) {
	wchar_t *str;
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pcube = PtrToStringChars( cube );

	ARRAY_MARSHAL_CONVERT( adimension_elements, dimension_elements )
	palo_ecubedimension_w_r( &err, &str, pso, pconvs, pdatabase, pcube, adimension_elements, should_be_unique ? PALO_TRUE : PALO_FALSE );
	ARRAY_MARSHAL_CLEANUP( adimension_elements )

	if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
		throwException( err );
	}

	return Helper::CharPtrToStringFree( str );
}


void Connection::SetData( String *database, String *cube, String *coordinates[], String *str_value ) {
	SetData( database, cube, coordinates, str_value, 0.0, SplashMode::SplashModeDisable );
}


void Connection::SetData( String *database, String *cube, String *coordinates[], double dbl_value, SplashMode mode ) {
	SetData( database, cube, coordinates, NULL, dbl_value, mode );
}


void Connection::SetData( String *database, String *cube, String *coordinates[], String *str_value, double dbl_value, SplashMode mode ) {
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pcube = PtrToStringChars( cube );
	const wchar_t __pin *pstr_value = str_value != NULL ? PtrToStringChars( str_value ) : NULL;

	ARRAY_MARSHAL_CONVERT( acoordinates, coordinates )
	palo_setdata_w_r( &err, pso, pconvs, pdatabase, pcube, libpalo_make_arg_palo_dataset_w( acoordinates, libpalo_make_arg_palo_value_w( pstr_value, dbl_value ) ), ( splash_mode )mode );
	ARRAY_MARSHAL_CLEANUP( acoordinates )

	if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
		throwException( err );
	}
}


void Connection::SetData( String *database, String *cube, String *coordinates[], double dbl_value ) {
	Connection::SetData( database, cube, coordinates, dbl_value, SplashMode::SplashModeDisable );
}


void Connection::CopyCell( String *database, String *cube, String *from[], String *to[], bool withVal, double dbl_value ) {
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pcube = PtrToStringChars( cube );

	ARRAY_MARSHAL_CONVERT( afrom, from )
	ARRAY_MARSHAL_CONVERT( ato, to )
	palo_cell_copy_w_r( &err, pso, pconvs, pdatabase, pcube, &afrom, &ato, withVal, dbl_value );
	ARRAY_MARSHAL_CLEANUP( afrom )
	ARRAY_MARSHAL_CLEANUP( ato )

	if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
		throwException( err );
	}
}


void Connection::CopyCell( String *database, String *cube, String *from[], String *to[] ) {
	Connection::CopyCell( database, cube, from, to, false, 0 );
}


void Connection::CopyCell( String *database, String *cube, String *from[], String *to[], double dbl_value ) {
	Connection::CopyCell( database, cube, from, to, true, dbl_value );
}


void Connection::GoalSeek( String *database, String *cube, String *path[], double dbl_value ) {
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pcube = PtrToStringChars( cube );

	ARRAY_MARSHAL_CONVERT( apath, path )
	palo_cell_goalseek_w_r( &err, pso, pconvs, pdatabase, pcube, &apath, dbl_value );
	ARRAY_MARSHAL_CLEANUP( apath )

	if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
		throwException( err );
	}
}


void Connection::DimensionAddOrUpdateDimElement( String *database, String *dimension, String *element, DimensionAddOrUpdateElementMode mode, DimElementType type, ConsolidationInfo ci[], bool append_c ) {
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	struct arg_consolidation_element_info_array_w ceia;
	ceia.len = ci->Length;
	ceia.a = new struct arg_consolidation_element_info_w[ceia.len];

	for ( int i = 0; i < ci->Length; i++ ) {
		ceia.a[i].factor = ci[i].Factor;
		ceia.a[i].name = reinterpret_cast<wchar_t*>( Marshal::StringToHGlobalUni( ci[i].Name ).ToPointer() );
	}

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );
	const wchar_t __pin *pelement = PtrToStringChars( element );

	palo_eadd_or_update_w_r( &err, pso, pconvs, pdatabase, pdimension, pelement, ( dimension_add_or_update_element_mode )mode, ( de_type )type, ceia, append_c == true ? PALO_TRUE : PALO_FALSE );

	for ( int i = 0; i < ci->Length; i++ ) {
		Marshal::FreeHGlobal( ceia.a[i].name );
	}

	delete ceia.a;

	if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
		throwException( err );
	}
}

ElementInfo* Connection::ElementCreate( String *database, String *dimension, String *element, DimElementType type, ConsolidationInfo ci[]) {
	ElementInfo* ei;
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;
	struct arg_dim_element_info2_raw_w result;

	struct arg_consolidation_element_info_array_w ceia;
	ceia.len = ci->Length;
	ceia.a = new struct arg_consolidation_element_info_w[ceia.len];

	for ( int i = 0; i < ci->Length; i++ ) {
		ceia.a[i].factor = ci[i].Factor;
		ceia.a[i].name = reinterpret_cast<wchar_t*>( Marshal::StringToHGlobalUni( ci[i].Name ).ToPointer() );
	}

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );
	const wchar_t __pin *pelement = PtrToStringChars( element );

	palo_element_create_w_r( &err, &result, pso, pconvs, pdatabase, pdimension, pelement, ( de_type )type, &ceia);

	for ( int i = 0; i < ci->Length; i++ ) {
		Marshal::FreeHGlobal( ceia.a[i].name );
	}

	delete ceia.a;

	if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
		throwException( err );
	}

	ei = new ElementInfo();

	FillElementInfo(*ei, result);

	free_arg_dim_element_info2_raw_contents_w( &result );

	return ei;
}

ElementInfo* Connection::ElementCreate( String *database, String *dimension, String *element) {
	return this->ElementCreate( database, dimension, element, DimElementType::DimElementTypeNumeric, new ConsolidationInfo[] );
}

void Connection::DimensionAddOrUpdateDimElement( String *database, String *dimension, String *element, DimElementType type, ConsolidationInfo ci[] ) {
	this->DimensionAddOrUpdateDimElement( database, dimension, element, DimensionAddOrUpdateElementMode::DimensionAddOrUpdateElementModeAddOrUpdate, type, ci, false );
}

void Connection::DimensionAddOrUpdateDimElement( String *database, String *dimension, String *element, DimensionAddOrUpdateElementMode mode, DimElementType type  ) {
	this->DimensionAddOrUpdateDimElement( database, dimension, element, mode, type, new ConsolidationInfo[], false);
}


void Connection::DimensionAddOrUpdateDimElement( String *database, String *dimension, String *element ) {
	this->DimensionAddOrUpdateDimElement( database, dimension, element, DimElementType::DimElementTypeNumeric, new ConsolidationInfo[] );
}

void Connection::DimensionAddOrUpdateElement( String *database, String *dimension, String *element, AddOrUpdateElementMode mode, DimElementType type, ConsolidationInfo ci[], bool append_c ) {
	this->DimensionAddOrUpdateDimElement(database, dimension, element, DimensionAddOrUpdateElementMode(mode), type, ci, append_c );
}

void Connection::DimensionAddOrUpdateElement( String *database, String *dimension, String *element, DimElementType type, ConsolidationInfo ci[] ) {
	this->DimensionAddOrUpdateElement( database, dimension, element, AddOrUpdateElementMode::AddOrUpdateElementModeAddOrUpdate, type, ci, false );
}

void Connection::DimensionAddOrUpdateElement( String *database, String *dimension, String *element, AddOrUpdateElementMode mode, DimElementType type  ) {
	this->DimensionAddOrUpdateElement( database, dimension, element, mode, type, new ConsolidationInfo[], false);
}

void Connection::DimensionAddOrUpdateElement( String *database, String *dimension, String *element ) {
	this->DimensionAddOrUpdateElement( database, dimension, element, DimElementType::DimElementTypeNumeric, new ConsolidationInfo[] );
}

void Connection::AddDatabase( String *database ) {
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );

	if ( !LIBPALO_SUCCESSFUL( palo_root_add_database_w_r( &err, pso, pconvs, pdatabase ) ) ) {
		throwException( err );
	}
}

DatabaseType Connection::GetDatabaseTypeHelper( unsigned int type ) {
	DatabaseType retval = DatabaseType::NormalDatabase;

	switch ( type ) {

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


DatabaseType Connection::GetDatabaseType( String *database ) {
	libpalo_err err;
	unsigned int result = normal_db;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );

	if ( !LIBPALO_SUCCESSFUL( palo_get_database_type_w_r( &err, &result, pso, pconvs, pdatabase ) ) ) {
		throwException( err );
	}

	return Connection::GetDatabaseTypeHelper(result);

}

DimensionType Connection::GetDimensionTypeHelper( unsigned int type ) {
	DimensionType retval = DimensionType:: UnknownDimensionType;

	switch ( type ) {

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


DimensionType Connection::GetDimensionType( String *database, String *dimension ) {
	libpalo_err err;
	unsigned int result = normal_dim;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );

	if ( !LIBPALO_SUCCESSFUL( palo_get_dimension_type_w_r( &err, &result, pso, pconvs, pdatabase, pdimension ) ) ) {
		throwException( err );
	}

	return Connection::GetDimensionTypeHelper( result );
}

CubeType Connection::GetCubeTypeHelper( unsigned int type ) {
	CubeType retval = CubeType::NormalCube;

	switch ( type ) {

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

CubeType Connection::GetCubeType( String *database, String *cube ) {
	libpalo_err err;
	unsigned int result = normal_cube;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pcube = PtrToStringChars( cube );

	if ( !LIBPALO_SUCCESSFUL( palo_get_cube_type_w_r( &err, &result, pso, pconvs, pdatabase, pcube ) ) ) {
		throwException( err );
	}

	return Connection::GetCubeTypeHelper( result );
}

String *Connection::GetAttributeDimension( String *database, String *dimension ) {
	libpalo_err err;
	wchar_t *result;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );

	if ( !LIBPALO_SUCCESSFUL( palo_get_attribute_dimension_w_r( &err, &result, pso, pconvs, pdatabase, pdimension ) ) ) {
		throwException( err );
	}

	return Helper::CharPtrToStringFree( result );
}


void Connection::SetClientVersion(String *client_version) {
	libpalo_err err;
	struct conversions __pin *pconvs = NULL;
	const wchar_t __pin *pclient_version = PtrToStringChars( client_version );

	if ( !LIBPALO_SUCCESSFUL( palo_set_client_version_w_r( &err, pconvs, pclient_version ) ) ) {
		throwException( err );
	}
}


bool Connection::IsAdminUser() {
	libpalo_err err;
	unsigned short result;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	if ( !LIBPALO_SUCCESSFUL( palo_is_admin_user_w_r( &err, &result, pso, pconvs ) ) ) {
		throwException( err );
	}

	return (result != 0);
}

bool Connection::IsGpuServer() {
	return IsGpu;
}


String *Connection::GetAttributeCube( String *database, String *dimension ) {
	libpalo_err err;
	wchar_t *result;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );

	if ( !LIBPALO_SUCCESSFUL( palo_get_attribute_cube_w_r( &err, &result, pso, pconvs, pdatabase, pdimension ) ) ) {
		throwException( err );
	}

	return Helper::CharPtrToStringFree( result );
}


String *Connection::GetRightsCube( String *database, String *dimension ) {
	libpalo_err err;
	wchar_t *result;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );

	if ( !LIBPALO_SUCCESSFUL( palo_get_rights_cube_w_r( &err, &result, pso, pconvs, pdatabase, pdimension ) ) ) {
		throwException( err );
	}

	return Helper::CharPtrToStringFree( result );
}


String *Connection::ElementChildName( String *database, String *dimension, String *dimension_element, unsigned int n ) {
	libpalo_err err;
	wchar_t *result;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );
	const wchar_t __pin *pdimension_element = PtrToStringChars( dimension_element );

	if ( !LIBPALO_SUCCESSFUL( palo_echildname_w_r( &err, &result, pso, pconvs, pdatabase, pdimension, pdimension_element, n ) ) ) {
		throwException( err );
	}

	return Helper::CharPtrToStringFree( result );
}


String *Connection::ElementFirst( String *database, String *dimension ) {
	libpalo_err err;
	wchar_t *result;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );

	if ( !LIBPALO_SUCCESSFUL( palo_efirst_w_r( &err, &result, pso, pconvs, pdatabase, pdimension ) ) ) {
		throwException( err );
	}

	return Helper::CharPtrToStringFree( result );
}


unsigned int Connection::ElementChildCount( String *database, String *dimension, String *dimension_element ) {
	libpalo_err err;
	unsigned int result;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );
	const wchar_t __pin *pdimension_element = PtrToStringChars( dimension_element );

	if ( !LIBPALO_SUCCESSFUL( palo_echildcount_w_r( &err, &result, pso, pconvs, pdatabase, pdimension, pdimension_element ) ) ) {
		throwException( err );
	}

	return result;
}


unsigned int Connection::ElementParentCount( String *database, String *dimension, String *dimension_element ) {
	libpalo_err err;
	unsigned int result;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );
	const wchar_t __pin *pdimension_element = PtrToStringChars( dimension_element );

	if ( !LIBPALO_SUCCESSFUL( palo_eparentcount_w_r( &err, &result, pso, pconvs, pdatabase, pdimension, pdimension_element ) ) ) {
		throwException( err );
	}

	return result;
}


unsigned int Connection::ElementCount( String *database, String *dimension ) {
	libpalo_err err;
	unsigned int result;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );

	if ( !LIBPALO_SUCCESSFUL( palo_ecount_w_r( &err, &result, pso, pconvs, pdatabase, pdimension ) ) ) {
		throwException( err );
	}

	return result;
}


unsigned int Connection::ElementIndex( String *database, String *dimension, String *dimension_element ) {
	libpalo_err err;
	unsigned int result;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );
	const wchar_t __pin *pdimension_element = PtrToStringChars( dimension_element );

	if ( !LIBPALO_SUCCESSFUL( palo_eindex_w_r( &err, &result, pso, pconvs, pdatabase, pdimension, pdimension_element ) ) ) {
		throwException( err );
	}

	return result;
}


bool Connection::ElementIsChild( String *database, String *dimension, String *de_parent, String *de_child ) {
	libpalo_err err;
	palo_bool result;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );
	const wchar_t __pin *pde_parent = PtrToStringChars( de_parent );
	const wchar_t __pin *pde_child = PtrToStringChars( de_child );

	if ( !LIBPALO_SUCCESSFUL( palo_eischild_w_r( &err, &result, pso, pconvs, pdatabase, pdimension, pde_parent, pde_child ) ) ) {
		throwException( err );
	}

	return result ? true : false;
}


String *Connection::ElementName( String *database, String *dimension, unsigned int n ) {
	libpalo_err err;
	wchar_t *result;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );

	if ( !LIBPALO_SUCCESSFUL( palo_ename_w_r( &err, &result, pso, pconvs, pdatabase, pdimension, n ) ) ) {
		throwException( err );
	}

	return Helper::CharPtrToStringFree( result );
}


String *Connection::ElementParentName( String *database, String *dimension, String *dimension_element, unsigned int n ) {
	libpalo_err err;
	wchar_t *result;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );
	const wchar_t __pin *pdimension_element = PtrToStringChars( dimension_element );

	if ( !LIBPALO_SUCCESSFUL( palo_eparentname_w_r( &err, &result, pso, pconvs, pdatabase, pdimension, pdimension_element, n ) ) ) {
		throwException( err );
	}

	return Helper::CharPtrToStringFree( result );
}


String *Connection::ElementNext( String *database, String *dimension, String *dimension_element ) {
	libpalo_err err;
	wchar_t *result;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );
	const wchar_t __pin *pdimension_element = PtrToStringChars( dimension_element );

	if ( !LIBPALO_SUCCESSFUL( palo_enext_w_r( &err, &result, pso, pconvs, pdatabase, pdimension, pdimension_element ) ) ) {
		throwException( err );
	}

	return Helper::CharPtrToStringFree( result );
}


String *Connection::ElementPrev( String *database, String *dimension, String *dimension_element ) {
	libpalo_err err;
	wchar_t *result;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );
	const wchar_t __pin *pdimension_element = PtrToStringChars( dimension_element );

	if ( !LIBPALO_SUCCESSFUL( palo_eprev_w_r( &err, &result, pso, pconvs, pdatabase, pdimension, pdimension_element ) ) ) {
		throwException( err );
	}

	return Helper::CharPtrToStringFree( result );
}


DimElementType Connection::ElementType( String *database, String *dimension, String *dimension_element ) {
	libpalo_err err;
	de_type result;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );
	const wchar_t __pin *pdimension_element = PtrToStringChars( dimension_element );

	if ( !LIBPALO_SUCCESSFUL( palo_etype_w_r( &err, &result, pso, pconvs, pdatabase, pdimension, pdimension_element ) ) ) {
		throwException( err );
	}

	return getType( result );
}


String *Connection::ElementSibling( String *database, String *dimension, String *dimension_element, int n ) {
	libpalo_err err;
	wchar_t *result;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );
	const wchar_t __pin *pdimension_element = PtrToStringChars( dimension_element );

	if ( !LIBPALO_SUCCESSFUL( palo_esibling_w_r( &err, &result, pso, pconvs, pdatabase, pdimension, pdimension_element, n ) ) ) {
		throwException( err );
	}

	return Helper::CharPtrToStringFree( result );
}


double Connection::ElementWeight( String *database, String *dimension, String *de_parent, String *de_child ) {
	libpalo_err err;
	double result;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );
	const wchar_t __pin *pde_parent = PtrToStringChars( de_parent );
	const wchar_t __pin *pde_child = PtrToStringChars( de_child );

	if ( !LIBPALO_SUCCESSFUL( palo_eweight_w_r( &err, &result, pso, pconvs, pdatabase, pdimension, pde_parent, pde_child ) ) ) {
		throwException( err );
	}

	return result;
}


unsigned int Connection::ElementLevel( String *database, String *dimension, String *dimension_element ) {
	libpalo_err err;
	unsigned int result;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );
	const wchar_t __pin *pdimension_element = PtrToStringChars( dimension_element );

	if ( !LIBPALO_SUCCESSFUL( palo_elevel_w_r( &err, &result, pso, pconvs, pdatabase, pdimension, pdimension_element ) ) ) {
		throwException( err );
	}

	return result;
}


unsigned int Connection::ElementIndent( String *database, String *dimension, String *dimension_element ) {
	libpalo_err err;
	unsigned int result;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );
	const wchar_t __pin *pdimension_element = PtrToStringChars( dimension_element );

	if ( !LIBPALO_SUCCESSFUL( palo_eindent_w_r( &err, &result, pso, pconvs, pdatabase, pdimension, pdimension_element ) ) ) {
		throwException( err );
	}

	return result;
}


unsigned int Connection::ElementTopLevel( String *database, String *dimension ) {
	libpalo_err err;
	unsigned int result;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );

	if ( !LIBPALO_SUCCESSFUL( palo_etoplevel_w_r( &err, &result, pso, pconvs, pdatabase, pdimension ) ) ) {
		throwException( err );
	}

	return result;
}


void Connection::CubeLoad( String *database, String *cube ) {
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pcube = PtrToStringChars( cube );

	if ( !LIBPALO_SUCCESSFUL( palo_cube_load_w_r( &err, pso, pconvs, pdatabase, pcube ) ) ) {
		throwException( err );
	}
}


void Connection::DatabaseLoad( String *database ) {
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );

	if ( !LIBPALO_SUCCESSFUL( palo_database_load_w_r( &err, pso, pconvs, pdatabase ) ) ) {
		throwException( err );
	}
}


void Connection::CubeCommitLog( String *database, String *cube ) {
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pcube = PtrToStringChars( cube );

	if ( !LIBPALO_SUCCESSFUL( palo_cube_commit_log_w_r( &err, pso, pconvs, pdatabase, pcube ) ) ) {
		throwException( err );
	}
}


void Connection::DatabaseSave( String *database ) {
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );

	if ( !LIBPALO_SUCCESSFUL( palo_database_save_w_r( &err, pso, pconvs, pdatabase ) ) ) {
		throwException( err );
	}
}

void Connection::UnloadCube( String *database, String *cube ) {
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pcube = PtrToStringChars( cube );

	if ( !LIBPALO_SUCCESSFUL( palo_cube_unload_w_r( &err, pso, pconvs, pdatabase, pcube ) ) ) {
		throwException( err );
	}
}


void Connection::DeleteCube( String *database, String *cube ) {
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pcube = PtrToStringChars( cube );

	if ( !LIBPALO_SUCCESSFUL( palo_cube_delete_w_r( &err, pso, pconvs, pdatabase, pcube ) ) ) {
		throwException( err );
	}
}


void Connection::ServerShutdown() {
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	if ( !LIBPALO_SUCCESSFUL( palo_server_shutdown_w_r( &err, pso, pconvs ) ) ) {
		throwException( err );
	}
}


void Connection::SetUserPassword( String *user, String *password ) {
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *puser = PtrToStringChars( user );
	const wchar_t __pin *ppassword = PtrToStringChars( password );

	if ( !LIBPALO_SUCCESSFUL( palo_set_user_password_w_r( &err, pso, pconvs, puser, ppassword ) ) ) {
		throwException( err );
	}
}


void Connection::ChangePassword( String *oldpassword, String *newpassword ) {
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *poldpassword = PtrToStringChars( oldpassword );
	const wchar_t __pin *pnewpassword = PtrToStringChars( newpassword );

	if ( !LIBPALO_SUCCESSFUL( palo_change_password_w_r( &err, pso, pconvs, poldpassword, pnewpassword ) ) ) {
		throwException( err );
	}
}


void Connection::UnloadDatabase( String *database ) {
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );

	if ( !LIBPALO_SUCCESSFUL( palo_database_unload_w_r( &err, pso, pconvs, pdatabase ) ) ) {
		throwException( err );
	}
}


void Connection::DeleteDatabase( String *database ) {
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );

	if ( !LIBPALO_SUCCESSFUL( palo_database_delete_w_r( &err, pso, pconvs, pdatabase ) ) ) {
		throwException( err );
	}
}


void Connection::DimElementMove( String *database, String *dimension, String *dimension_element, unsigned int new_position ) {
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );
	const wchar_t __pin *pdimension_element = PtrToStringChars( dimension_element );

	if ( !LIBPALO_SUCCESSFUL( palo_emove_w_r( &err, pso, pconvs, pdatabase, pdimension, pdimension_element, new_position ) ) ) {
		throwException( err );
	}
}


void Connection::DimElementRename( String *database, String *dimension, String *de_oldname, String *de_newname ) {
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );
	const wchar_t __pin *pde_oldname = PtrToStringChars( de_oldname );
	const wchar_t __pin *pde_newname = PtrToStringChars( de_newname );

	if ( !LIBPALO_SUCCESSFUL( palo_erename_w_r( &err, pso, pconvs, pdatabase, pdimension, pde_oldname, pde_newname ) ) ) {
		throwException( err );
	}
}


void Connection::DimElementDelete( String *database, String *dimension, String *dimension_element ) {
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );
	const wchar_t __pin *pdimension_element = PtrToStringChars( dimension_element );

	if ( !LIBPALO_SUCCESSFUL( palo_edelete_w_r( &err, pso, pconvs, pdatabase, pdimension, pdimension_element ) ) ) {
		throwException( err );
	}
}

void Connection::DatabaseAddCube( String *database, String *cube, String *dimensions[], CubeType type ) {
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pcube = PtrToStringChars( cube );

	ARRAY_MARSHAL_CONVERT( adimensions, dimensions )
	palo_database_add_cube2_w_r( &err, pso, pconvs, pdatabase, pcube, adimensions, (cube_type)type );
	ARRAY_MARSHAL_CLEANUP( adimensions );

	if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
		throwException( err );
	}
}


void Connection::DatabaseAddCube( String *database, String *cube, String *dimensions[] ) {
	DatabaseAddCube( database, cube, dimensions, NormalCube );
}


void Connection::DatabaseAddDimension( String *database, String *dimension, DimensionType type ) {
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );

	if ( !LIBPALO_SUCCESSFUL( palo_database_add_dimension2_w_r( &err, pso, pconvs, pdatabase, pdimension, (dim_type)type ) ) ) {
		throwException( err );
	}
}

void Connection::DatabaseAddDimension( String *database, String *dimension) {
	DatabaseAddDimension( database, dimension, NormalDimension );
}

void Connection::DatabaseRenameCube( String *database, String *cube_oldname, String *cube_newname ) {
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pcube_oldname = PtrToStringChars( cube_oldname );
	const wchar_t __pin *pcube_newname = PtrToStringChars( cube_newname );

	if ( !LIBPALO_SUCCESSFUL( palo_database_rename_cube_w_r( &err, pso, pconvs, pdatabase, pcube_oldname, pcube_newname ) ) ) {
		throwException( err );
	}
}

void Connection::DatabaseRenameDimension( String *database, String *dimension_oldname, String *dimension_newname ) {
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension_oldname = PtrToStringChars( dimension_oldname );
	const wchar_t __pin *pdimension_newname = PtrToStringChars( dimension_newname );

	if ( !LIBPALO_SUCCESSFUL( palo_database_rename_dimension_w_r( &err, pso, pconvs, pdatabase, pdimension_oldname, pdimension_newname ) ) ) {
		throwException( err );
	}
}


void Connection::Ping() {
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	if ( !LIBPALO_SUCCESSFUL( palo_ping_w_r( &err, pso, pconvs ) ) ) {
		throwException( err );
	}
}

ServerInfo Connection::ServerInformation() {
	libpalo_err err;
	struct server_info srvinfo;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;
	ServerInfo si;

	if ( !LIBPALO_SUCCESSFUL( palo_server_info_w_r( &err, pso, pconvs, &srvinfo ) ) ) {
		throwException( err );
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

ApiInfo Connection::ApiInformation() {
	libpalo_err err;
	struct api_info apiinfo;
	ApiInfo ai;

	if ( !LIBPALO_SUCCESSFUL( palo_api_version_w_r( &err, &apiinfo ) ) ) {
		throwException( err );
	}

	ai.MajorVersion = apiinfo.major_version;
	ai.MinorVersion = apiinfo.minor_version;
	ai.BugfixVersion = apiinfo.bugfix_version;
	ai.BuildNumber = apiinfo.build_number;

	return ai;
}

unsigned long long Connection::LicenseExpires() {
	ErrorInformation *ErrorInfo = new ErrorInformation();

	unsigned long long expires = std::numeric_limits<unsigned long long>::max();

	struct sock_obj __pin *pso = &so;

	try {
		LICENSE_LIST lic = (*(pso->myServer )).getLicenseInfo();
		for (std::vector<LICENSE_INFO>::const_iterator it = lic.licenses.begin(); it != lic.licenses.end(); ++it) {
			if (expires > it->expiration) {
				expires = it->expiration;
			}
		}
	} catch ( const SocketException& exp ) {
		ErrorInfo->ProcessSocketError(exp.what());
	} catch ( const PaloServerException& exp ) {
		ErrorInfo->ProcessError(exp.code(), exp.Description());
	} catch ( const std::exception& exp ) {
		ErrorInfo->ProcessStandardError(exp.what());
	} catch ( const System::Exception* ) {
		throw;
#ifndef _DEBUG
	} catch ( ... ) {
		ErrorInfo->ProcessUnknownError();
#endif
	}

	ErrorInfo->Check();

    return expires;
}

DatabaseInfo Connection::DatabaseInformation( String *database ) {
	libpalo_err err;
	struct arg_db_info_w dbinfo;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;
	DatabaseInfo di;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );

	if ( !LIBPALO_SUCCESSFUL( palo_database_info_w_r( &err, &dbinfo, pso, pconvs, pdatabase ) ) ) {
		throwException( err );
	}

	di.id = dbinfo.id;
	di.Name = new String( dbinfo.name );
	di.NumberDimensions = dbinfo.number_dimensions;
	di.NumberCubes = dbinfo.number_cubes;

	switch ( dbinfo.status ) {

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

	di.Type = Connection::GetDatabaseTypeHelper(dbinfo.type);

	free_arg_db_info_contents_w( &dbinfo );

	return di;
}


DimensionInfo Connection::DimensionInformation( String *database, String *dimension ) {
	libpalo_err err;
	struct arg_dim_info_w dimensioninfo;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;
	DimensionInfo di;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );

	if ( !LIBPALO_SUCCESSFUL( palo_dimension_info_w_r( &err, &dimensioninfo, pso, pconvs, pdatabase, pdimension ) ) ) {
		throwException( err );
	}

	di.id = dimensioninfo.id;
	di.Name = new String( dimensioninfo.name );
	di.AssocDimension = new String( dimensioninfo.assoc_dimension );
	di.AttributeCube = new String( dimensioninfo.attribut_cube );
	di.RightsCube = new String( dimensioninfo.rights_cube );
	di.NumberElements = dimensioninfo.number_elements;
	di.MaximumLevel = dimensioninfo.maximum_level;
	di.MaximumIndent = dimensioninfo.maximum_indent;
	di.MaximumDepth = dimensioninfo.maximum_depth;

	di.Type = Connection::GetDimensionTypeHelper( dimensioninfo.type );

	free_arg_dim_info_contents_w( &dimensioninfo );

	return di;
}

DimensionInfoSimple Connection::DimensionInformationSimple( String *database, String *dimension ) {
	libpalo_err err;
	struct arg_dim_info_simple_w dimensioninfo;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;
	DimensionInfoSimple di;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );

	if ( !LIBPALO_SUCCESSFUL( palo_dimension_info_simple_w_r( &err, &dimensioninfo, pso, pconvs, pdatabase, pdimension ) ) ) {
		throwException( err );
	}

	di.id = dimensioninfo.id;
	di.Name = new String( dimensioninfo.name );
	di.AssocDimensionId = dimensioninfo.assoc_dimension_id;
	di.AttributeCubeId = dimensioninfo.attribut_cube_id;
	di.RightsCubeId = dimensioninfo.rights_cube_id;
	di.NumberElements = dimensioninfo.number_elements;
	di.MaximumLevel = dimensioninfo.maximum_level;
	di.MaximumIndent = dimensioninfo.maximum_indent;
	di.MaximumDepth = dimensioninfo.maximum_depth;

	di.Type = Connection::GetDimensionTypeHelper( dimensioninfo.type );

	free_arg_dim_info_simple_contents_w( &dimensioninfo );

	return di;
}

ElementInfo* Connection::ElementInformationSimple( String *database, String *dimension, String *element ) {
	libpalo_err err;
	struct arg_dim_element_info2_raw_w elementinfo;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;
	ElementInfo* ei;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );
	const wchar_t __pin *pelement = PtrToStringChars( element );

	if ( !LIBPALO_SUCCESSFUL( palo_element_info_simple_w_r( &err, &elementinfo, pso, pconvs, pdatabase, pdimension, pelement ) ) ) {
		throwException( err );
	}

	ei = new ElementInfo();

	FillElementInfo(*ei, elementinfo);

	free_arg_dim_element_info2_raw_contents_w( &elementinfo );

	return ei;
}


DimElementInfo2 Connection::ElementInformation( String *database, String *dimension, String *element ) {
	libpalo_err err;
	struct arg_dim_element_info2_w elementinfo;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;
	DimElementInfo2 ei;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );
	const wchar_t __pin *pelement = PtrToStringChars( element );

	if ( !LIBPALO_SUCCESSFUL( palo_element_info_w_r( &err, &elementinfo, pso, pconvs, pdatabase, pdimension, pelement ) ) ) {
		throwException( err );
	}

	ei = getElementInfo2(elementinfo);

	free_arg_dim_element_info2_contents_w( &elementinfo );

	return ei;
}

CubeInfo Connection::CubeInformation( String *database, String *cube ) {
	libpalo_err err;
	struct arg_cube_info_w cubeinfo;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;
	CubeInfo ci;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pcube = PtrToStringChars( cube );

	if ( !LIBPALO_SUCCESSFUL( palo_cube_info_w_r( &err, &cubeinfo, pso, pconvs, pdatabase, pcube ) ) ) {
		throwException( err );
	}

	ci.id = cubeinfo.id;
	ci.Name = new String( cubeinfo.name );
	ci.NumberDimensions = cubeinfo.number_dimensions;
	long i, len = (long)cubeinfo.dimensions.len;
	ci.Dimensions = new String * __gc[len];

	for ( i = 0; i < len; i++ ) {
		ci.Dimensions[i] = new String( cubeinfo.dimensions.a[i] );
	}

	ci.NumberCells = cubeinfo.number_cells;
	ci.NumberFilledCells = cubeinfo.number_filled_cells;

	switch ( cubeinfo.status ) {

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

	ci.Type = Connection::GetCubeTypeHelper( cubeinfo.type );

	free_arg_cube_info_contents_w( &cubeinfo );

	return ci;
}

RuleInfo Connection::getRuleInfo(arg_rule_info_w &ruleinfo) {
	RuleInfo ri;

	ri.id = ruleinfo.identifier;
	ri.timestamp = ruleinfo.timestamp;
	ri.definition = new String( ruleinfo.definition );
	ri.extern_id = new String( ruleinfo.extern_id );
	ri.comment = new String( ruleinfo.comment );
	ri.activated = (ruleinfo.activated != 0);

	return ri;

};

LockInfo Connection::getLockInfo(arg_lock_info_w &lockinfo) {
	LockInfo li;
	String *elems[];
	long i, j, size1 = (long)lockinfo.area.len, size2;

	li.id = lockinfo.identifier;
	li.user = new String( lockinfo.user );
	li.steps = lockinfo.steps;

	li.area = new Collections::ArrayList(size1);
	for (i=0; i < size1; i++) {
		size2 = (long)lockinfo.area.a[i].len;
		elems = new String * __gc[size2];
		for (j=0; j < size2; j++) {
			elems[j] = new String(lockinfo.area.a[i].a[j]);
		}
		li.area->Add(elems);
	}
	return li;

};

RuleInfo Connection::RuleCreate( String *database, String *cube, String *definition, String *ExternId, String *Comment, bool Activate ) {
	libpalo_err err;
	RuleInfo ri;
	arg_rule_info_w ruleinfo;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;
	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pcube = PtrToStringChars( cube );
	const wchar_t __pin *pdefinition = PtrToStringChars( definition );
	const wchar_t __pin *pExternId = NULL;
	const wchar_t __pin *pComment = NULL;

	if ( ExternId != NULL ) {
		pExternId = PtrToStringChars( ExternId );
	}

	if ( Comment != NULL ) {
		pComment = PtrToStringChars( Comment );
	}

	palo_rule_add_w_r( &err, &ruleinfo, pso, pconvs, pdatabase, pcube, pdefinition, 0, pExternId, pComment, (Activate) ? 1 : 0  );

	if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
		throwException( err );
	}

	ri = this->getRuleInfo(ruleinfo);

	free_arg_rule_info_contents_w( &ruleinfo );

	return ri;

};

RuleInfo Connection::RuleCreate( String *database, String *cube, String *definition, bool Activate  ) {
	return RuleCreate( database, cube, definition, NULL, NULL, Activate  );
};

RuleInfo Connection::RuleModify( String *database, String *cube, long id, String *definition, String *ExternId, String *Comment, bool Activate ) {
	libpalo_err err;
	RuleInfo ri;
	arg_rule_info_w ruleinfo;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;
	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pcube = PtrToStringChars( cube );
	const wchar_t __pin *pdefinition = NULL;
	const wchar_t __pin *pExternId = NULL;
	const wchar_t __pin *pComment = NULL;

	if ( ExternId != NULL ) {
		pExternId = PtrToStringChars( ExternId );
	}

	if ( Comment != NULL ) {
		pComment = PtrToStringChars( Comment );
	}

	if ( definition != NULL ) {
		pdefinition = PtrToStringChars( definition );
	}

	palo_rule_modify_w_r( &err, &ruleinfo, pso, pconvs, pdatabase, pcube, id, pdefinition, 0, pExternId, pComment,  (Activate) ? 1 : 0);

	if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
		throwException( err );
	}

	ri = this->getRuleInfo(ruleinfo);

	free_arg_rule_info_contents_w( &ruleinfo );

	return ri;

};

RuleInfo Connection::RuleModify( String *database, String *cube, long id, String *definition, bool Activate ) {
	return RuleModify( database, cube, id, definition, NULL, NULL, Activate );
};


RuleInfo Connection::RulesActivate( String *database, String *cube, long ids __gc [], RuleActivationMode mode )[] {
	libpalo_err err;
    RuleInfo ria[];
    arg_rule_info_array_w ruleinfoa;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;
	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pcube = PtrToStringChars( cube );
	long i, vsize = ids->Length, *pids = NULL;
	
	if (vsize > 0) {
		pids = (long *)calloc(vsize, sizeof(long));
		if (pids == NULL) {
			throw new System::OutOfMemoryException();
		}
		for (i = 0; i < vsize; i++) {
			*(pids + i) = ids[i];	
		}
	}

	palo_rules_activate_w_r( &err, &ruleinfoa, pso, pconvs, pdatabase, pcube, vsize, pids, (rule_activation_mode)mode);

	free(pids);

	if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
		throwException( err );
	}

	long len = (long)ruleinfoa.len;

    ria = new RuleInfo[len];

    for ( i = 0; i < len; i++ ) {
		ria[i] = this->getRuleInfo( *(ruleinfoa.a + i) );
    }

    free_arg_rule_info_array_contents_w( &ruleinfoa );

    return ria;

};

RuleInfo Connection::RuleInformation( String *database, String *cube, long id ) {
	libpalo_err err;
	RuleInfo ri;
	arg_rule_info_w ruleinfo;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;
	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pcube = PtrToStringChars( cube );

	palo_rule_info_w_r( &err, &ruleinfo, pso, pconvs, pdatabase, pcube, id, 0 );

	if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
		throwException( err );
	}

	ri = this->getRuleInfo(ruleinfo);

	free_arg_rule_info_contents_w( &ruleinfo );

	return ri;

};


RuleInfo Connection::CellRuleInformation( String *database, String *cube, String *coordinates[] ) {
	libpalo_err err;
	RuleInfo ri;
	arg_rule_info_w ruleinfo;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pcube = PtrToStringChars( cube );

	ARRAY_MARSHAL_CONVERT( acoordinates, coordinates )
	palo_cell_rule_info_w_r( &err, &ruleinfo, pso, pconvs, pdatabase, pcube, acoordinates );
	ARRAY_MARSHAL_CLEANUP( acoordinates );

	if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
		throwException( err );
	}

	ri = this->getRuleInfo(ruleinfo);

	free_arg_rule_info_contents_w( &ruleinfo );

	return ri;
}


String *Connection::RuleParse( String *database, String *cube, String *definition ) {
	libpalo_err err;
	wchar_t *xmldef;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;
	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pcube = PtrToStringChars( cube );
	const wchar_t __pin *pdefinition = PtrToStringChars( definition );

	palo_rule_parse_w_r( &err, &xmldef, pso, pconvs, pdatabase, pcube, pdefinition );

	if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
		throwException( err );
	}

	return Helper::CharPtrToStringFree( xmldef );

};

void Connection::RuleDelete( String *database, String *cube, long id ) {
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;
	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pcube = PtrToStringChars( cube );

	palo_rule_delete_w_r( &err, pso, pconvs, pdatabase, pcube, id );

	if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
		throwException( err );
	}

};


void Connection::RulesDelete( String *database, String *cube, long ids __gc [] ) {
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;
	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pcube = PtrToStringChars( cube );

	long i, vsize = ids->Length, *pids = NULL;

	if (vsize > 0) {
		pids = (long *)calloc(vsize, sizeof(long));
		if (pids == NULL) {
			throw new System::OutOfMemoryException();
		}
		for (i = 0; i < vsize; i++) {
			*(pids + i) = ids[i];	
		}
	}

	palo_rules_delete_w_r( &err, pso, pconvs, pdatabase, pcube, vsize, pids );

	free(pids);

	if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
		throwException( err );
	}

};


RuleInfo Connection::ListRules( String *database, String *cube )[]
{
    libpalo_err err;
    RuleInfo ria[];
    arg_rule_info_array_w ruleinfoa;
    struct sock_obj __pin *pso = &so;
    struct conversions __pin *pconvs = &convs;
    const wchar_t __pin *pdatabase = PtrToStringChars( database );
    const wchar_t __pin *pcube = PtrToStringChars( cube );

    palo_list_rules_w_r( &err, &ruleinfoa, pso, pconvs, pdatabase, pcube, 0 );

    if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
		throwException( err );
    }

	long i = 0, len = (long)ruleinfoa.len;

    ria = new RuleInfo[len];

    for ( i = 0; i < len; i++ ) {
		ria[i] = this->getRuleInfo( *(ruleinfoa.a + i) );
    }

    free_arg_rule_info_array_contents_w( &ruleinfoa );

    return ria;

};

String *Connection::ListRuleFunctions( ) {
	libpalo_err err;
	wchar_t *functions;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	palo_list_rulefunctions_w_r( &err, &functions, pso, pconvs );

	if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
		throwException( err );
	}

	return Helper::CharPtrToStringFree( functions );
};


static struct arg_palo_value_w CellValue2arg_palo_value_w( CellValue &v ) {
	struct arg_palo_value_w pv;

	pv.type = argPaloValueTypeError;
	pv.val.err.code = 0;

	if ( v.Type == CellValueType::CellValueTypeString ) {
		pv.type = argPaloValueTypeStr;
		pv.val.s = reinterpret_cast<wchar_t*>( Marshal::StringToHGlobalUni( v.Value.StrValue ).ToPointer() );
	} else if ( v.Type == CellValueType::CellValueTypeDouble ) {
		pv.type = argPaloValueTypeDouble;
		pv.val.d = v.Value.DblValue;
	}

	return pv;
}


static void FreeConverted_arg_palo_value_w( struct arg_palo_value_w &pv ) {
	if ( pv.type == argPaloValueTypeStr ) {
		Marshal::FreeHGlobal( pv.val.s );
		pv.val.s = NULL;
	}
}


static struct arg_str_array_w ManagedStrArray2arg_str_array_w( String *a[] ) {
	struct arg_str_array_w sa;

	sa.len = a->Length;
	if ( sa.len > 0 ) {
		sa.a = ( wchar_t** )malloc( sa.len * sizeof( wchar_t* ) );
		if ( sa.a == NULL ) {
			throw new System::OutOfMemoryException();
		}
	}
	unsigned int i = 0;
	try {
		for ( i = 0; i < sa.len; i++ ) {
			sa.a[i] = reinterpret_cast<wchar_t*>( Marshal::StringToHGlobalUni( a[i] ).ToPointer() );
		}
	} catch (const Exception *e ) {
		while ( i-- > 0 ) {
			Marshal::FreeHGlobal( sa.a[i] );
		}
		if ( sa.len > 0 && sa.a != NULL ) {
			free( sa.a );
		}
		throw e;
	}

	return sa;
}


static void FreeConverted_arg_str_array_w( struct arg_str_array_w &sa ) {
	if ( sa.len > 0 && sa.a != NULL ) {
		for ( unsigned int i = 0; i < sa.len; i++ ) {
			Marshal::FreeHGlobal( sa.a[i] );
		}
		free( sa.a );
		sa.a = NULL;
		sa.len = 0;
	}
}


static struct arg_str_array_array_w ManagedStringArrayArray2arg_str_array_array_w( Array *a[] ) {
	struct arg_str_array_array_w sa;

	sa.len = a->Length;
	if ( sa.len > 0 ) {
		sa.a = ( struct arg_str_array_w* )malloc( sa.len * sizeof( struct arg_str_array_w ) );
		if ( sa.a == NULL ) {
			throw new System::OutOfMemoryException();
		}
	} else {
		sa.a = NULL;
	}

	unsigned int i = 0;
	try {
		for ( i = 0; i < sa.len; i++ ) {
			sa.a[i] = ManagedStrArray2arg_str_array_w( Helper::ManagedJaggedArrayMemberToStringArray( a[i] ) );
		}
	} catch (const Exception *e ) {
		while ( i-- > 0 ) {
			FreeConverted_arg_str_array_w( sa.a[i] );
		}
		if ( sa.len > 0 && sa.a != NULL ) {
			free( sa.a );
		}
		throw e;
	}

	return sa;
}


static void FreeConverted_arg_str_array_array_w( struct arg_str_array_array_w &sa ) {
	if ( sa.len > 0 && sa.a != NULL ) {
		for ( unsigned int i = 0; i < sa.len; i++ ) {
			FreeConverted_arg_str_array_w( sa.a[i] );
		}
		free( sa.a );
		sa.a = NULL;
		sa.len = 0;
	}
}

static void FreeConverted_arg_str_array_2d_w( struct arg_str_array_2d_w &sa ) {
	unsigned int i;
	size_t len = sa.rows * sa.cols;
	if (len > 0 && sa.a != NULL ) {
		for ( i = 0; i < len; i++ ) {
			if (sa.a[i] != NULL) {
				Marshal::FreeHGlobal(sa.a[i]);
			}
		}
		free( sa.a );
		sa.a = NULL;
		sa.cols = 0;
		sa.rows = 0;
	}
}

static struct arg_str_array_2d_w ManagedStringArrayArray2arg_str_array_2d_w( Array *a[] ) {
	struct arg_str_array_2d_w sa;

	sa.rows = a->Length;
	sa.cols = (sa.rows > 0) ? a[0]->Length : 0;

	unsigned int i, j;
	size_t len =  sa.rows * sa.cols, len2;
	if ( len > 0 ) {
		sa.a = ( wchar_t ** )calloc(len , sizeof(wchar_t * ) );
		if ( sa.a == NULL ) {
			throw new System::OutOfMemoryException();
		}
	} else {
		sa.a = NULL;
	}

	try {
		for ( i = 0; i < sa.rows; i++ ) {
			len2 = a[i]->Length;
			if (len2 > sa.cols) {
				len2 = sa.cols;
			}
			for (j=0; j < len2; j++) {
				sa.a[i*sa.cols+j] =  reinterpret_cast<wchar_t*>( Marshal::StringToHGlobalUni( reinterpret_cast<String *[]>(a[i])[j] ).ToPointer() );
			}
		}
	} catch (const Exception *e ) {
		FreeConverted_arg_str_array_2d_w(sa);
		throw e;
	}

	return sa;
}

static bool_op _GetDataExport_BoolOp2bool_op( BoolOp op ) {
	switch ( op ) {

		case BoolOp::BoolOpAND:
			return bool_op_and;

		case BoolOp::BoolOpOR:
			return bool_op_or;

		case BoolOp::BoolOpXOR:
			return bool_op_xor;

		default:
			return bool_op_unknown;
	}
}


static compare_op _GetDataExport_CompareOp2compare_op( CompareOp op ) {
	switch ( op ) {

		case CompareOp::CompareOpLT:
			return compare_op_lt;

		case CompareOp::CompareOpGT:
			return compare_op_gt;

		case CompareOp::CompareOpLTE:
			return compare_op_lte;

		case CompareOp::CompareOpGTE:
			return compare_op_gte;

		case CompareOp::CompareOpEQ:
			return compare_op_eq;

		case CompareOp::CompareOpNEQ:
			return compare_op_neq;

		case CompareOp::CompareOpTRUE:
			return compare_op_true;

		default:
			return compare_op_unknown;
	}
}


static struct arg_getdata_export_options_w GetDataExportOptions2arg_getdata_export_options_w( GetDataExportOptions opts ) {
	struct arg_getdata_export_options_w lib_opts;

	lib_opts.last_coordinates = ManagedStrArray2arg_str_array_w( opts.LastCoordinates );
	try {
		lib_opts.filter.val1 = CellValue2arg_palo_value_w( opts.Filter.Value1 );
		try {
			lib_opts.filter.val2 = CellValue2arg_palo_value_w( opts.Filter.Value2 );
			try {
				lib_opts.filter.cmp1 = _GetDataExport_CompareOp2compare_op( opts.Filter.CmpOp1 );
				lib_opts.filter.cmp2 = _GetDataExport_CompareOp2compare_op( opts.Filter.CmpOp2 );
				lib_opts.filter.andor12 = _GetDataExport_BoolOp2bool_op( opts.Filter.AndOr12 );
				lib_opts.num_datasets = opts.NumDatasets;
				lib_opts.base_only = opts.BaseElementsOnly ? PALO_TRUE : PALO_FALSE;
				lib_opts.ignore_empty = opts.IngoreEmptyCells ? PALO_TRUE : PALO_FALSE;

				return lib_opts;
			} catch (const Exception *e ) {
				FreeConverted_arg_palo_value_w( lib_opts.filter.val2 );
				throw e;
			}
		} catch (const Exception *e ) {
			FreeConverted_arg_palo_value_w( lib_opts.filter.val1 );
			throw e;
		}
	} catch (const Exception *e ) {
		FreeConverted_arg_str_array_w( lib_opts.last_coordinates );
		throw e;
	}
}


static void FreeConverted_arg_getdata_export_options_w( struct arg_getdata_export_options_w lib_opts ) {
	FreeConverted_arg_str_array_w( lib_opts.last_coordinates );
	FreeConverted_arg_palo_value_w( lib_opts.filter.val1 );
	FreeConverted_arg_palo_value_w( lib_opts.filter.val2 );
}

Dataset Connection::GetDataExport( String *database, String *cube, Array *elements[], GetDataExportOptions opts, bool useRules, double __gc &progress )[] {
    progress = 0.0;

    // convert options
    struct arg_getdata_export_options_w lib_opts = GetDataExportOptions2arg_getdata_export_options_w( opts );

    // convert elements
    struct arg_str_array_array_w lib_elements = ManagedStringArrayArray2arg_str_array_array_w( elements );

    libpalo_err err;
    struct sock_obj __pin *pso = &so;
    struct conversions __pin *pconvs = &convs;

    const wchar_t __pin *pdatabase = PtrToStringChars( database );
    const wchar_t __pin *pcube = PtrToStringChars( cube );

    struct arg_getdata_export_result_w result;

    palo_getdata_export_rule_w_r( &err, &result, pso, pconvs, pdatabase, pcube, lib_elements, lib_opts, ( useRules ) ? 1 : 0);

    // cleanup
    FreeConverted_arg_getdata_export_options_w( lib_opts );
    FreeConverted_arg_str_array_array_w( lib_elements );

    if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
		throwException( err );
    }

    // convert return value
    Dataset dsa[];
    try {
	    long i = 0, len = (long)result.results.len;
	    dsa = new Dataset[len];

	    for ( i = 0; i < len; i++ ) {
		    dsa[i].Coordinates = Helper::ArgStringArrayToManaged( result.results.a[i].coordinates );
		    dsa[i].Value = Helper::ArgPaloValueToCellValue( result.results.a[i].value );
	    }
    } catch (const Exception *e ) {
		free_arg_getdata_export_result_contents_w( &result );
	    throw e;
    }

    progress = result.progress;

    free_arg_getdata_export_result_contents_w( &result );

    return dsa;
}

Dataset Connection::GetDataExport( String *database, String *cube, Array *elements[], GetDataExportOptions opts, double __gc &progress )[]
{
    return GetDataExport( database, cube, elements, opts, false, progress );
}

Dataset Connection::GetDataExport( String *database, String *cube, Array *elements[], GetDataExportOptions opts )[]
{
    double d;
    return GetDataExport( database, cube, elements, opts, false, d );
}

// use directly libpalo_ng
Dictionary<long, String*> __gc * Connection::GetAttributeValues( String *database, String *dimension, long elements __gc [], String *attribute, long lastelement, unsigned long NumDatasets)
{
    ErrorInformation *ErrorInfo = new ErrorInformation();
	Dictionary<long, String*> __gc * result = NULL;

    struct sock_obj __pin *pso = &so;

	std::string sdatabase, sdimension, sattribute;

	Helper::ConvertStringToUTF8(sdatabase, database);
	Helper::ConvertStringToUTF8(sdimension, dimension);
	Helper::ConvertStringToUTF8(sattribute, attribute);

	try {
		long i, vsize = elements->Length, vsize2 = (lastelement > -1) ? 2 : 0;
		Database& db = (*(pso->myServer ))[sdatabase];
		const DIMENSION_INFO& dim = db.dimension[sdimension].getCacheData();
		long attribute_cube = dim.attribute_cube;
		long attribute_dim = dim.assoc_dimension;
		unsigned long maxsize = (vsize == 0) ? dim.number_elements : vsize;
		if ((NumDatasets > 0) && (NumDatasets < maxsize))
		{
			maxsize = NumDatasets;
		}

		std::vector<long> v0(1), v1(vsize), start(vsize2);
		std::vector<std::vector<long> > area(2);
		v0[0] = db.dimension[attribute_dim][sattribute].getCacheData().element;
		for (i = 0; i < vsize; i++) {
			v1[i] = elements[i];
		}
		if (vsize2 > 0)
		{
			start[0] = v0[0];
			start[1] = lastelement;
		}
		area[0] = v0;
		area[1] = v1;

		std::vector<CELL_VALUE_EXPORTED> expresult;
		db.cube[attribute_cube].CellExport(expresult, area, maxsize, start, "", 0, 0, 0);
		vsize = (long)(expresult.size() - 1);
		result = new Dictionary<long, String*>(vsize);
		std::stringstream tmpstr;
		long id;
	
		for (i = 0; i < vsize; i++) {
			id = expresult[i].cvp.path[1];
			if (expresult[i].cvp.type == CELL_VALUE_PATH_EXPORTED::NUMERIC) {
				tmpstr.str("");
				tmpstr << expresult[i].cvp.val.d;
				result->Add(id, Helper::ConvertUTF8ToString(tmpstr.str()));
			} else {
				result->Add(id, Helper::ConvertUTF8ToString(expresult[i].cvp.val.s));
			}
		}
	} catch ( const SocketException& exp ) {
		ErrorInfo->ProcessSocketError(exp.what());
	} catch ( const PaloServerException& exp ) {
		ErrorInfo->ProcessError(exp.code(), exp.Description());
	} catch ( const std::exception& exp ) {
		ErrorInfo->ProcessStandardError(exp.what());
	} catch ( const System::Exception* ) {
		throw;
#ifndef _DEBUG
	} catch ( ... ) {
		ErrorInfo->ProcessUnknownError();
#endif
	}

	ErrorInfo->Check();

    return result;

}


Dictionary<long, String*> __gc * Connection::GetAttributeValues( String *database, String *dimension, long elements __gc [], String *attribute)
{
	return GetAttributeValues(database, dimension, elements, attribute, -1, 0);
}

CellValue Connection::GetDataMulti( String *database, String *cube, Array *elements[]  )[] {
    
    // convert elements
    struct arg_str_array_2d_w lib_elements = ManagedStringArrayArray2arg_str_array_2d_w( elements );

    libpalo_err err;
    struct sock_obj __pin *pso = &so;
    struct conversions __pin *pconvs = &convs;

    const wchar_t __pin *pdatabase = PtrToStringChars( database );
    const wchar_t __pin *pcube = PtrToStringChars( cube );

    struct arg_palo_value_array_w result;

    palo_getdata_multi_w_r( &err, &result, pso, pconvs, pdatabase, pcube, lib_elements);

	// cleanup
    FreeConverted_arg_str_array_2d_w( lib_elements );

    if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
		throwException( err );
	}

	// convert return value
	CellValue cva[];
    try {
	    long i, len = (long)result.len;
	    cva = new CellValue[len];

		for ( i = 0; i < len; i++ ) {
		    cva[i] = Helper::ArgPaloValueToCellValue( result.a[i]);
	    }
    } catch (const Exception *e ) {
		free_arg_palo_value_array_contents_w( &result );
	    throw e;
    }

    free_arg_palo_value_array_contents_w( &result );

	return cva;
}


CellValue Connection::GetDataArea( String *database, String *cube, Array *elements[]  )[] {
    
    // convert elements
    struct arg_str_array_array_w lib_elements = ManagedStringArrayArray2arg_str_array_array_w( elements );

    libpalo_err err;
    struct sock_obj __pin *pso = &so;
    struct conversions __pin *pconvs = &convs;

    const wchar_t __pin *pdatabase = PtrToStringChars( database );
    const wchar_t __pin *pcube = PtrToStringChars( cube );

    struct arg_palo_value_array_w result;

    palo_getdata_area_w_r( &err, &result, pso, pconvs, pdatabase, pcube, lib_elements, PALO_TRUE);

	// cleanup
    FreeConverted_arg_str_array_array_w( lib_elements );

    if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
		throwException( err );
	}

	// convert return value
	CellValue cva[];
    try {
	    long i, len = (long)result.len;
	    cva = new CellValue[len];

		for ( i = 0; i < len; i++ ) {
		    cva[i] = Helper::ArgPaloValueToCellValue( result.a[i]);
	    }
    } catch (const Exception *e ) {
		free_arg_palo_value_array_contents_w( &result );
	    throw e;
    }

    free_arg_palo_value_array_contents_w( &result );

	return cva;
}

LockInfo Connection::CubeLock( String *database, String *cube, Array *elements[]) {
    struct sock_obj __pin *pso = &so;
    struct conversions __pin *pconvs = &convs;
    libpalo_err err;
    struct arg_lock_info_w result;
	LockInfo li;

    const wchar_t __pin *pdatabase = PtrToStringChars( database );
    const wchar_t __pin *pcube = PtrToStringChars( cube );

    // convert elements
    struct arg_str_array_array_w lib_elements = ManagedStringArrayArray2arg_str_array_array_w( elements );


    palo_cube_lock_w_r( &err, &result, pso, pconvs, pdatabase, pcube, lib_elements);

    // cleanup
    FreeConverted_arg_str_array_array_w( lib_elements );

    if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
		throwException( err );
    }

	li = this->getLockInfo(result);
	
    free_arg_lock_info_contents_w( &result );

    return li;
}

LockInfo Connection::CubeLocks( String *database, String *cube)[] {
    struct sock_obj __pin *pso = &so;
    struct conversions __pin *pconvs = &convs;
    libpalo_err err;
	struct arg_lock_info_array_w result;
	LockInfo lia[];
	long i, lsize;

    const wchar_t __pin *pdatabase = PtrToStringChars( database );
    const wchar_t __pin *pcube = PtrToStringChars( cube );

    palo_cube_locks_w_r( &err, &result, pso, pconvs, pdatabase, pcube);

    if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
		throwException( err );
    }

	lsize = (long)result.len;

	lia = new LockInfo[lsize];
	for (i=0; i < lsize; i++) {
		lia[i] = this->getLockInfo(*(result.a+i));
	}
	
    free_arg_lock_info_array_contents_w( &result );

    return lia;
}


void Connection::CubeRollback( String *database, String *cube, long lockid, long steps){
	struct sock_obj __pin *pso = &so;
    struct conversions __pin *pconvs = &convs;
    libpalo_err err;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
    const wchar_t __pin *pcube = PtrToStringChars( cube );

    palo_cube_rollback_w_r( &err, pso, pconvs, pdatabase, pcube, lockid, steps);

    if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
		throwException( err );
    }
}	

void Connection::CubeRollback( String *database, String *cube, long lockid){
	this->CubeRollback(database, cube, lockid, -1);
}	

void Connection::CubeCommit( String *database, String *cube, long lockid){
	struct sock_obj __pin *pso = &so;
    struct conversions __pin *pconvs = &convs;
    libpalo_err err;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
    const wchar_t __pin *pcube = PtrToStringChars( cube );

    palo_cube_commit_w_r( &err, pso, pconvs, pdatabase, pcube, lockid);

    if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
		throwException( err );
    }
}	

SuperVisionServerInfo Connection::SVSInformation(){
	struct sock_obj __pin *pso = &so;
    struct conversions __pin *pconvs = &convs;
    libpalo_err err;
	struct arg_svs_info_w result;
	SuperVisionServerInfo svsinfo;

    palo_get_svs_info_w_r( &err, &result, pso, pconvs);

    if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
		throwException( err );
    }

	svsinfo.CubeWorkerActive = (result.cube_worker_active != 0);
	svsinfo.DrillThroughEnabled = (result.drill_through_enabled != 0);
	svsinfo.LoginMode = SVSLoginMode(result.login_mode);
	svsinfo.SVSActive = (result.svs_active != 0);
	svsinfo.WindowsSsoEnabled = (result.windows_sso_enabled != 0);

	return svsinfo;
}

void Connection::CubeConvert( String *database, String *cube, CubeType newType){
    struct sock_obj __pin *pso = &so;
    struct conversions __pin *pconvs = &convs;
    libpalo_err err;

    const wchar_t __pin *pdatabase = PtrToStringChars( database );
    const wchar_t __pin *pcube = PtrToStringChars( cube );

    palo_cube_convert_w_r( &err, pso, pconvs, pdatabase, pcube, cube_type(newType));

    if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
		throwException( err );
    }
}

void Connection::CubeClear( String *database, String *cube, unsigned short complete, Array *elements[] ) {
	/* convert elements */

	struct arg_str_array_array_w lib_elements;

	if ( complete == 0 ) {
		lib_elements = ManagedStringArrayArray2arg_str_array_array_w( elements );
	}

	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;

	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pcube = PtrToStringChars( cube );

	palo_cube_clear_w_r( &err, pso, pconvs, pdatabase, pcube, complete, lib_elements );

	if ( complete == 0 ) {
		/* cleanup */
		FreeConverted_arg_str_array_array_w( lib_elements );
	}

	if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
		throwException( err );
	}

	return;
}


void Connection::CubeClear( String *database, String *cube ) {
	Connection::CubeClear( database, cube, 1, NULL );
}


void Connection::CubeClear( String *database, String *cube, Array *elements[] ) {
	Connection::CubeClear( database, cube, 0, elements );
}


void Connection::SetDataMulti( String *database, String *cube, DataSetMulti dsa[], SplashMode mode, bool Add, bool EventProccessor ) {
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;
	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pcube = PtrToStringChars( cube );

	struct arg_palo_dataset_array_w palo_dsa;
	unsigned int i;

	palo_dsa.len = dsa->Length;

	if ( palo_dsa.len > 0 ) {

		palo_dsa.a = ( struct arg_palo_dataset_w * )calloc( palo_dsa.len , sizeof( struct arg_palo_dataset_w ) );

		if ( palo_dsa.a == NULL ) {
			throw new System::OutOfMemoryException();
		}

		for ( i = 0; i < palo_dsa.len; i++ ) {
			*( palo_dsa.a + i ) = libpalo_make_arg_palo_dataset_w( ManagedStrArray2arg_str_array_w( dsa[i].Coordinates ) , libpalo_make_arg_palo_value_w( reinterpret_cast<wchar_t*>( Marshal::StringToHGlobalUni( dsa[i].Value.s ).ToPointer() ), dsa[i].Value.d ) );
		}

		palo_setdata_multi_w_r( &err, pso, pconvs, pdatabase, pcube, palo_dsa, ( splash_mode )mode, Add, EventProccessor );

		for ( i = 0; i < palo_dsa.len; i++ ) {
			FreeConverted_arg_str_array_w((palo_dsa.a + i)->coordinates);
			if((palo_dsa.a + i)->value.type == PaloValueTypeStr){
				Marshal::FreeHGlobal((palo_dsa.a + i)->value.val.s);
			}
		}

		free( palo_dsa.a );

		if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
			throwException( err );
		}
	}
};

SubsetResult Connection::DimensionSubset(	String *database, String *dimension, int indent,
											ArgAliasFilterSettings alias,
											ArgFieldFilterSettings field,
											ArgBasicFilterSettings basic,
											ArgDataFilterSettings data,
											ArgSortingFilterSettings sorting,
											ArgStructuralFilterSettings structural,
											ArgTextFilterSettings text)[] {
	libpalo_err err;
	struct sock_obj __pin *pso = &so;
	struct conversions __pin *pconvs = &convs;
	const wchar_t __pin *pdatabase = PtrToStringChars( database );
	const wchar_t __pin *pdimension = PtrToStringChars( dimension );

	struct arg_subset_result_array_w result;
	struct arg_alias_filter_settings_w m_alias;
	struct arg_field_filter_settings_w m_field;
	struct arg_basic_filter_settings_w m_basic;
	struct arg_data_filter_settings_w m_data;
	struct arg_sorting_filter_settings_w m_sorting;
	struct arg_structural_filter_settings_w m_structural;
	struct arg_text_filter_settings_w m_text;

	size_t len2;
	unsigned int i, len;

	m_alias.active = (alias.Active) ? 1 : 0;

	if (alias.Active)  {
		m_alias.flags = alias_filter_flags(alias.Flags);
		m_alias.attribute1 = reinterpret_cast<wchar_t*>(Marshal::StringToHGlobalUni(alias.Attribute1).ToPointer());
		m_alias.attribute2 = reinterpret_cast<wchar_t*>(Marshal::StringToHGlobalUni(alias.Attribute2).ToPointer());
	}

	m_field.active = (field.Active) ? 1 : 0;

	if (field.Active)  {
		m_field.flags = alias_filter_flags(field.Flags);
		m_field.advanced = ManagedStringArrayArray2arg_str_array_array_w( field.Advanced );
	}

	m_basic.active = (basic.Active) ? 1 : 0;

	if (basic.Active)  {
		m_basic.flags = pick_list_flags(basic.Flags);
		m_basic.manual_subset_set = (basic.ManualSubsetSet) ? 1: 0;
		Helper::ManagedToArgStringArray( basic.ManualSubset, m_basic.manual_subset );
	}

	m_data.active = (data.Active) ? 1 : 0;

	if (data.Active)  {
		m_data.flags = data_filter_flags(data.Flags);
		m_data.cmp.op1 = reinterpret_cast<wchar_t*>(Marshal::StringToHGlobalUni(data.Cmp.Op1).ToPointer());
		m_data.cmp.op2 = reinterpret_cast<wchar_t*>(Marshal::StringToHGlobalUni(data.Cmp.Op2).ToPointer());
		m_data.cmp.par1d = data.Cmp.Par1d;
		m_data.cmp.par2d = data.Cmp.Par2d;
		m_data.cmp.par1s = reinterpret_cast<wchar_t*>(Marshal::StringToHGlobalUni(data.Cmp.Par1s).ToPointer());
		m_data.cmp.par2s = reinterpret_cast<wchar_t*>(Marshal::StringToHGlobalUni(data.Cmp.Par2s).ToPointer());
		m_data.cmp.use_strings = (data.Cmp.UseStrings) ? 1: 0;
		m_data.cube = reinterpret_cast<wchar_t*>(Marshal::StringToHGlobalUni(data.Cube).ToPointer());
		m_data.lower_percentage_set = (data.LowerPercentageSet) ? 1: 0;
		m_data.lower_percentage = data.LowerPercentage;
		m_data.upper_percentage_set = (data.UpperPercentageSet) ? 1: 0;
		m_data.upper_percentage = data.UpperPercentage;
		m_data.top = data.Top;
		m_data.coords_set =  (data.CoordsSet) ? 1: 0;

		len = data.Coords->Length;

		m_data.coords.a = (arg_bool_str_array_w*)calloc(len, sizeof(arg_bool_str_array_w));
		
		if (m_data.coords.a == NULL) {
			throw new System::OutOfMemoryException();
		}

		m_data.coords.len = len;

		for (i = 0; i< len; i++) {
			len2 = data.Coords[i].Str->Length;
			(m_data.coords.a[i]).str.a = (wchar_t**)calloc(len2, sizeof(wchar_t*));
		
			if ((m_data.coords.a[i]).str.a == NULL) {
				throw new System::OutOfMemoryException();
			}

			(m_data.coords.a[i]).boolval = (data.Coords[i].BoolVal) ? 1 : 0;
			Helper::ManagedToArgStringArray( data.Coords[i].Str, (m_data.coords.a[i]).str );
		}
	}

	m_sorting.active = (sorting.Active) ? 1 : 0;

	if (sorting.Active)  {
		m_sorting.flags = sorting_filter_flags(sorting.Flags);
		m_sorting.attribute = reinterpret_cast<wchar_t*>(Marshal::StringToHGlobalUni(sorting.Attribute).ToPointer());
		m_sorting.level = sorting.Level;
		m_sorting.limit_count = sorting.Limit_count;
		m_sorting.limit_start = sorting.Limit_start;
	}

	m_structural.active = (structural.Active) ? 1 : 0;

	if (structural.Active)  {
		m_structural.flags = structural_filter_flags(structural.Flags);

		m_structural.bound = reinterpret_cast<wchar_t*>(Marshal::StringToHGlobalUni(structural.Bound).ToPointer());

		m_structural.level = (structural.Level) ? 1 : 0;
		m_structural.level_start = structural.LevelStart;
		m_structural.level_end = structural.LevelEnd;

		m_structural.revolve = (structural.Revolve) ? 1 : 0;
		m_structural.revolve_count = structural.RevolveCount;
		m_structural.revolve_element = reinterpret_cast<wchar_t*>(Marshal::StringToHGlobalUni(structural.RevolveElement).ToPointer());;
	}

	m_text.active = (text.Active) ? 1 : 0;

	if (text.Active)  {
		m_text.flags = text_filter_flags(text.Flags);
		Helper::ManagedToArgStringArray( text.RegularExpressions, m_text.reg_exps );
	}

	palo_dimension_subset_w_r( &err, &result, pso, pconvs, pdatabase, pdimension, indent, &m_alias, &m_field, &m_basic, &m_data, &m_sorting, &m_structural, &m_text);

	if (alias.Active)  {
		Marshal::FreeHGlobal(m_alias.attribute1);
		Marshal::FreeHGlobal(m_alias.attribute2);
	}

	if (field.Active)  {
	    FreeConverted_arg_str_array_array_w( m_field.advanced );
	}

	if (basic.Active)  {
		Helper::FreeConvertedArgStringArray(m_basic.manual_subset );
	}

	if (data.Active)  {
		Marshal::FreeHGlobal(m_data.cmp.op1);
		Marshal::FreeHGlobal(m_data.cmp.op2);
		Marshal::FreeHGlobal(m_data.cmp.par1s);
		Marshal::FreeHGlobal(m_data.cmp.par2s);
		Marshal::FreeHGlobal(m_data.cube);

		len = m_data.coords.len;

		for (i = 0; i< len; i++) {
			Helper::FreeConvertedArgStringArray( (m_data.coords.a[i]).str );
		}

		free(m_data.coords.a);
	}

	if (sorting.Active)  {
		Marshal::FreeHGlobal(m_sorting.attribute);
	}

	if (structural.Active)  {
		Marshal::FreeHGlobal(m_structural.bound);
		Marshal::FreeHGlobal(m_structural.revolve_element);
	}

	if (text.Active)  {
		Helper::FreeConvertedArgStringArray(m_text.reg_exps);
	}

	if ( !LIBPALO_SUCCESSFUL( err.result ) ) {
		throwException( err );
	}

	SubsetResult sra[];

	len = result.len;

	sra = new SubsetResult[len];

	for (i = 0; i < len; i++) {
		sra[i].Alias = new String((result.a+i)->alias);
		sra[i].Name = new String((result.a+i)->name);
		sra[i].Index = (result.a+i)->index;
		sra[i].Depth = (result.a+i)->depth;
		sra[i].Path = (result.a+i)->path;
		sra[i].Identifier = (result.a+i)->identifier;
		sra[i].HasChildren = ((result.a+i)->has_children == 1);
	}

    free_arg_subset_result_array_contents_w( &result );
	
	return sra;

}

// use directly libpalo_ng
ElementInfo* Connection::RestrictedList( String *database, String *dimension, long id, long start, long limit )[]
{
    ErrorInformation *ErrorInfo = new ErrorInformation();
    ElementInfo* eia[];

    struct sock_obj __pin *pso = &so;

	if (*(pso->version) < 302) {
		if (id == -2) {
			return DimensionListElements(database, dimension );
		} else {
			ErrorInfo->ProcessError(PALO_ERR_NOT_IMPLEMENTED_ID, "This server doesn't support this call since it is too old.");
			ErrorInfo->Check();
		}
	}

	std::string sdatabase, sdimension;

	Helper::ConvertStringToUTF8(sdatabase, database);
	Helper::ConvertStringToUTF8(sdimension, dimension);

	long pos = 0;
	try {
		std::list<ELEMENT_INFO> elems = (*(pso->myServer ))[sdatabase].dimension[sdimension].getElements(id, start, limit);
		std::list<ELEMENT_INFO>::iterator it = elems.begin();
		std::list<ELEMENT_INFO>::iterator endlist = elems.end();

		long i, len = (long)elems.size(), len2;
		eia = new ElementInfo*[len];

		ElementInfo* current_ei;

		while ( (pos < len) && (it != endlist)) {
			const jedox::palo::ELEMENT_INFO& ei = ( *it );
			current_ei = eia[pos] = new ElementInfo();

			if (!ei.nelement.empty()) {
				current_ei->Identifier = ei.element;
				current_ei->Name = Helper::ConvertUTF8ToString(ei.nelement);
			}
		
			switch (ei.type) {
				case ELEMENT_INFO::NUMERIC:
					current_ei->Type = DimElementTypeNumeric;
					break;

				case ELEMENT_INFO::STRING:
					current_ei->Type = DimElementTypeString;
					break;

				case ELEMENT_INFO::CONSOLIDATED:
					current_ei->Type = DimElementTypeConsolidated;
					break;

				default:
					current_ei->Type = DimElementTypeRule;
					break;
			}

			current_ei->Position = ei.position;
			current_ei->Level = ei.level;
			current_ei->Indent = ei.indent;
			current_ei->Depth = ei.depth;

			len2 = ei.number_parents;
			current_ei->NumberParents = len2;
			current_ei->Parents = new ParentInfoRaw[len2];
			for (i = 0; i < len2; i++ ) {
				current_ei->Parents[i].Identifier = ei.parents[i];
			}

			len2 = ei.number_children;
			current_ei->NumberChildren = len2;
			current_ei->Children = new ChildInfoRaw[len2];
			for (i = 0; i < len2; i++ ) {
				current_ei->Children[i].Identifier = ei.children[i];
				current_ei->Children[i].Factor = ei.weights[i];
			}

			pos++;
			++it;
		}
	} catch ( const SocketException& exp ) {
		ErrorInfo->ProcessSocketError(exp.what());
	} catch ( const PaloServerException& exp ) {
		ErrorInfo->ProcessError(exp.code(), exp.Description());
	} catch ( const std::exception& exp ) {
		ErrorInfo->ProcessStandardError(exp.what());
	} catch ( const System::Exception* ) {
		throw;
#ifndef _DEBUG
	} catch ( ... ) {
		ErrorInfo->ProcessUnknownError();
#endif
	}

	ErrorInfo->Check();

	System::Array::Resize<ElementInfo*>(&eia, pos);

    return eia;
}


/*
// use libpalo2
ElementInfo Connection::DimensionRestrictedFlatListDimElements( String *database, String *dimension, long start, long limit )[] {
    libpalo_err err;
    unsigned long i;
    ElementInfo ei[];

    struct arg_dim_element_info2_raw_array_w deia;
    struct sock_obj __pin *pso = &so;
    struct conversions __pin *pconvs = &convs;

    const wchar_t __pin *pdatabase = PtrToStringChars( database );
    const wchar_t __pin *pdimension = PtrToStringChars( dimension );

    deia.len = 0;
    if ( !LIBPALO_SUCCESSFUL( palo_dimension_restricted_flat_list_elements_w_r( &err, &deia, pso, pconvs, pdatabase, pdimension, start, limit ) ) ) {
		throwException( err );
    }

    ei = new ElementInfo[deia.len];

    for ( i = 0; i < deia.len; i++ ) {
		 FillElementInfo(ei[i], deia.a[i]);
    }

	free_arg_dim_element_info2_raw_array_contents_w( &deia );

    return ei;

}
*/

// use directly libpalo_ng
ElementInfo* Connection::DimensionRestrictedFlatListDimElements( String *database, String *dimension, long start, long limit )[] {
	return RestrictedList(database, dimension, -2, start, limit );
}

/*
// use libpalo2
ElementInfo Connection::DimensionRestrictedTopListDimElements( String *database, String *dimension, long start, long limit )[] {
    libpalo_err err;
    unsigned long i;
    ElementInfo ei[];

    struct arg_dim_element_info2_raw_array_w deia;
    struct sock_obj __pin *pso = &so;
    struct conversions __pin *pconvs = &convs;

    const wchar_t __pin *pdatabase = PtrToStringChars( database );
    const wchar_t __pin *pdimension = PtrToStringChars( dimension );

    deia.len = 0;
    if ( !LIBPALO_SUCCESSFUL( palo_dimension_restricted_top_list_elements_w_r( &err, &deia, pso, pconvs, pdatabase, pdimension, start, limit ) ) ) {
		throwException( err );
    }

    ei = new ElementInfo[deia.len];

    for ( i = 0; i < deia.len; i++ ) {
		 FillElementInfo(ei[i], deia.a[i]);
    }

	free_arg_dim_element_info2_raw_array_contents_w( &deia );

    return ei;

}
*/

// use directly libpalo_ng
ElementInfo* Connection::DimensionRestrictedTopListDimElements( String *database, String *dimension, long start, long limit )[] {
	return RestrictedList(database, dimension, -1, start, limit );
}

/*
use libpalo2
ElementInfo Connection::DimensionRestrictedChildrenListDimElements( String *database, String *dimension, long elementidentifier, long start, long limit )[] {
    libpalo_err err;
    unsigned long i;
    ElementInfo ei[];

    struct arg_dim_element_info2_raw_array_w deia;
    struct sock_obj __pin *pso = &so;
    struct conversions __pin *pconvs = &convs;

    const wchar_t __pin *pdatabase = PtrToStringChars( database );
    const wchar_t __pin *pdimension = PtrToStringChars( dimension );

    deia.len = 0;
    if ( !LIBPALO_SUCCESSFUL( palo_dimension_restricted_children_list_elements_w_r( &err, &deia, pso, pconvs, pdatabase, pdimension, elementidentifier, start, limit ) ) ) {
		throwException( err );
    }

    ei = new ElementInfo[deia.len];

    for ( i = 0; i < deia.len; i++ ) {
		 FillElementInfo(ei[i], deia.a[i]);
    }

	free_arg_dim_element_info2_raw_array_contents_w( &deia );

    return ei;

}
*/

// use directly libpalo_ng
ElementInfo* Connection::DimensionRestrictedChildrenListDimElements( String *database, String *dimension, long elementidentifier, long start, long limit )[] {
	return RestrictedList(database, dimension, elementidentifier, start, limit );
}

size_t Connection::DimensionFlatCount( String *database, String *dimension) {
    libpalo_err err;

    size_t result;
    struct sock_obj __pin *pso = &so;
    struct conversions __pin *pconvs = &convs;

    const wchar_t __pin *pdatabase = PtrToStringChars( database );
    const wchar_t __pin *pdimension = PtrToStringChars( dimension );

    if ( !LIBPALO_SUCCESSFUL( palo_dimension_flat_count_w_r( &err, &result, pso, pconvs, pdatabase, pdimension ) ) ) {
		throwException( err );
    }

	return result;
}


String *Helper::CharPtrToStringFree( wchar_t *s ) {
	String *S = new String( s );
	free_arg_str_w( s );
	return S;
}


String *Helper::GetErrorMessage( libpalo_err err ) {
	String *pserrmsg;

	wchar_t *errstr = libpalo_err_get_string_w( &err );

	if ( errstr == NULL ) {
		pserrmsg = S"could not get error information.";
	} else {
		pserrmsg = Helper::CharPtrToStringFree( errstr );
	}

	return pserrmsg;
}


String *Helper::ArgStringArrayToManaged( struct arg_str_array_w a )[] {
	long i, len = (long)a.len;
    String *sa[] = new String * __gc[len];

    for ( i = 0; i < len; i++ ) {
		sa[i] = new String( a.a[i] );
    }

    return sa;
}

void Helper::ManagedToArgStringArray( String *src[], struct arg_str_array_w &dest ) {
	dest.len = src->Length;
	if(dest.len > 0) { 
		dest.a = (wchar_t**)calloc(dest.len, sizeof(wchar_t*));

		if (dest.a == NULL) {
			throw new System::OutOfMemoryException();
		}

		for(unsigned int i=0; i<dest.len; i++) {
			dest.a[i] = reinterpret_cast<wchar_t*>(Marshal::StringToHGlobalUni((src)[i]).ToPointer());
		}
	}
}

void Helper::FreeConvertedArgStringArray( struct arg_str_array_w &tofree ) {
	if(tofree.len > 0) { 
		for(unsigned int i=0; i<tofree.len; i++) {
			Marshal::FreeHGlobal(tofree.a[i]); \
		}
		free(tofree.a);
		tofree.a = NULL;
		tofree.len = 0;
	}
}

String *Helper::ArgStringArrayToManagedFree( struct arg_str_array_w a )[] {
    String *sa[] = ArgStringArrayToManaged( a );

    free_arg_str_array_contents_w( &a );

    return sa;
}

CellValue Helper::ArgPaloValueToCellValue( struct arg_palo_value_w pv ) {
	CellValue v;

	switch ( pv.type ) {

		case argPaloValueTypeDouble:
			v.Value.DblValue = pv.val.d;
			v.Type = CellValueType::CellValueTypeDouble;
			break;

		case argPaloValueTypeStr:
			v.Value.DblValue = pv.val.d;
			v.Value.StrValue = new String( pv.val.s );
			v.Type = CellValueType::CellValueTypeString;
			break;

		case argPaloValueTypeError:
			v.Value.DblValue = pv.val.d;
			v.Value.StrValue = new String( pv.val.s );
			v.Type = CellValueType::CellValueTypeError;
			break;

	}

	return v;
}


/* needed because Managed C++ seems not to support jagged arrays (why?) */
String *Helper::ManagedJaggedArrayMemberToStringArray( Array *a )[]
{
    String *sa[] = new String * __gc[a->Length];

    for ( int i = 0; i < a->Length; i++ ) {
		sa[i] = static_cast<String*>( a->GetValue( i ) );
    }

    return sa;
}

void Helper::HandleConversionError(bool ToUtf8) 
{
	DWORD LastError = GetLastError();
	if ( LastError != ERROR_INSUFFICIENT_BUFFER ) {
		if (ToUtf8) {
			throw new System::SystemException("WideCharToMultiByte failed with Error=" + LastError);
		} else {
			throw new System::SystemException("MultiByteToWideChar failed with Error=" + LastError);
		}

	} else {
		throw new System::OutOfMemoryException;
	}
}

void Helper::wcs2utf8( char **utf8_str, const wchar_t *s, size_t len ) {
	size_t _len = WideCharToMultiByte( CP_UTF8, 0, s, (int)len, NULL, 0, NULL, NULL );

	if ( _len == 0 ) {
		HandleConversionError(true);
	}

	/* len in bytes */
	*utf8_str = ( char* )malloc( _len + 1 );
	if ( *utf8_str == NULL ) {
		throw new System::OutOfMemoryException;
	}
	
	if ( WideCharToMultiByte( CP_UTF8, 0, s, (int)len, *utf8_str, SIZE_T_TO_MAX_INT( _len ), NULL, NULL ) == 0 ) {
		free(*utf8_str);
		HandleConversionError(true);
	}

	(*utf8_str)[_len] = '\0';
}

void Helper::ConvertStringToUTF8(std::string& To, String * From)
{
	if (String::IsNullOrEmpty(From)) {
		To.clear();
	} else {
		char * pFrom = NULL;
		const wchar_t __pin *wFrom = PtrToStringChars( From );

		wcs2utf8(&pFrom, wFrom, wcslen(wFrom));
		To = pFrom;
		free(pFrom);
	}
}

void Helper::utf82wcs( wchar_t **wcs, const char *utf8_str )
{
	int len = MultiByteToWideChar( CP_UTF8, 0, utf8_str, -1, NULL, 0 );

	if (( len = MultiByteToWideChar( CP_UTF8, 0, utf8_str, -1, NULL, 0 ) ) == 0 ) {
		HandleConversionError(false);
	}
	/* len in wchar's */
	*wcs = ( wchar_t* )malloc( len*sizeof( wchar_t ) );
	if ( *wcs == NULL ) {
		throw new System::OutOfMemoryException;
	}

	if ( MultiByteToWideChar( CP_UTF8, 0, utf8_str, -1, *wcs, SIZE_T_TO_MAX_INT( len ) ) == 0 ) {
		free(*wcs);
		HandleConversionError(false);
	}
}

String *Helper::ConvertUTF8ToString(const std::string& From)
{
	String *To = NULL;
	if (From.size() == 0) {
		To = new String(L"");
	} else {
		wchar_t *wFrom = NULL;

		utf82wcs(&wFrom, From.c_str());
		To = new String(wFrom);
		free(wFrom);
	}

	return To;
}

void ErrorInformation::ProcessErrorCode(int OriginalError) {

	switch ( OriginalError ) {
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
			ErrorCode = OriginalError;
			break;
	}
}

void ErrorInformation::ProcessError(int OriginalError, const std::string& Message) {
	ProcessErrorCode(OriginalError);
	std::stringstream tmpstr;
	tmpstr << Message << " (Error " << ErrorCode << ")"; 
	ErrorMessage = Helper::ConvertUTF8ToString(tmpstr.str());
}

void ErrorInformation::ProcessSocketError(const std::string& Message) {
	ProcessError(PALO_ERR_NETWORK_ID,Message);
}

void ErrorInformation::ProcessStandardError(const std::string& Message) {
	ProcessError(PALO_ERR_SYSTEM_ID,Message);
}

void ErrorInformation::ProcessUnknownError() {
	ProcessStandardError("non standard exception occurred");
}

void ErrorInformation::Check() {
	if (ErrorCode != 0) {
		throw new PaloException( this );
	}
}

String* ErrorInformation::GetMessage() {
	return ErrorMessage;
}

int ErrorInformation::GetCode() {
	return ErrorCode;
}

PaloCommException::PaloCommException( String *message ) : ApplicationException( message ) {
}


Jedox::Palo::Comm::PaloException::PaloException( libpalo_err err ) : ApplicationException( Helper::GetErrorMessage( err ) ) {
	if ( LIBPALO_SUCCESSFUL( err.result ) ) {
		throw new Exception( S"Requested to throw an Exception where no error happened!" );
	}

	this->ErrorCode = ( int )err.palo_error;
	libpalo_err_free_contents( &err );

	this->PaloMessage = this->Message;
}

Jedox::Palo::Comm::PaloException::PaloException( ErrorInformation* ei ) : ApplicationException( ei->GetMessage() ) {
	this->ErrorCode = ei->GetCode();
	this->PaloMessage = this->Message;
}

