/* 
 *
 * Copyright (C) 2006-2014 Jedox AG
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 *
 * \author Florian Schaper <florian.schaper@jedox.com>
 * \author Hendrik Schmieder <hendrik.schmieder@jedox.com>
 * \author Martin Jakl <martin.jakl@qbicon.cz>
 * \author Jiri Junek <jiri.junek@qbicon.cz>
 * 
 *
 */

#if defined(WIN32) || defined(WIN64)
#pragma warning(disable : 4996)
#endif

#include <iomanip>
#include <iostream>

#include <boost/regex.hpp>

#include <libpalo_ng/Palo/types.h>
#include <libpalo_ng/Palo/Exception/PaloExceptionFactory.h>
#include <libpalo_ng/Util/StringUtils.h>

#include <libpalo_ng/Palo/Exception/LibPaloNGExceptionFactory.h>
#include "../Util/CsvLineDecoder.h"
#include "../Util/CsvTokenFromStream.h"

namespace jedox {
namespace palo {
using util::CsvLineDecoder;
using util::TOKEN_LIST;

template<typename T, typename R> static void unserializeAssignList(const std::string& token, T& eList)
{
	T().swap(eList);
	TOKEN_LIST tokenList = CsvLineDecoder::decode(token, ',');
	TOKEN_LIST::const_iterator tokenListEnd(tokenList.end());
	for (TOKEN_LIST::const_iterator it(tokenList.begin()); it != tokenListEnd; ++it) {
		eList.push_back(util::lexicalConversion(R, std::string, (*it).token));
	}
}

std::istream& csvgetline(std::istream& in, std::string& csvline);

inline void unserializeFromCsv(SERVER_INFO& si, std::istream& in);
inline void unserializeFromCsv(SUPERVISION_SERVER_INFO& si, std::istream& in);
inline void unserializeFromCsv(DATABASE_INFO& db, std::istream& in);
inline void unserializeFromCsv(DATABASE_INFO_PERMISSIONS& db, std::istream& in);
inline void unserializeFromCsv(DATABASE_INFO_PERMISSIONS_LIST& dbList, std::istream& in);
inline void unserializeFromCsv(DIMENSION_INFO& di, std::istream& in);
inline void unserializeFromCsv(DIMENSION_INFO_PERMISSIONS& dim, std::istream& in);
inline void unserializeFromCsv(DIMENSION_INFO_PERMISSIONS_LIST& dimList, std::istream& in);
inline void unserializeFromCsv(ELEMENT_INFO& ei, std::istream& in);
inline void unserializeFromCsv(ELEMENT_INFO_PERM& ei, std::istream& in);
inline void unserializeFromCsv(CUBE_INFO& ci, std::istream& in);
inline void unserializeFromCsv(CUBE_INFO_PERMISSIONS& dim, std::istream& in);
inline void unserializeFromCsv(CUBE_INFO_PERMISSIONS_LIST& dimList, std::istream& in);
inline void unserializeFromCsv(CELL_VALUE& cv, std::istream& in, jedox::util::CsvTokenFromStream *tfs = 0);
inline void unserializeFromCsv(CELL_VALUE_PATH& cvp, std::istream& in, jedox::util::CsvTokenFromStream *tfs = 0);
inline void unserializeFromCsv(CELL_VALUE_EXPORTED& cve, std::istream& in, TOKEN_LIST *tokens = 0, UINT *c = 0);
inline void unserializeFromCsv(CELL_VALUE_PROPS& CellValueProps, std::istream& in);
inline void unserializeFromCsv(CELL_VALUE_PATH_PROPS& CellValuePathProps, std::istream& in);
inline void unserializeFromCsv(CELL_VALUE_EXPORTED_PROPS& cellValueExpProps, std::istream& in);
inline void unserializeFromCsv(LOGIN_DATA& logindata, std::istream& in);
inline void unserializeFromCsv(RULE_INFO& ruleinfo, std::istream& in);
inline void unserializeFromCsv(LOCK_INFO& lockinfo, std::istream& in);
inline void unserializeFromCsv(DRILLTHROUGH_INFO& drillthroughinfo, std::istream& in);
inline void unserializeFromCsv(Cell_Value_C& cellValueC, std::istream& in);
inline void unserializeFromCsv(LICENSE_INFO& lic, std::istream& in);
inline void unserializeFromCsv(LICENSE_LIST& lic_list, std::istream& in);
inline void unserializeFromCsv(USER_INFO& lic, std::istream& in);

class Serializer {
public:
	enum Type {
		CSV = 0, XML = 1
	} type;

	explicit Serializer(Type mode = CSV) :
		m_Mode(mode)
	{
	}

	virtual ~Serializer()
	{
	}
	;

private:
	friend std::ostream& operator <<(std::ostream& stream, const Serializer& serializer);
	friend std::istream& operator >>(std::istream& stream, const Serializer& serializer);

	friend std::ostream& operator <<(std::ostream&, const SERVER_INFO&);
	friend std::istream& operator >>(std::istream&, SERVER_INFO&);

	friend std::ostream& operator <<(std::ostream&, const SUPERVISION_SERVER_INFO&);
	friend std::istream& operator >>(std::istream&, SUPERVISION_SERVER_INFO&);

	friend std::ostream& operator <<(std::ostream&, const DATABASE_INFO&);
	friend std::istream& operator >>(std::istream&, DATABASE_INFO&);
	friend std::istream& operator >>(std::istream&, DATABASE_INFO_PERMISSIONS&);
	friend std::istream& operator >>(std::istream&, DATABASE_INFO_PERMISSIONS_LIST&);

	friend std::ostream& operator <<(std::ostream&, const DIMENSION_INFO&);
	friend std::istream& operator >>(std::istream&, DIMENSION_INFO&);
	friend std::istream& operator >>(std::istream&, DIMENSION_INFO_PERMISSIONS&);
	friend std::istream& operator >>(std::istream&, DIMENSION_INFO_PERMISSIONS_LIST&);

	friend std::ostream& operator <<(std::ostream&, const ELEMENT_INFO&);
	friend std::istream& operator >>(std::istream&, ELEMENT_INFO&);
	friend std::ostream& operator <<(std::ostream&, const ELEMENT_INFO_PERM&);
	friend std::istream& operator >>(std::istream&, ELEMENT_INFO_PERM&);

	friend std::ostream& operator <<(std::ostream&, const CUBE_INFO&);
	friend std::istream& operator >>(std::istream&, CUBE_INFO&);
	friend std::istream& operator >>(std::istream&, CUBE_INFO_PERMISSIONS&);
	friend std::istream& operator >>(std::istream&, CUBE_INFO_PERMISSIONS_LIST&);

	friend std::istream& operator >>(std::istream&, CELL_VALUE&);

	friend std::istream& operator >>(std::istream&, CELL_VALUE_PATH&);

	friend std::istream& operator >>(std::istream&, CELL_VALUE_EXPORTED&);

	friend std::istream& operator>>(std::istream& in, CELL_VALUE_PROPS& CellValueProps);
	friend std::istream& operator>>(std::istream& in, CELL_VALUE_PATH_PROPS& CellValuePathProps);
	friend std::istream& operator>>(std::istream& in, CELL_VALUE_EXPORTED_PROPS& cellValueExpProps);

	friend std::istream& operator>>(std::istream&, LOGIN_DATA&);

	friend std::istream& operator>>(std::istream&, RULE_INFO&);

	friend std::istream& operator>>(std::istream&, LOCK_INFO&);

	friend std::istream& operator>>(std::istream&, DRILLTHROUGH_INFO&);

	friend std::istream& operator>>(std::istream&, LICENSE_INFO&);
	friend std::istream& operator>>(std::istream&, LICENSE_LIST&);

	friend std::istream& operator>>(std::istream&, USER_INFO&);

	// faster Method for C
	friend std::istream& operator>>(std::istream&, Cell_Value_C&);

	Type m_Mode;
	static const long m_StreamIdx;
};

const long Serializer::m_StreamIdx = std::ios_base::xalloc();

std::ostream& operator <<(std::ostream& stream, const Serializer& serializer)
{
	stream.iword(Serializer::m_StreamIdx) = serializer.m_Mode;
	return stream;
}

std::istream& operator >>(std::istream& stream, const Serializer& serializer)
{
	stream.iword(Serializer::m_StreamIdx) = serializer.m_Mode;
	return stream;
}

std::ostream& xml(std::ostream& os)
{
	os << Serializer(Serializer::XML);
	return os;
}

std::ostream& csv(std::ostream& os)
{
	os << Serializer(Serializer::CSV);
	return os;
}

std::istream& xml(std::istream& is)
{
	is >> Serializer(Serializer::XML);
	return is;
}

std::istream& csv(std::istream& is)
{
	is >> Serializer(Serializer::CSV);
	return is;
}

std::ostream& operator<<(std::ostream& out, const SERVER_INFO& /* serverInfo */)
{
	switch (out.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		// Is the default
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
	}
	return out;
}

std::istream& operator>>(std::istream& in, SERVER_INFO& serverInfo)
{
	switch (in.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		unserializeFromCsv(serverInfo, in);
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
		break;
	}
	return in;
}

std::ostream& operator<<(std::ostream& out, const SUPERVISION_SERVER_INFO& /* serverInfo */)
{
	switch (out.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		// Is the default
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
	}
	return out;
}

std::istream& operator>>(std::istream& in, SUPERVISION_SERVER_INFO& svsInfo)
{
	switch (in.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		unserializeFromCsv(svsInfo, in);
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
		break;
	}
	return in;
}

std::istream& operator>>(std::istream& in, LICENSE_INFO& licenseInfo)
{
	switch (in.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		unserializeFromCsv(licenseInfo, in);
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
		break;
	}
	return in;
}

std::istream& operator>>(std::istream& in, LICENSE_LIST& licenseList)
{
	switch (in.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		unserializeFromCsv(licenseList, in);
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
		break;
	}
	return in;
}

std::ostream& operator<<(std::ostream& out, const DATABASE_INFO& /* databaseInfo */)
{
	switch (out.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		// Is the default
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
	}
	return out;
}

std::istream& operator>>(std::istream& in, DATABASE_INFO& databaseInfo)
{
	switch (in.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		unserializeFromCsv(databaseInfo, in);
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
		break;
	}
	return in;
}

PERMISSION stringToPermission(std::string &s)
{
	if (s == "S") {
		return SPLASH_PERM;
	} else if (s == "D") {
		return DELETE_PERM;
	} else if (s == "W") {
		return WRITE_PERM;
	} else if (s == "R") {
		return READ_PERM;
	} else {
		return NONE_PERM;
	}
}

std::istream& operator>>(std::istream& in, DATABASE_INFO_PERMISSIONS& databaseInfo)
{
	switch (in.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		unserializeFromCsv(databaseInfo, in);
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
		break;
	}
	return in;
}

std::istream& operator>>(std::istream& in, DATABASE_INFO_PERMISSIONS_LIST& dbList)
{
	switch (in.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		unserializeFromCsv(dbList, in);
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
		break;
	}
	return in;
}

std::istream& operator>>(std::istream& in, DIMENSION_INFO_PERMISSIONS_LIST& dimList)
{
	switch (in.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		unserializeFromCsv(dimList, in);
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
		break;
	}
	return in;
}

std::istream& operator>>(std::istream& in, CUBE_INFO_PERMISSIONS_LIST& cubeList)
{
	switch (in.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		unserializeFromCsv(cubeList, in);
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
		break;
	}
	return in;
}

std::ostream& operator<<(std::ostream& out, const DIMENSION_INFO& /* dimensionInfo */)
{
	switch (out.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		// Is the default
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
		break;
	}

	return out;
}

std::istream& operator>>(std::istream& in, DIMENSION_INFO_PERMISSIONS& dimensionInfo)
{
	switch (in.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		unserializeFromCsv(dimensionInfo, in);
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
		break;
	}
	return in;
}

std::istream& operator>>(std::istream& in, DIMENSION_INFO& dimensionInfo)
{
	switch (in.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		unserializeFromCsv(dimensionInfo, in);
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
		break;
	}
	return in;
}

std::istream& operator>>(std::istream& in, CUBE_INFO_PERMISSIONS& cubeInfo)
{
	switch (in.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		unserializeFromCsv(cubeInfo, in);
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
		break;
	}
	return in;
}

std::ostream& operator<<(std::ostream& out, const ELEMENT_INFO& /* elementInfo */)
{
	switch (out.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		// Is the default
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
	}

	return out;
}

std::istream& operator>>(std::istream& in, ELEMENT_INFO& elementInfo)
{
	switch (in.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		unserializeFromCsv(elementInfo, in);
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
		break;
	}
	return in;
}

std::ostream& operator<<(std::ostream& out, const ELEMENT_INFO_PERM& /* elementInfo */)
{
	switch (out.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		// Is the default
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
	}

	return out;
}

std::istream& operator>>(std::istream& in, ELEMENT_INFO_PERM& elementInfo)
{
	switch (in.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		unserializeFromCsv(elementInfo, in);
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
		break;
	}
	return in;
}

std::ostream& operator<<(std::ostream& out, const CUBE_INFO& /* cubeInfo */)
{
	switch (out.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		// Is the default
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
		break;
	}

	return out;
}

std::istream& operator>>(std::istream& in, CUBE_INFO& cubeInfo)
{
	switch (in.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		unserializeFromCsv(cubeInfo, in);
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
		break;
	}
	return in;
}

std::istream& operator>>(std::istream& in, CELL_VALUE& CellValue)
{
	switch (in.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		unserializeFromCsv(CellValue, in);
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
		break;
	}
	return in;
}

std::istream& operator>>(std::istream& in, CELL_VALUE_EXPORTED& cellValueExp)
{
	switch (in.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		unserializeFromCsv(cellValueExp, in);
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
		break;
	}
	return in;
}

std::istream& operator>>(std::istream& in, CELL_VALUE_PATH& cellValuePath)
{
	switch (in.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		unserializeFromCsv(cellValuePath, in);
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
		break;
	}
	return in;
}

std::istream& operator>>(std::istream& in, CELL_VALUE_PROPS& CellValueProps)
{
	switch (in.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		unserializeFromCsv(CellValueProps, in);
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
		break;
	}
	return in;
}

std::istream& operator>>(std::istream& in, CELL_VALUE_PATH_PROPS& CellValuePathProps)
{
	switch (in.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		unserializeFromCsv(CellValuePathProps, in);
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
		break;
	}
	return in;
}

std::istream& operator>>(std::istream& in, CELL_VALUE_EXPORTED_PROPS& cellValueExpProps)
{
	switch (in.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		unserializeFromCsv(cellValueExpProps, in);
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
		break;
	}
	return in;
}

std::istream& operator>>(std::istream& in, LOGIN_DATA& logindata)
{
	switch (in.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		unserializeFromCsv(logindata, in);
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
		break;
	}
	return in;
}

std::istream& operator>>(std::istream& in, RULE_INFO& ri)
{
	switch (in.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		unserializeFromCsv(ri, in);
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
		break;
	}
	return in;
}

std::istream& operator>>(std::istream& in, LOCK_INFO& li)
{
	switch (in.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		unserializeFromCsv(li, in);
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
		break;
	}
	return in;
}

std::istream& operator>>(std::istream& in, DRILLTHROUGH_INFO& ti)
{
	switch (in.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		unserializeFromCsv(ti, in);
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
		break;
	}
	return in;
}

std::istream& operator>>(std::istream& in, USER_INFO& user_info)
{
	switch (in.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		unserializeFromCsv(user_info, in);
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
		break;
	}
	return in;
}

void unserializeFromCsv(SERVER_INFO& si, std::istream& in)
{
	jedox::util::CsvTokenFromStream tfs(in);
	tfs.get(si.major_version, 0);
	tfs.get(si.minor_version, 0);
	tfs.get(si.bugfix_version, 0);
	tfs.get(si.build_number, 0);
	tfs.get(si.encryption, 0);
	tfs.get(si.httpsPort, 0);
	tfs.get(si.data_sequence_number, invalid_server_info_data_sequence_number);
}

void unserializeFromCsv(SUPERVISION_SERVER_INFO& svs, std::istream& in)
{
	jedox::util::CsvTokenFromStream tfs(in);

	tfs.get(svs.svs_active, false);
	UINT login_mode = 0;
	tfs.get(login_mode, 0);
	svs.login_mode = SVS_LOGIN_MODE(login_mode);

	tfs.get(svs.cube_worker_active, false);
	tfs.get(svs.drill_through_enabled, false);
	tfs.get(svs.dimension_worker_enabled, false);
	tfs.get(svs.windows_sso_enabled, false);
}

void unserializeFromCsv(DATABASE_INFO& db, std::istream& in)
{
	jedox::util::CsvTokenFromStream tfs(in);

	tfs.get(db.database, -1);

	tfs.get(db.ndatabase);
	tfs.get(db.number_dimensions, 0);
	tfs.get(db.number_cubes, 0);
	int db_status = 0;
	tfs.get(db_status, 0);
	switch (db_status) {
	case DATABASE_INFO::LOADED:
		db.status = DATABASE_INFO::LOADED;
		break;

	case DATABASE_INFO::UNLOADED:
		db.status = DATABASE_INFO::UNLOADED;
		break;

	case DATABASE_INFO::CHANGED:
		db.status = DATABASE_INFO::CHANGED;
		break;

	case DATABASE_INFO::LOADING:
		db.status = DATABASE_INFO::LOADING;
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_DBINFOTYPE);
	}

	int db_type = 0;
	tfs.get(db_type, 0);
	switch (db_type) {
	case DATABASE_INFO::NORMAL:
		db.type = DATABASE_INFO::NORMAL;
		break;

	case DATABASE_INFO::SYSTEM:
		db.type = DATABASE_INFO::SYSTEM;
		break;

	case DATABASE_INFO::USERINFO:
		db.type = DATABASE_INFO::USERINFO;
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_DBINFOTYPE);
	}
}

void unserializeFromCsv(DATABASE_INFO_PERMISSIONS& db, std::istream& in)
{
	jedox::util::CsvTokenFromStream tfs(in);
	unserializeFromCsv((DATABASE_INFO &)db, in);
	std::string perms;
	tfs.get(perms); // skip token
	tfs.get(perms);
	db.permission = stringToPermission(perms);
}

void unserializeFromCsv(DIMENSION_INFO_PERMISSIONS& dim, std::istream& in)
{
	jedox::util::CsvTokenFromStream tfs(in);
	unserializeFromCsv((DIMENSION_INFO &)dim, in);
	std::string perms;
	tfs.get(perms); // skip token
	tfs.get(perms);
	dim.permission = stringToPermission(perms);
}

void unserializeFromCsv(CUBE_INFO_PERMISSIONS& cip, std::istream& in)
{
	jedox::util::CsvTokenFromStream tfs(in);
	unserializeFromCsv((CUBE_INFO &)cip, in);
	std::string perms;
	tfs.get(perms); // skip token
	tfs.get(perms);
	cip.permission = stringToPermission(perms);
}

void unserializeFromCsv(DATABASE_INFO_PERMISSIONS_LIST& dbList, std::istream& in)
{
	jedox::util::CsvTokenFromStream tfs(in);

	while (!in.eof()) {
		DATABASE_INFO_PERMISSIONS dip;
		in >> dip;
		dbList.databases.push_back(dip);
	}
}

void unserializeFromCsv(DIMENSION_INFO_PERMISSIONS_LIST& dimList, std::istream& in)
{
	jedox::util::CsvTokenFromStream tfs(in);

	while (!in.eof()) {
		DIMENSION_INFO_PERMISSIONS dip;
		in >> dip;
		dimList.dimensions.push_back(dip);
	}
}

void unserializeFromCsv(CUBE_INFO_PERMISSIONS_LIST& cubeList, std::istream& in)
{
	jedox::util::CsvTokenFromStream tfs(in);

	while (!in.eof()) {
		CUBE_INFO_PERMISSIONS cip;
		in >> cip;
		cubeList.cubes.push_back(cip);
	}
}

void unserializeFromCsv(DIMENSION_INFO& di, std::istream& in)
{
	jedox::util::CsvTokenFromStream tfs(in);
	tfs.get(di.dimension, -1);
	tfs.get(di.ndimension);
	tfs.get(di.number_elements, 0);
	tfs.get(di.maximum_level, 0);
	tfs.get(di.maximum_indent, 0);
	tfs.get(di.maximum_depth, 0);
	int dim_type = 0;
	tfs.get(dim_type, 0);
	switch (dim_type) {
	case DIMENSION_INFO::NORMAL:
		di.type = DIMENSION_INFO::NORMAL;
		break;

	case DIMENSION_INFO::SYSTEM:
		di.type = DIMENSION_INFO::SYSTEM;
		break;

	case DIMENSION_INFO::ATTRIBUTE:
		di.type = DIMENSION_INFO::ATTRIBUTE;
		break;

	case DIMENSION_INFO::USERINFO:
		di.type = DIMENSION_INFO::USERINFO;
		break;

	case DIMENSION_INFO::SYSTEM_ID:
		di.type = DIMENSION_INFO::SYSTEM_ID;
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_DBINFOTYPE);
	}

	tfs.get(di.assoc_dimension, -1);
	tfs.get(di.attribute_cube, -1);
	tfs.get(di.rights_cube, -1);
}

void unserializeFromCsv(ELEMENT_INFO& ei, std::istream& in)
{
	jedox::util::CsvTokenFromStream tfs(in);

	tfs.get(ei.element, -1L);
	tfs.get(ei.nelement);
	tfs.get(ei.position, 0);
	tfs.get(ei.level, 0);
	tfs.get(ei.indent, 0);
	tfs.get(ei.depth, 0);
	int get_type;
	tfs.get(get_type, 0);
	switch (get_type) {
	case ELEMENT_INFO::NUMERIC:
		ei.type = ELEMENT_INFO::NUMERIC;
		break;

	case ELEMENT_INFO::STRING:
		ei.type = ELEMENT_INFO::STRING;
		break;

	case ELEMENT_INFO::CONSOLIDATED:
		ei.type = ELEMENT_INFO::CONSOLIDATED;
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_ELEMENTINFOTYPE);
	}

	tfs.get(ei.number_parents, 0);
	tfs.get_list(ei.parents, ei.number_parents);
	assert( ei.number_parents == ei.parents.size() );

	tfs.get(ei.number_children, 0);
	tfs.get_list(ei.children, ei.number_children);
	assert( ei.number_children == ei.children.size() );

	tfs.get_list(ei.weights, ei.number_children);
	assert( ei.number_children == ei.weights.size() );

	// 3.3 compatibility
	//int lock;
	//tfs.get(lock, 0);
}

void unserializeFromCsv(ELEMENT_INFO_PERM& ei, std::istream& in)
{
	jedox::util::CsvTokenFromStream tfs(in);
	unserializeFromCsv((ELEMENT_INFO &)ei, in);
	tfs.get(ei.permission);
}

void unserializeFromCsv(CUBE_INFO& ci, std::istream& in)
{
	jedox::util::CsvTokenFromStream tfs(in);
	std::string val;

	tfs.get(ci.cube, -1L);
	tfs.get(ci.ncube);
	tfs.get(ci.number_dimensions, 0);

	//std::string dim_list;
	//tfs.get(dim_list);
	tfs.get_list(ci.dimensions, ci.number_dimensions);
	//unserializeAssignList<DIMENSION_LIST, long>( dim_list, ci.dimensions );
	assert( ci.number_dimensions == ci.dimensions.size() );

	tfs.get(ci.number_cells, 0);
	tfs.get(ci.number_filled_cells, 0);
	int get_status;
	tfs.get(get_status, 0);
	switch (get_status) {
	case CUBE_INFO::LOADED:
		ci.status = CUBE_INFO::LOADED;
		break;

	case CUBE_INFO::UNLOADED:
		ci.status = CUBE_INFO::UNLOADED;
		break;

	case CUBE_INFO::CHANGED:
		ci.status = CUBE_INFO::CHANGED;
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_CUBEINFOTYPE);

	}

	int get_type;
	tfs.get(get_type, 0);
	switch (get_type) {
	case CUBE_INFO::NORMAL:
		ci.type = CUBE_INFO::NORMAL;
		break;

	case CUBE_INFO::SYSTEM:
		ci.type = CUBE_INFO::SYSTEM;
		break;

	case CUBE_INFO::ATTRIBUTE:
		ci.type = CUBE_INFO::ATTRIBUTE;
		break;

	case CUBE_INFO::USERINFO:
		ci.type = CUBE_INFO::USERINFO;
		break;

	case CUBE_INFO::GPU:
		ci.type = CUBE_INFO::GPU;
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_CUBEINFOTYPE);
	}
}

void unserializeFromCsv(CELL_VALUE& cv, std::istream& in, jedox::util::CsvTokenFromStream *tfs)
{
	jedox::util::CsvTokenFromStream t(in);
	if (!tfs) {
		tfs = &t;
	}

	int get_type;
	tfs->get(get_type, -1);
	switch (get_type) {
	case CELL_VALUE::NUMERIC: {
		cv.type = CELL_VALUE::NUMERIC;
		tfs->get(cv.exists, false);
		tfs->get(cv.val.d, 0.0);
	}
		break;
	case CELL_VALUE::STRING: {
		cv.type = CELL_VALUE::STRING;
		tfs->get(cv.exists, false);
		tfs->get(cv.val.s);
	}
		break;
	case CELL_VALUE::ERROR: {
		cv.type = CELL_VALUE::ERROR;
		tfs->get(cv.val.errorcode, static_cast<unsigned int> (0));
		tfs->get(cv.val.s);
	}
		break;
	default:
		cv.type = CELL_VALUE::ERROR;
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_CELLVALUEINFOTYPE);

	}
	tfs->get(cv.ruleID, -1L);
	int lock_status;
	tfs->get(lock_status, static_cast<int> (Unlocked));
	cv.lock_status = static_cast<LOCK_STATUS> (lock_status);
}

void unserializeFromCsv(CELL_VALUE_PATH& cvp, std::istream& in, jedox::util::CsvTokenFromStream *tfs)
{
	jedox::util::CsvTokenFromStream t(in);
	if (!tfs) {
		tfs = &t;
	}

	int get_type;
	tfs->get(get_type, -1);
	switch (get_type) {
	case CELL_VALUE::NUMERIC: {
		cvp.type = CELL_VALUE::NUMERIC;
		tfs->get(cvp.exists, false);
		tfs->get(cvp.val.d, 0.0);
	}
		break;
	case CELL_VALUE::STRING: {
		cvp.type = CELL_VALUE::STRING;
		tfs->get(cvp.exists, false);
		tfs->get(cvp.val.s);
	}
		break;
	case CELL_VALUE::ERROR: {
		cvp.type = CELL_VALUE::ERROR;
		tfs->get(cvp.val.errorcode, static_cast<unsigned int> (0));
		tfs->get(cvp.val.s);
	}
		break;
	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_CELLVALUEINFOTYPE);

	}
	tfs->get_list(cvp.path);
	tfs->get(cvp.ruleID, -1L);
	int lock_status;
	tfs->get(lock_status, static_cast<int> (Unlocked));
	cvp.lock_status = static_cast<LOCK_STATUS> (lock_status);
}

void unserializeFromCsv(CELL_VALUE_EXPORTED& cve, std::istream& in, TOKEN_LIST *tokens, UINT *c)
{
	TOKEN_LIST t;
	if (!tokens) {
		std::string csvLine;
		csvgetline(in, csvLine);
		t = CsvLineDecoder::decode(csvLine);
		tokens = &t;
	}
	std::string val;

	UINT counter = c ? *c : 0;

	if (tokens->size() == 2) {
		cve.type = CELL_VALUE_EXPORTED::EXPORTINFO;
		val = (*tokens)[counter++].token;
		cve.exportinfo.usedcells = (val.empty()) ? 0 : util::lexicalConversion(long double, std::string, val);
		val = (*tokens)[counter++].token;
		cve.exportinfo.allcells = (val.empty()) ? 0 : util::lexicalConversion(long double, std::string, val);
	} else if (tokens->size() > 0) {
		cve.type = CELL_VALUE_EXPORTED::CELLVALUE;
		val = (*tokens)[counter++].token;
		switch ((val.empty()) ? 0 : util::lexicalConversion(int, std::string, val)) {
		case CELL_VALUE_PATH_EXPORTED::NUMERIC:
			cve.cvp.type = CELL_VALUE_PATH_EXPORTED::NUMERIC;
			break;

		case CELL_VALUE_PATH_EXPORTED::STRING:
			cve.cvp.type = CELL_VALUE_PATH_EXPORTED::STRING;
			break;

		case CELL_VALUE_PATH_EXPORTED::ERROR:
			cve.cvp.type = CELL_VALUE_PATH_EXPORTED::ERROR;
			break;

		default:
			LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_CELLVALUEPATHTYPE);

		}

		switch (cve.cvp.type) {
		case CELL_VALUE_PATH::NUMERIC:
			val = (*tokens)[counter++].token;
			cve.cvp.exists = (val.empty()) ? false : util::lexicalConversion(bool, std::string, val);
			val = (*tokens)[counter++].token;
			cve.cvp.val.d = (val.empty()) ? 0 : util::lexicalConversion(double, std::string, val);
			break;

		case CELL_VALUE_PATH::STRING:
			val = (*tokens)[counter++].token;
			cve.cvp.exists = (val.empty()) ? false : util::lexicalConversion(bool, std::string, val);
			cve.cvp.val.s = (*tokens)[counter++].token;
			break;

		case CELL_VALUE_PATH::ERROR:
			val = (*tokens)[counter++].token;
			cve.cvp.val.errorcode = (val.empty()) ? 0 : util::lexicalConversion(unsigned int, std::string, val);
			cve.cvp.val.s = (*tokens)[counter++].token;
			break;

		default:
			LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_CELLVALUEPATHTYPE);

		}
		unserializeAssignList<ELEMENT_LIST, long> ((*tokens)[counter++].token, cve.cvp.path);
	}
	if (c) {
		*c = counter;
	}
}

void unserializeFromCsv(CELL_VALUE_PROPS& cv, std::istream& in)
{
	jedox::util::CsvTokenFromStream tfs(in);
	unserializeFromCsv((CELL_VALUE &)cv, in, &tfs);
	std::string props;
	tfs.get(props);
	if (!props.empty()) {
		TOKEN_LIST tokens = CsvLineDecoder::decode(props, ',');
		for (TOKEN_LIST::const_iterator it = tokens.begin(); it != tokens.end(); ++it) {
			cv.prop_vals.push_back(it->token);
		}
	}
}

void unserializeFromCsv(CELL_VALUE_PATH_PROPS& cv, std::istream& in)
{
	jedox::util::CsvTokenFromStream tfs(in);
	unserializeFromCsv((CELL_VALUE_PATH &)cv, in, &tfs);
	std::string props;
	tfs.get(props);
	if (!props.empty()) {
		TOKEN_LIST tokens = CsvLineDecoder::decode(props, ',');
		for (TOKEN_LIST::const_iterator it = tokens.begin(); it != tokens.end(); ++it) {
			cv.prop_vals.push_back(it->token);
		}
	}
}

void unserializeFromCsv(CELL_VALUE_EXPORTED_PROPS& cv, std::istream& in)
{
	std::string csvLine;
	csvgetline(in, csvLine);
	TOKEN_LIST tokens = CsvLineDecoder::decode(csvLine);
	UINT counter = 0;
	unserializeFromCsv((CELL_VALUE_EXPORTED &)cv, in, &tokens, &counter);
	if (counter < tokens.size()) {
		std::string prop_str = csvLine.substr(tokens[counter].startPos, csvLine.size() - tokens[counter].startPos - 2);
		if (!prop_str.empty()) {
			TOKEN_LIST props = CsvLineDecoder::decode(prop_str, ',');
			for (TOKEN_LIST::const_iterator it = props.begin(); it != props.end(); ++it) {
				cv.prop_vals.push_back(it->token);
			}
		}
	}
}

void unserializeFromCsv(LOGIN_DATA& logindata, std::istream& in)
{
	jedox::util::CsvTokenFromStream tfs(in);
	tfs.get(logindata.sid);
	tfs.get(logindata.ttl, 0);
	tfs.get(logindata.optFeatures);
}

void unserializeFromCsv(RULE_INFO& ri, std::istream& in)
{
	std::string csvLine;
	csvgetline(in, csvLine);
	TOKEN_LIST tokens = CsvLineDecoder::decode(csvLine);
	std::string val;

	UINT counter = 0;
	size_t count = tokens.size();
	val = tokens[counter++].token;
	ri.identifier = val.empty() ? 0 : util::lexicalConversion(long, std::string, val);
	ri.definition = tokens[counter++].token;

	// First Extension
	if (counter < count - 2) {
		ri.extern_id = tokens[counter++].token;
		ri.comment = tokens[counter++].token;
		val = tokens[counter++].token;
		ri.timestamp = (val.empty() || (val[0] == '\n')) ? 0 : util::lexicalConversion(unsigned long long, std::string, val);
	} else {
		ri.extern_id = "";
		ri.comment = "";
		ri.timestamp = 0;
	}

	// Second Extension
	if (counter < count) {
		val = tokens[counter++].token;
		ri.activated = (val.empty() || (val[0] == '\n')) ? true : util::lexicalConversion(bool, std::string, val);
	} else {
		ri.activated = true;
	}

	// Third Extension
	if (counter < count) {
		val = tokens[counter++].token;
		ri.position = (val.empty() || (val[0] == '\n')) ? 0 : util::lexicalConversion(double, std::string, val);
	} else {
		ri.position = 0;
	}
}

void unserializeFromCsv(LOCK_INFO& li, std::istream& in)
{
	std::string csvLine;
	std::getline(in, csvLine);
	TOKEN_LIST tokens = CsvLineDecoder::decode(csvLine);
	std::string val;

	UINT counter = 0;

	val = tokens[counter++].token;
	li.lockid = (val.empty()) ? 0 : util::lexicalConversion(unsigned long, std::string, val);

	li.area.clear();
	val = tokens[counter++].token;

	// Split the area-string at commas.
	boost::regex regComma(",");
	std::vector<std::string> vec;

	// boost - splitting strings with regex_token_iterator, page 151.
	boost::sregex_token_iterator it(val.begin(), val.end(), regComma, -1);
	boost::sregex_token_iterator end;
	while (it != end) {
		vec.push_back(*it++);
	}

	// Split the parts at colons.
	boost::regex regColon(":");
	std::vector<long> vecl;
	for (unsigned int i = 0; i < vec.size(); i++) {
		vecl.clear();
		boost::sregex_token_iterator it2(vec[i].begin(), vec[i].end(), regColon, -1);
		boost::sregex_token_iterator end2;
		while (it2 != end2) {
			if (*it2 != '*') {
				vecl.push_back(util::lexicalConversion(long, std::string, *it2++));
			} else {
				break;
			}
		}
		li.area.push_back(vecl);
	}

	li.user = tokens[counter++].token;
	val = tokens[counter++].token;
	li.steps = (val.empty()) ? 0 : util::lexicalConversion(unsigned long, std::string, val);
}

bool isComplete(std::string &str)
{
	if (str.length() > 0 && str[str.length() - 1] != ';') {
		return false;
	}

	int quotesCount = 0;
	for (size_t i = 0; i < str.length(); i++) {
		if (str[i] == '\"') {
			quotesCount++;
		}
	}
	return (quotesCount % 2 == 0);
}

void unserializeFromCsv(DRILLTHROUGH_INFO& di, std::istream& in)
{
	std::string csvLine;
	std::getline(in, csvLine);

	// fix of multi-lines string cells
	while (!isComplete(csvLine) && !in.eof()) {
		std::string nextLine;
		std::getline(in, nextLine);
		csvLine += "\n" + nextLine;
	}

	di.line.clear();

	TOKEN_LIST tokens = CsvLineDecoder::decode(csvLine);
	size_t counter, lenght = tokens.size();

	for (counter = 0; counter < lenght; counter++) {
		di.line.push_back(tokens[counter].token);
	}
}

void unserializeFromCsv(LICENSE_INFO& lic, std::istream& in)
{
	jedox::util::CsvTokenFromStream tfs(in);

	tfs.get(lic.key);
	tfs.get(lic.customer);
	tfs.get(lic.version, 0);
	tfs.get(lic.license_count, 0);
	tfs.get(lic.named_count, 0);
	tfs.get(lic.start, 0);
	tfs.get(lic.expiration, 0);
	tfs.get(lic.sharing_limit, 0);
	tfs.get(lic.gpu_count, 0);
	tfs.get(lic.features);

}

void unserializeFromCsv(LICENSE_LIST& lic_list, std::istream& in)
{
	jedox::util::CsvTokenFromStream tfs(in);

	tfs.get(lic_list.hw_key);
	if (lic_list.hw_key.size() > 1) {
		while (!in.eof()) {
			LICENSE_INFO l;
			in >> l;
			lic_list.licenses.push_back(l);
		}
	}
}

void unserializeFromCsv(USER_INFO& user_info, std::istream& in)
{
	jedox::util::CsvTokenFromStream tfs(in);

	tfs.get(user_info.user, -1);
	tfs.get(user_info.nuser);
	tfs.get_list(user_info.groups);
	for (size_t i = 0; i < user_info.groups.size(); ++i) {
		std::string str;
		tfs.get(str, ',');
		user_info.ngroups.push_back(str);
	}
	if (user_info.groups.empty()) {
		std::string dummy;
		tfs.get(dummy);
	}
	tfs.get(user_info.ttl, 0);
	std::string perm;
	tfs.get(perm);

	boost::regex regComma(",");
	boost::regex regColon(":");
	std::vector<std::string> vec;
	boost::sregex_token_iterator it(perm.begin(), perm.end(), regComma, -1);
	boost::sregex_token_iterator end;
	for (;it != end; ++it) {
		std::string val = *it;
		boost::sregex_token_iterator it2(val.begin(), val.end(), regColon, -1);
		std::string name = *it2++;
		std::string v = *it2;
		user_info.permissions[name] = v[0];
	}
	tfs.get(user_info.license_key);
}

inline std::istream& csvgetline(std::istream& in, std::string& csvline)
{
	bool indoublequote = false;
	char c;
	csvline.clear();

	while (in.get(c)) {
		csvline += c;
		if (!indoublequote && (c == '\n')) {
			break;
		} else if (c == '"') {
			indoublequote = !indoublequote;
		}
	}
	return in;
}

// faster method for C

std::istream& operator>>(std::istream& in, Cell_Value_C& cellValueC)
{
	switch (in.iword(Serializer::m_StreamIdx)) {
	case Serializer::XML:
		// Not implemented
		break;

	case Serializer::CSV:
		unserializeFromCsv(cellValueC, in);
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_SERIALIZERTYPE);
		break;
	}
	return in;
}

void unserializeFromCsv(Cell_Value_C& cvc, std::istream& in)
{
	std::string csvLine;
	csvgetline(in, csvLine);
	TOKEN_LIST tokens = CsvLineDecoder::decode(csvLine);

	std::string val;

	UINT counter = 0;

	val = tokens[counter++].token;
	switch ((val.empty()) ? 0 : util::lexicalConversion(int, std::string, val)) {
	case Cell_Value_C::NUMERIC:
		cvc.type = Cell_Value_C::NUMERIC;
		break;

	case Cell_Value_C::STRING:
		cvc.type = Cell_Value_C::STRING;
		break;

	case Cell_Value_C::ERROR:
		cvc.type = Cell_Value_C::ERROR;
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_CELLVALUEINFOTYPE);

	}

	val = tokens[counter++].token;
	if (cvc.type == Cell_Value_C::ERROR) {
		cvc.val.errorcode = (val.empty()) ? 0 : util::lexicalConversion(UINT, std::string, val);
	}

	switch (cvc.type) {
	case Cell_Value_C::NUMERIC:
		val = tokens[counter++].token;
		cvc.val.d = (val.empty()) ? 0 : util::lexicalConversion(double, std::string, val);
		break;

	case Cell_Value_C::STRING:
		cvc.val.s = strdup(tokens[counter++].token.c_str());
		break;

	case Cell_Value_C::ERROR:
		break;

	default:
		LibPaloNGExceptionFactory::raise(LibPaloNGExceptionFactory::PALO_NG_ERROR_UNDEFINED_CELLVALUEINFOTYPE);

	}

}

C_2DARRAY_LONG::C_2DARRAY_LONG(size_t rows, size_t cols) :
	m_rows(0), m_cols(0), m_a(NULL)
{
	size_t prod = rows * cols;
	if (prod > 0) {
		m_a = (long *)calloc(rows * cols, sizeof(long));
		if (m_a == NULL) {
			PaloExceptionFactory::raise(PaloExceptionFactory::ERROR_OUT_OF_MEMORY, ERROR_OUT_OF_MEMORY_MSG, ERROR_OUT_OF_MEMORY_MSG);
		}
		m_rows = rows;
		m_cols = cols;
	}
}

void FreeCell_Values_C_Content(Cell_Values_C& cvc)
{
	size_t i = 0, len = cvc.len;

	for (i = 0; i < len; i++) {
		if ((cvc.a + i)->type == Cell_Value_C::STRING) {
			free((cvc.a + i)->val.s);
			(cvc.a + i)->val.s = NULL;
		}
	}
	free(cvc.a);
	cvc.a = NULL;
	cvc.len = 0;
}

} /* namespace palo */
} /* namespace jedox */
