/* 
 *
 * Copyright (C) 2006-2014 Jedox AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (Version 2) as published
 * by the Free Software Foundation at http://www.gnu.org/copyleft/gpl.html.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * If you are developing and distributing open source applications under the
 * GPL License, then you are free to use Palo under the GPL License.  For OEMs,
 * ISVs, and VARs who distribute Palo with their products, and do not license
 * and distribute their source code under the GPL, Jedox provides a flexible
 * OEM Commercial License.
 *
 * \author Marek Pikulski <marek.pikulski@jedox.com>
 * \author Martin Jakl <martin.jakl@qbicon.cz>
 * \author Jiri Junek <jiri.junek@qbicon.cz>
 * 
 *
 */

#include <libpalo_ng/Network/SocketException.h>

#include <PaloSpreadsheetFuncs/SpreadsheetFuncs.h>
#include <PaloSpreadsheetFuncs/WrongParamCountException.h>
#include <PaloSpreadsheetFuncs/SpreadsheetFuncsException.h>

#include "GenericCellCellSerializer.h"

using namespace Palo::SpreadsheetFuncs;
using namespace Palo::Util;
using namespace std;

namespace Palo {
namespace SpreadsheetFuncs {
class SpreadsheetFuncs::GetdataHelper {
public:
	static string getWordOrWhitespace(string& s)
	{
		const string whitespaces = " \t\r\n\v";

		if (s.empty()) {
			return s;
		}

		string t = s.substr(0, 1);
		if (t.find_first_not_of(whitespaces) == string::npos) {
			// first is whitespace
			s = s.substr(1);
			return t;
		}

		// first is not whitespace
		string::size_type i = s.find_first_of(whitespaces);
		if (i == string::npos) {
			t = s;
			s = "";
			return t;
		} else {
			t = s.substr(0, i);
			s = s.substr(i);
			return t;
		}
	}

	static void _GetdataT(GenericCell& retval, const CellValue& cv)
	{
		const string::size_type max_len = 255;
		if (cv.type == CellValue::STRING) {
			string s, line;
			string val = cv.val.s;
			StringArray result;
			string::size_type cur_len = 0;

			while (!(s = getWordOrWhitespace(val)).empty()) {
				while (s.length() > max_len) {
					// forced break (word too long)
					line += s.substr(0, max_len - cur_len);
					s = s.substr(max_len - cur_len);
					result.push_back(line);
					line = "";
					cur_len = 0;
				}

				// line full
				if (cur_len + s.length() > max_len) {
					result.push_back(line);
					cur_len = 0;
					line = "";
				}

				// linefeed
				if (s == "\n") {
					result.push_back(line);
					cur_len = 0;
					line = "";
				} else {
					line += s;
					cur_len += s.length();
				}
			}

			if (!line.empty())
				result.push_back(line);

			retval.set(result);
		} else {
			retval.set(cv);
		}
	}

	static void _returnGetdataExportResult(GenericCell& retval, const GetdataExportResult& ger)
	{
		GenericCell::ArrayBuilder i = retval.setArray(ger.size());
		for (GetdataExportResult::const_iterator k = ger.begin(); k != ger.end() && k != ger.end(); k++) {
			unique_ptr < GenericCell > z = i.createGenericCell();
			GenericArrayBuilder j = z->setArray(2);
			j.append("path", j.createGenericCell()->set(k->path));
			j.append("value", j.createGenericCell()->set(k->val));
			i.append(*z.get());
		}
	}

	struct DataVCoordinateInfo {
		// true if array was a column
		bool is_vertical;
	};

	struct IndexMapEntry {
		// number of empty cells between to non-empty entries
		size_t pad_count;
		// real index
		size_t cube_idx;
	};

	struct DataVCoordinates {
		vector<DataVCoordinateInfo> info;
		StringArrayArray coords;
		vector<IndexMapEntry> idx_map;

		size_t tail_padding_h;
		size_t tail_padding_v;
	};

	static DataVCoordinates GetdataVParseCoordinates(GenericCell& arg)
	{
		DataVCoordinates result;
		bool have_tail_h = false, have_tail_v = false;

		result.tail_padding_h = result.tail_padding_v = 0;

		size_t k = 0;
		for (GenericCell::Iterator i = arg.getArray(); !i.end(); ++i, k++) {
			DataVCoordinateInfo inf;
			IndexMapEntry idx;

			if (i->getType() == GenericCell::TArray) {
				size_t rows, cols;
				size_t _tail_padding;

				GenericCell::Iterator j = i->getMatrix(rows, cols);
				if ((rows != 1) && (cols != 1)) {
					throw ArgumentException(CurrentSourceLocation, SpreadsheetFuncsErrors::ERROR_NOT_ROW_OR_COLUMN);
				}

				inf.is_vertical = (cols == 1 );

				result.coords.push_back(GetdataVParseStringArray(j, idx.pad_count, _tail_padding));

				if (_tail_padding != 0) {
					if (((inf.is_vertical && have_tail_v) || (!inf.is_vertical && have_tail_h) )) {
						// we ignore it if more than one coordinate-list is over-sized
						// throw ArgumentException( CurrentSourceLocation, SpreadsheetFuncsErrors::ERROR_INVALID_ARGUMENT_FORMAT );
					} else {
						if (inf.is_vertical) {
							have_tail_v = true;
							result.tail_padding_v = _tail_padding;
						} else {
							have_tail_h = true;
							result.tail_padding_h = _tail_padding;
						}
					}
				}
			} else {
				inf.is_vertical = true;
				idx.pad_count = 0;
				result.coords.push_back(StringArray());
				(result.coords.end() - 1)->push_back(i->getString());
			}

			idx.cube_idx = k;
			result.idx_map.push_back(idx);
			result.info.push_back(inf);
		}

		return result;
	}

private:
	static StringArray GetdataVParseStringArray(GenericCell::Iterator& i, size_t& pad_count, size_t& tail_padding)
	{
		StringArray sa;

		size_t _pad_count = 0, _last_pad_count = -1;
		bool is_first = true;

		while (true) {
			if (!i.end() && i->empty()) {
				_pad_count++;
			} else {
				if ((_last_pad_count != _pad_count && _last_pad_count != (size_t) - 1) && !(i.end() && _pad_count > _last_pad_count))
					throw ArgumentException(CurrentSourceLocation, SpreadsheetFuncsErrors::ERROR_INVALID_ARGUMENT_FORMAT);

				if (i.end()) {
					if (_last_pad_count != (size_t) - 1) {
						// >=0
						tail_padding = _pad_count - _last_pad_count;
					} else {
						tail_padding = 0; // fixes for 1233 (needs testing)
						_last_pad_count = _pad_count;
					}
					// done
					break;
				}

				if (!is_first) {
					_last_pad_count = _pad_count;
					_pad_count = 0;
				} else {
					is_first = false;
				}

				sa.push_back(i->getString());
			}

			++i;
		}

		pad_count = _last_pad_count;

		return sa;
	}
};
}
}

SpreadsheetFuncs::SpreadsheetFuncs() :
		SpreadsheetFuncsBase()
{
}

SpreadsheetFuncs::~SpreadsheetFuncs()
{
}

#define SPREADSHEET_FUNC_IMPL_BEGIN(func_name) \
	template<> \
class FuncImplementation<SpreadsheetFuncs, &SpreadsheetFuncs::func_name> : public FuncData \
{ \
	void operator ()(SpreadsheetFuncs* base, GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg )

#define SPREADSHEET_FUNC_IMPL_END(func_name) \
};

#define CLEAR_ERRORINFO \
	try { \
		opts.setError( ErrorInfo());  \
	} catch (...) {  \
		/* for VBA */ \
	} 

void SpreadsheetFuncs::FPaloInit(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	arg.insert(0, "");
	FPaloRegisterServer(retval, opts, arg);
}

void SpreadsheetFuncs::FPaloSetSvs(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.checkArgCount(1);
	SpreadsheetFuncsBase::FPaloSetSvs(arg[0].getConnection());
	retval.set(true);
}

void SpreadsheetFuncs::FPaloDisconnect(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	FPaloRemoveConnection(retval, opts, arg);
}

void SpreadsheetFuncs::FPaloRegisterServer(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	std::string machineString;
	std::string requiredFeatures;
	std::string optionalFeatures;

	size_t arg_length = arg.length();

	if (arg_length < 4) {
		throw WrongParamCountException(CurrentSourceLocation);
	}

	if (arg[3].getType() == GenericCell::TBool) {
		// windows SSO
		bool automatic = arg[3].getBool();
		size_t maxArgs = automatic ? 7 : 9;

		if (arg_length > maxArgs) {
			throw WrongParamCountException(CurrentSourceLocation);
		}
		if (!automatic && arg_length < 6) {
			throw WrongParamCountException(CurrentSourceLocation);
		}
		if (arg_length == maxArgs) {
			optionalFeatures = arg[maxArgs - 1].getString();
		}
		if (arg_length >= maxArgs - 1) {
			requiredFeatures = arg[maxArgs - 2].getString();
		}
		if (arg_length >= maxArgs - 2) {
			machineString = arg[maxArgs - 3].getString();
		}
		bool opt = !optionalFeatures.empty();
		std::string nego = automatic ? "" : arg[maxArgs - 5].getString();
		std::string negoId = automatic ? "" : arg[maxArgs - 4].getString();
		bool finished;
		std::string ret = SpreadsheetFuncsBase::FPaloRegisterServer(arg[0].getString(), arg[1].getString(), (unsigned short int)arg[2].getUInt(), automatic, automatic ? 0 : &finished, automatic ? 0 : &nego, automatic ? 0 : &negoId, machineString, requiredFeatures, optionalFeatures);

		if (automatic) {
			if (opt) {
				GenericCell::ArrayBuilder a = retval.setArray(2);
				a.append(a.createGenericCell()->set(ret));
				a.append(a.createGenericCell()->set(optionalFeatures));
			} else {
				retval.set(ret);
			}
		} else {
			if (opt && finished) {
				GenericCell::ArrayBuilder a = retval.setArray(3);
				a.append(a.createGenericCell()->set(finished));
				a.append(a.createGenericCell()->set(ret));
				a.append(a.createGenericCell()->set(negoId));
				a.append(a.createGenericCell()->set(optionalFeatures));
			} else {
				GenericCell::ArrayBuilder a = retval.setArray(2);
				a.append(a.createGenericCell()->set(finished));
				a.append(a.createGenericCell()->set(ret));
				a.append(a.createGenericCell()->set(negoId));
			}
		}
	} else {
		if (arg_length == 4) {
			// login with sid
			retval.set(SpreadsheetFuncsBase::FPaloRegisterServer(arg[0].getString(), arg[1].getString(), arg[2].getUInt(), arg[3].getString()));
		} else {
			// traditional login
			if (arg_length > 8) {
				throw WrongParamCountException(CurrentSourceLocation);
			}
			switch (arg_length) {
			case 8:
				optionalFeatures = arg[7].getString();
			case 7:
				requiredFeatures = arg[6].getString();
			case 6:
				machineString = arg[5].getString();
			}

			bool opt = !optionalFeatures.empty();

			std::string ret = SpreadsheetFuncsBase::FPaloRegisterServer(arg[0].getString(), arg[1].getString(), (unsigned short int)arg[2].getUInt(), arg[3].getString(), arg[4].getString(), machineString, requiredFeatures, optionalFeatures);

			if (opt) {
				GenericCell::ArrayBuilder a = retval.setArray(2);
				a.append(a.createGenericCell()->set(ret));
				a.append(a.createGenericCell()->set(optionalFeatures));
			} else {
				retval.set(ret);
			}
		}
	}
}

void SpreadsheetFuncs::FPaloPing(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.checkArgCount(1);
	retval.set(SpreadsheetFuncsBase::FPaloPing(arg[0].getConnection()));
}

void SpreadsheetFuncs::FPaloServerInfo(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.checkArgCount(1);
	retval.set(SpreadsheetFuncsBase::FPaloServerInfo(arg[0].getConnection()));
}

void SpreadsheetFuncs::FPaloLicenseInfo(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.checkArgCount(1);
	retval.set(SpreadsheetFuncsBase::FPaloLicenseInfo(arg[0].getConnection()));
}

void SpreadsheetFuncs::FPaloChangePassword(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.checkArgCount(3);
	SpreadsheetFuncsBase::FPaloChangePassword(arg[0].getConnection(), arg[1].getString(), arg[2].getString());
	retval.set(true);
}

void SpreadsheetFuncs::FPaloChangeUserPassword(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.checkArgCount(3);
	SpreadsheetFuncsBase::FPaloChangeUserPassword(arg[0].getConnection(), arg[1].getString(), arg[2].getString());
	retval.set(true);
}

void SpreadsheetFuncs::FPaloCubeClear(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);

	switch (arg.length()) {
	case 3:
		SpreadsheetFuncsBase::FPaloCubeClear(arg[0].getConnection(), arg[1].getString(), arg[2].getString());
		break;

	case 4:
		SpreadsheetFuncsBase::FPaloCubeClear(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getElementListArray());
		break;

	default:
		throw WrongParamCountException(CurrentSourceLocation);
	}
	retval.set(true);
}

void SpreadsheetFuncs::FPaloDimensionClear(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(3);
	// void
	SpreadsheetFuncsBase::FPaloDimensionClear(arg[0].getConnection(), arg[1].getString(), arg[2].getString());
	retval.set(true);
}

void SpreadsheetFuncs::FPaloRootAddDatabase(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);

	size_t arglen = arg.length();
	if (arglen == 2) {
		SpreadsheetFuncsBase::FPaloRootAddDatabase(arg[0].getConnection(), arg[1].getString());
	} else if (arglen == 3) {
		if (arg[2].isNumber()) {
			SpreadsheetFuncsBase::FPaloRootAddDatabase(arg[0].getConnection(), arg[1].getString(), jedox::palo::DATABASE_INFO::TYPE(arg[2].getUInt()));
		} else {
			SpreadsheetFuncsBase::FPaloRootAddDatabase(arg[0].getConnection(), arg[1].getString(), jedox::palo::DATABASE_INFO::NORMAL, arg[2].getString());
		}
	} else {
		throw WrongParamCountException(CurrentSourceLocation);
	}
	retval.set(true);
}

void SpreadsheetFuncs::FPaloRootDeleteDatabase(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(2);
	retval.set(SpreadsheetFuncsBase::FPaloRootDeleteDatabase(arg[0].getConnection(), arg[1].getString()));
}

void SpreadsheetFuncs::FPaloRootSaveDatabase(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);

	switch (arg.length()) {
	case 2:
		retval.set(SpreadsheetFuncsBase::FPaloRootSaveDatabase(arg[0].getConnection(), arg[1].getString()));
		break;
	case 3:
		retval.set(SpreadsheetFuncsBase::FPaloRootSaveDatabase(arg[0].getConnection(), arg[1].getString(), arg[2].getString()));
		break;
	case 4:
		retval.set(SpreadsheetFuncsBase::FPaloRootSaveDatabase(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getBool()));
		break;
	default:
		throw WrongParamCountException(CurrentSourceLocation);
	}
}

void SpreadsheetFuncs::FPaloRootUnloadDatabase(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(2);
	retval.set(SpreadsheetFuncsBase::FPaloRootUnloadDatabase(arg[0].getConnection(), arg[1].getString()));
}

void SpreadsheetFuncs::FPaloDatabaseAddCube(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(4);
	// void
	SpreadsheetFuncsBase::FPaloDatabaseAddCube(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getStringArray());
	retval.set(true);
}

void SpreadsheetFuncs::FPaloDatabaseAddDimension(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(3);
	// void
	SpreadsheetFuncsBase::FPaloDatabaseAddDimension(arg[0].getConnection(), arg[1].getString(), arg[2].getString());
	retval.set(true);
}

void SpreadsheetFuncs::FPaloDatabaseDeleteCube(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(3);
	retval.set(SpreadsheetFuncsBase::FPaloDatabaseDeleteCube(arg[0].getConnection(), arg[1].getString(), arg[2].getString()));
}

void SpreadsheetFuncs::FPaloDatabaseDeleteDimension(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(3);
	retval.set(SpreadsheetFuncsBase::FPaloDatabaseDeleteDimension(arg[0].getConnection(), arg[1].getString(), arg[2].getString()));
}

void SpreadsheetFuncs::FPaloDatabaseLoadCube(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(3);
	retval.set(SpreadsheetFuncsBase::FPaloDatabaseLoadCube(arg[0].getConnection(), arg[1].getString(), arg[2].getString()));
}

void SpreadsheetFuncs::FPaloDatabaseUnloadCube(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(3);
	retval.set(SpreadsheetFuncsBase::FPaloDatabaseUnloadCube(arg[0].getConnection(), arg[1].getString(), arg[2].getString()));
}

void SpreadsheetFuncs::FPaloElementListConsolidationElements(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(4);
	if (arg.length() >= 3 && (arg[2].getType() == GenericCell::TArray || arg[2].getType() == GenericCell::TMatrix)) {
		GenericCell::Iterator it = arg[2].getArray();
		if (it.end()) {
			throw WrongParamCountException(CurrentSourceLocation);
		}
		std::string viewHandle = it->getString();
		if ((++it).end()) {
			throw WrongParamCountException(CurrentSourceLocation);
		}
		std::string dimname = it->getString();
		if ((++it).end()) {
			throw WrongParamCountException(CurrentSourceLocation);
		}
		unsigned int elemPos = it->getUInt();
		jedox::palo::AxisElement::MEMBERS selector = jedox::palo::AxisElement::NAME;
		if (!(++it).end()) {
			selector = (jedox::palo::AxisElement::MEMBERS)it->getUInt();
		}
		retval.set(SpreadsheetFuncsBase::FPaloViewAxisGetChildren(arg[0].getConnection(), arg[1].getString(), viewHandle, dimname, elemPos, arg[3].getString()), selector);
	} else {
		if (arg[3].empty(false))
			retval.setEmpty();
		else
			retval.set(SpreadsheetFuncsBase::FPaloElementListConsolidationElements(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString()));
	}
}

void SpreadsheetFuncs::FPaloDimensionTopElementsCount(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);

	if (arg.length() != 3) {
		throw WrongParamCountException(CurrentSourceLocation);
	}

	retval.set(SpreadsheetFuncsBase::FPaloDimensionTopElementsCount(arg[0].getConnection(), arg[1].getString(), arg[2].getString()));
}

void SpreadsheetFuncs::FPaloDimensionSimpleFlatListElements(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);

	switch (arg.length()) {
	case 3:
		retval.set(*SpreadsheetFuncsBase::FPaloDimensionSimpleFlatListElements(arg[0].getConnection(), arg[1].getString(), arg[2].getString()));
		break;

	case 4:
		retval.set(*SpreadsheetFuncsBase::FPaloDimensionSimpleFlatListElements(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getSLong()));
		break;

	case 5:
		retval.set(*SpreadsheetFuncsBase::FPaloDimensionSimpleFlatListElements(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getSLong(), arg[4].getSLong()));
		break;

	default:
		throw WrongParamCountException(CurrentSourceLocation);
	}
}

void SpreadsheetFuncs::FPaloDimensionSimpleTopListElements(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);

	if (arg.length() >= 3 && (arg[2].getType() == GenericCell::TArray || arg[2].getType() == GenericCell::TMatrix)) {
		GenericCell::Iterator it = arg[2].getArray();
		if (it.end()) {
			throw WrongParamCountException(CurrentSourceLocation);
		}
		std::string viewHandle = it->getString();
		if ((++it).end()) {
			throw WrongParamCountException(CurrentSourceLocation);
		}
		std::string dimname = it->getString();
		if ((++it).end()) {
			throw WrongParamCountException(CurrentSourceLocation);
		}
		unsigned int elemPos = it->getUInt();
		jedox::palo::AxisElement::MEMBERS selector = jedox::palo::AxisElement::NAME;
		if (!(++it).end()) {
			selector = (jedox::palo::AxisElement::MEMBERS)it->getUInt();
		}
		size_t start = (arg.length() >= 4 ? arg[3].getULong() : 0);
		size_t limit = (arg.length() >= 5 ? arg[4].getULong() : -1);
		retval.set(SpreadsheetFuncsBase::FPaloViewAxisGetTop(arg[0].getConnection(), arg[1].getString(), viewHandle, dimname, elemPos, start, limit), selector);
	} else {
		switch (arg.length()) {
		case 3:
			retval.set(*SpreadsheetFuncsBase::FPaloDimensionSimpleTopListElements(arg[0].getConnection(), arg[1].getString(), arg[2].getString()));
			break;

		case 4:
			retval.set(*SpreadsheetFuncsBase::FPaloDimensionSimpleTopListElements(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getSLong()));
			break;

		case 5:
			retval.set(*SpreadsheetFuncsBase::FPaloDimensionSimpleTopListElements(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getSLong(), arg[4].getSLong()));
			break;

		default:
			throw WrongParamCountException(CurrentSourceLocation);
		}
	}
}

void SpreadsheetFuncs::FPaloDimensionSimpleChildrenListElements(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);

	switch (arg.length()) {
	case 4:
		retval.set(*SpreadsheetFuncsBase::FPaloDimensionSimpleChildrenListElements(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getSLong()));
		break;

	case 5:
		retval.set(*SpreadsheetFuncsBase::FPaloDimensionSimpleChildrenListElements(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getSLong(), arg[4].getSLong()));
		break;

	case 6:
		retval.set(*SpreadsheetFuncsBase::FPaloDimensionSimpleChildrenListElements(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getSLong(), arg[4].getSLong(), arg[5].getSLong()));
		break;

	default:
		throw WrongParamCountException(CurrentSourceLocation);
	}
}

void SpreadsheetFuncs::FPaloDimensionReducedFlatListElements(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);

	switch (arg.length()) {
	case 3:
		retval.set(*SpreadsheetFuncsBase::FPaloDimensionReducedFlatListElements(arg[0].getConnection(), arg[1].getString(), arg[2].getString()));
		break;

	case 4:
		retval.set(*SpreadsheetFuncsBase::FPaloDimensionReducedFlatListElements(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getSLong()));
		break;

	case 5:
		retval.set(*SpreadsheetFuncsBase::FPaloDimensionReducedFlatListElements(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getSLong(), arg[4].getSLong()));
		break;

	default:
		throw WrongParamCountException(CurrentSourceLocation);
	}
}

void SpreadsheetFuncs::FPaloDimensionReducedTopListElements(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);

	switch (arg.length()) {
	case 3:
		retval.set(*SpreadsheetFuncsBase::FPaloDimensionReducedTopListElements(arg[0].getConnection(), arg[1].getString(), arg[2].getString()));
		break;

	case 4:
		retval.set(*SpreadsheetFuncsBase::FPaloDimensionReducedTopListElements(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getSLong()));
		break;

	case 5:
		retval.set(*SpreadsheetFuncsBase::FPaloDimensionReducedTopListElements(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getSLong(), arg[4].getSLong()));
		break;

	default:
		throw WrongParamCountException(CurrentSourceLocation);
	}
}

void SpreadsheetFuncs::FPaloDimensionReducedChildrenListElements(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);

	switch (arg.length()) {
	case 4:
		retval.set(*SpreadsheetFuncsBase::FPaloDimensionReducedChildrenListElements(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getSLong()));
		break;

	case 5:
		retval.set(*SpreadsheetFuncsBase::FPaloDimensionReducedChildrenListElements(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getSLong(), arg[4].getSLong()));
		break;

	case 6:
		retval.set(*SpreadsheetFuncsBase::FPaloDimensionReducedChildrenListElements(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getSLong(), arg[4].getSLong(), arg[5].getSLong()));
		break;

	default:
		throw WrongParamCountException(CurrentSourceLocation);
	}
}

void SpreadsheetFuncs::FPaloDimensionListElements2(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	bool filter = false;
	bool showPermission = false;
	if (arg.length() < 3 || arg.length() > 5) {
		throw WrongParamCountException(CurrentSourceLocation);
	}
	if (arg.length() > 3) {
		filter = arg[3].getBool();
		if (arg.length() > 4) {
			showPermission = arg[4].getBool();
			retval.set(*SpreadsheetFuncsBase::FPaloDimensionListElements2Perm(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), filter, showPermission));
			return;
		}
	}
	retval.set(*SpreadsheetFuncsBase::FPaloDimensionListElements2(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), filter));
}

void SpreadsheetFuncs::FPaloDimensionListElements(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	bool filter = false;
	if (arg.length() == 4) {
		filter = arg[3].getBool();
	}
	retval.set(SpreadsheetFuncsBase::FPaloDimensionListElements(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), filter));

}

void SpreadsheetFuncs::FPaloDimensionListCubes(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(3);
	retval.set(SpreadsheetFuncsBase::FPaloDimensionListCubes(arg[0].getConnection(), arg[1].getString(), arg[2].getString()));
}

void SpreadsheetFuncs::FPaloCubeListDimensions(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(3);
	retval.set(SpreadsheetFuncsBase::FPaloCubeListDimensions(arg[0].getConnection(), arg[1].getString(), arg[2].getString()));
}

void SpreadsheetFuncs::FPaloDatabaseListCubes(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if ((arg.length() == 2) || ((arg.length() > 2) && arg[2].isMissing())) {
		retval.set(SpreadsheetFuncsBase::FPaloDatabaseListCubes(arg[0].getConnection(), arg[1].getString()));
	} else if (arg.length() > 2) {
		jedox::palo::CUBE_INFO::TYPE type;
		switch (arg[2].getUInt()) {
		case 0:
			type = jedox::palo::CUBE_INFO::NORMAL;
			break;
		case 1:
			type = jedox::palo::CUBE_INFO::SYSTEM;
			break;
		case 2:
			type = jedox::palo::CUBE_INFO::ATTRIBUTE;
			break;
		case 3:
			type = jedox::palo::CUBE_INFO::USERINFO;
			break;
		case 4:
			type = jedox::palo::CUBE_INFO::GPU;
			break;
		default:
			throw ArgumentException(CurrentSourceLocation, SpreadsheetFuncsErrors::ERROR_INVALID_ARGUMENT_VALUE);
		}
		if (arg.length() == 3) {
			retval.set(SpreadsheetFuncsBase::FPaloDatabaseListCubes(arg[0].getConnection(), arg[1].getString(), type));
		} else if (arg.length() == 4) {
			retval.set(SpreadsheetFuncsBase::FPaloDatabaseListCubesExt(arg[0].getConnection(), arg[1].getString(), type, arg[3].getBool()));
		} else {
			throw WrongParamCountException(CurrentSourceLocation);
		}
	} else {
		throw WrongParamCountException(CurrentSourceLocation);
	}
}

void SpreadsheetFuncs::FPaloDatabaseListDimensions(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	size_t len = arg.length();
	if ((len == 2) || ((len == 3) && arg[2].empty())) {
		retval.set(SpreadsheetFuncsBase::FPaloDatabaseListDimensions(arg[0].getConnection(), arg[1].getString()));
	} else if (len == 3) {
		jedox::palo::DIMENSION_INFO::TYPE type;
		switch (arg[2].getUInt()) {
		case 0:
			type = jedox::palo::DIMENSION_INFO::NORMAL;
			break;
		case 1:
			type = jedox::palo::DIMENSION_INFO::SYSTEM;
			break;
		case 2:
			type = jedox::palo::DIMENSION_INFO::ATTRIBUTE;
			break;
		case 3:
			type = jedox::palo::DIMENSION_INFO::USERINFO;
			break;
		default:
			throw ArgumentException(CurrentSourceLocation, SpreadsheetFuncsErrors::ERROR_INVALID_ARGUMENT_VALUE);
		}
		retval.set(SpreadsheetFuncsBase::FPaloDatabaseListDimensions(arg[0].getConnection(), arg[1].getString(), type));
	} else {
		throw WrongParamCountException(CurrentSourceLocation);
	}
}

void SpreadsheetFuncs::FPaloDatabaseListDimensionsExt(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);

	if (arg.length() == 2) {
		retval.set(SpreadsheetFuncsBase::FPaloDatabaseListDimensionsExt(arg[0].getConnection(), arg[1].getString(), true, false, false, false, false));
	} else if (arg.length() == 3) {
		retval.set(SpreadsheetFuncsBase::FPaloDatabaseListDimensionsExt(arg[0].getConnection(), arg[1].getString(), arg[2].getBool(), false, false, false, false));
	} else if (arg.length() == 4) {
		retval.set(SpreadsheetFuncsBase::FPaloDatabaseListDimensionsExt(arg[0].getConnection(), arg[1].getString(), arg[2].getBool(), arg[3].getBool(), false, false, false));
	} else if (arg.length() == 5) {
		retval.set(SpreadsheetFuncsBase::FPaloDatabaseListDimensionsExt(arg[0].getConnection(), arg[1].getString(), arg[2].getBool(), arg[3].getBool(), arg[4].getBool(), false, false));
	} else if (arg.length() == 6) {
		retval.set(SpreadsheetFuncsBase::FPaloDatabaseListDimensionsExt(arg[0].getConnection(), arg[1].getString(), arg[2].getBool(), arg[3].getBool(), arg[4].getBool(), arg[5].getBool(), false));
	} else if (arg.length() == 7) {
		retval.set(SpreadsheetFuncsBase::FPaloDatabaseListDimensionsExt(arg[0].getConnection(), arg[1].getString(), arg[2].getBool(), arg[3].getBool(), arg[4].getBool(), arg[5].getBool(), arg[6].getBool()));
	} else {
		throw WrongParamCountException(CurrentSourceLocation);
	}
}

void SpreadsheetFuncs::FPaloRootListDatabases(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() == 1)
		retval.set(SpreadsheetFuncsBase::FPaloRootListDatabases(arg[0].getConnection(), false, false, false));
	else if (arg.length() == 2)
		retval.set(SpreadsheetFuncsBase::FPaloRootListDatabases(arg[0].getConnection(), arg[1].getBool(), false, false));
	else if (arg.length() == 3)
		retval.set(SpreadsheetFuncsBase::FPaloRootListDatabases(arg[0].getConnection(), arg[1].getBool(), arg[2].getBool(), false));
	else if (arg.length() == 4)
		retval.set(SpreadsheetFuncsBase::FPaloRootListDatabases(arg[0].getConnection(), arg[1].getBool(), arg[2].getBool(), arg[3].getBool()));
	else
		throw WrongParamCountException(CurrentSourceLocation);
}

void SpreadsheetFuncs::FPaloRootListDatabasesExt(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() == 1) {
		retval.set(SpreadsheetFuncsBase::FPaloRootListDatabasesExt(arg[0].getConnection(), false, false, false));
	} else if (arg.length() == 2) {
		retval.set(SpreadsheetFuncsBase::FPaloRootListDatabasesExt(arg[0].getConnection(), arg[1].getBool(), false, false));
	} else if (arg.length() == 3) {
		retval.set(SpreadsheetFuncsBase::FPaloRootListDatabasesExt(arg[0].getConnection(), arg[1].getBool(), arg[2].getBool(), false));
	} else if (arg.length() == 4) {
		retval.set(SpreadsheetFuncsBase::FPaloRootListDatabasesExt(arg[0].getConnection(), arg[1].getBool(), arg[2].getBool(), arg[3].getBool()));
	} else {
		throw WrongParamCountException(CurrentSourceLocation);
	}
}

void SpreadsheetFuncs::FPaloDatabaseRenameDimension(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(4);
	// void
	SpreadsheetFuncsBase::FPaloDatabaseRenameDimension(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString());
	retval.set(true);
}

void SpreadsheetFuncs::FPaloCubeRename(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(4);
	// void
	SpreadsheetFuncsBase::FPaloCubeRename(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString());
	retval.set(true);
}

void SpreadsheetFuncs::FPaloDimensionInfo(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);

	if (arg.length() == 3) {
		retval.set(SpreadsheetFuncsBase::FPaloDimensionInfo(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), false));
	} else if (arg.length() == 4) {
		retval.set(SpreadsheetFuncsBase::FPaloDimensionInfo(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getBool()));
	} else {
		throw WrongParamCountException(CurrentSourceLocation);
	}
}

void SpreadsheetFuncs::FPaloDatabaseInfo(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);

	if (arg.length() == 2) {
		retval.set(SpreadsheetFuncsBase::FPaloDatabaseInfo(arg[0].getConnection(), arg[1].getString(), true));
	} else {
		throw WrongParamCountException(CurrentSourceLocation);
	}
}

void SpreadsheetFuncs::FPaloCubeInfo(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);

	if (arg.length() == 3) {
		retval.set(SpreadsheetFuncsBase::FPaloCubeInfo(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), false));
	} else if (arg.length() == 4) {
		retval.set(SpreadsheetFuncsBase::FPaloCubeInfo(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getBool()));
	} else {
		throw WrongParamCountException(CurrentSourceLocation);
	}
}

void SpreadsheetFuncs::FPaloSetdataA(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(2);
	size_t arglen = arg.length();
	if ((arg.length() < 6) || (arg.length() > 7)) {
		throw WrongParamCountException(CurrentSourceLocation);
	}
	if (arg[3].empty(false)) {
		retval.setEmpty();
	} else {
		StringArrayArray Lockingarea;
		if (arglen == 7) {
			size_t i, j, rows, cols;
			GenericCell::Iterator it = arg[6].getMatrix(rows, cols);
			StringArray elems(cols);
			std::string tmpstr;
			for (i = 0; i < rows; i++) {
				elems.clear();
				for (j = 0; j < cols; j++) {
					tmpstr = it->getString();
					++it;
					if (tmpstr.length() > 0) {
						elems.push_back(tmpstr);
					}
				}
				Lockingarea.push_back(elems);
			}
		}
		retval.set(SpreadsheetFuncsBase::FPaloSetdata(arg[2].getConnection(), arg[3].getString(), arg[4].getString(), arg[5].getStringArray(), arg[0].getCellValue(), opts.splashingAllowed() ? arg[1].getSplashMode() : jedox::palo::MODE_SPLASH_NONE, Lockingarea));
	}
}

void SpreadsheetFuncs::FPaloSetdataBulk(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() == 6 || arg.length() == 5) {
		if (arg[3].empty(false)) {
			retval.setEmpty();
		} else {
			retval.set(SpreadsheetFuncsBase::FPaloSetdataBulk(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getStringArrayArray(), arg[4].getCellValueArray(), opts.splashingAllowed() ? (arg.length() == 6 ? arg[5].getSplashMode() : jedox::palo::MODE_SPLASH_DEFAULT ) : jedox::palo::MODE_SPLASH_NONE ));
		}
	} else {
		throw WrongParamCountException( CurrentSourceLocation );
	}
}

void SpreadsheetFuncs::FPaloGetdataA(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(4);
	if (arg[3].empty(false)) {
		retval.setEmpty();
	} else {
		StringArray *properties;
		StringArray defprops;
		CellValueWithProperties val = SpreadsheetFuncsBase::FPaloGetdata(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getStringArray(), properties);
		if (!properties) {
			properties = &defprops;
		}
		retval.set(val, *properties);
	}
}

void SpreadsheetFuncs::FPaloSetdataAIf(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(3);
	if (arg.length() < 7) {
		throw WrongParamCountException(CurrentSourceLocation);
	}

	if (!arg[0].getBool()) {
		retval.set(false);
	} else {
		if (arg[4].empty(false)) {
			retval.setEmpty();
		} else {
			StringArrayArray LockingArea;
			retval.set(SpreadsheetFuncsBase::FPaloSetdata(arg[3].getConnection(), arg[4].getString(), arg[5].getString(), arg[6].getStringArray(), arg[1].getCellValue(), opts.splashingAllowed() ? arg[2].getSplashMode() : jedox::palo::MODE_SPLASH_NONE, LockingArea));
		}
	}
}

void SpreadsheetFuncs::FPaloGetdataAC(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	FPaloGetdataACTIntern(retval, opts, arg, false, false);
}

void SpreadsheetFuncs::FPaloGetdataAV(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(4);

	// assert non-empty!
	GetdataHelper::DataVCoordinates a = GetdataHelper::GetdataVParseCoordinates(arg[3]);

	unique_ptr < CellValueArray > _vals(new CellValueArray());
	*_vals = SpreadsheetFuncsBase::FPaloGetdataV(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), a.coords);
	CellValueArray& vals = *_vals;

	size_t num_coordinates = a.coords.size();
	size_t num_rows = 1, num_cols = 1, num_results = 1;
	for (size_t i = 0; i < num_coordinates; i++) {
		size_t t = a.coords[i].size();

		if (a.info[i].is_vertical) {
			num_rows *= t;
		} else {
			num_cols *= t;
		}

		num_results *= t;
	}

	/* sort idx_map preserving order of elements with equal pad_count as well as v/h-order */
	for (size_t i = 0; i < num_coordinates - 1; i++) {
		for (size_t j = i + 1; j < num_coordinates; j++) {
			if (a.info[a.idx_map[i].cube_idx].is_vertical == a.info[a.idx_map[j].cube_idx].is_vertical && a.idx_map[i].pad_count > a.idx_map[j].pad_count) {
				/* swap */
				GetdataHelper::IndexMapEntry t = a.idx_map[i];
				a.idx_map[i] = a.idx_map[j];
				a.idx_map[j] = t;
			}
		}
	}

	vector<size_t> len_product_to_left_dest_v(num_coordinates), len_product_to_left_dest_h(num_coordinates);
	len_product_to_left_dest_v[0] = 1;
	len_product_to_left_dest_h[0] = 1;
	/* loop over dest dimensions */
	for (size_t i = 1; i < num_coordinates; i++) {
		if (a.info[a.idx_map[i - 1].cube_idx].is_vertical) {
			len_product_to_left_dest_v[i] = len_product_to_left_dest_v[i - 1] * a.coords[a.idx_map[i - 1].cube_idx].size();
			len_product_to_left_dest_h[i] = len_product_to_left_dest_h[i - 1];
		} else {
			len_product_to_left_dest_h[i] = len_product_to_left_dest_h[i - 1] * a.coords[a.idx_map[i - 1].cube_idx].size();
			len_product_to_left_dest_v[i] = len_product_to_left_dest_v[i - 1];
		}
	}

	vector < size_t > len_product_to_left_cube(num_coordinates);
	len_product_to_left_cube[0] = 1;

	/* loop over cube dimensions */
	for (size_t i = 1; i < num_coordinates; i++) {
		len_product_to_left_cube[i] = len_product_to_left_cube[i - 1] * a.coords[i - 1].size();
	}

	vector<CellValue*> result(num_results);

	size_t svals = vals.size();
	for (size_t i = 0; i < num_rows; i++) {
		for (size_t j = 0; j < num_cols; j++) {
			size_t cur_row = i;
			size_t cur_col = j;
			size_t cur_pos = 0;

			/* loop dest dimensions */
			for (size_t k = num_coordinates; k > 0; k--) {
				size_t cur_coord;

				if (a.info[a.idx_map[k - 1].cube_idx].is_vertical) {
					cur_coord = cur_row / len_product_to_left_dest_v[k - 1];
					cur_row = cur_row % len_product_to_left_dest_v[k - 1];
				} else {
					cur_coord = cur_col / len_product_to_left_dest_h[k - 1];
					cur_col = cur_col % len_product_to_left_dest_h[k - 1];
				}

				cur_pos += cur_coord * len_product_to_left_cube[a.idx_map[k - 1].cube_idx];
			}
			cur_row = i;
			cur_col = j;

			size_t insert_pos = cur_row * num_cols + cur_col;

			if (cur_pos < svals) {
				result[insert_pos] = &vals[cur_pos];
			}
		}
	}

	/* pad with "" */
	size_t real_num_rows = num_rows + a.tail_padding_v;
	size_t real_num_cols = num_cols + a.tail_padding_h;

	/* build result */
	GenericArrayBuilder dest = retval.setMatrix(real_num_rows, real_num_cols);
	size_t col, tmpind;

	bool error_processed = false;
	CellValue* currval;
	for (size_t row = 0; row < real_num_rows; row++) {
		col = 0;
		for (; row < num_rows && col < num_cols; col++) {
			tmpind = row * num_cols + col;
			if ((tmpind < svals) && ((currval = result[tmpind]) != NULL)) {
				currval = result[tmpind];
				dest.append(dest.createGenericCell()->set(*currval, !error_processed));
				error_processed = error_processed || ((currval->type == CellValue::ERR) && currval->val.err.have_desc);
			} else {
				dest.append(dest.createGenericCell()->setEmpty());
			}
		}
		for (; col < real_num_cols; col++) {
			dest.append(dest.createGenericCell()->setEmpty());
		}
	}

	/* free mem */
	_vals.reset();
}

void SpreadsheetFuncs::FPaloGetdataAT(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	FPaloGetdataACTIntern(retval, opts, arg, false, true);
}

void SpreadsheetFuncs::FPaloGetdataATC(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(4);
	if (arg[3].empty(false)) {
		retval.setEmpty();
	} else {
		GetdataHelper::_GetdataT(retval, _FPaloGetdataC(arg[0], arg[1], arg[2], arg[3]));
	}
}

void SpreadsheetFuncs::FPaloGetdataAggregation(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg, AggregationType aggregationtype)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(5);
	retval.set(SpreadsheetFuncsBase::FPaloGetdataAggregation(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getStringArray(), aggregationtype, arg[4].getIntArray()), PropertyNames::PropNames);
}

void SpreadsheetFuncs::FPaloGetdataAggregationSum(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	FPaloGetdataAggregation(retval, opts, arg, AggregationSum);
}

void SpreadsheetFuncs::FPaloGetdataAggregationAvg(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	FPaloGetdataAggregation(retval, opts, arg, AggregationAvg);
}

void SpreadsheetFuncs::FPaloGetdataAggregationCount(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	FPaloGetdataAggregation(retval, opts, arg, AggregationCount);
}

void SpreadsheetFuncs::FPaloGetdataAggregationMax(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	FPaloGetdataAggregation(retval, opts, arg, AggregationMax);
}

void SpreadsheetFuncs::FPaloGetdataAggregationMin(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	FPaloGetdataAggregation(retval, opts, arg, AggregationMin);
}

void SpreadsheetFuncs::FPaloSetdata(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(2);
	if (arg.length() < 6) {
		throw WrongParamCountException(CurrentSourceLocation);
	}
	arg.collapseToArray(5);
	FPaloSetdataA(retval, opts, arg);
}

void SpreadsheetFuncs::FPaloSetdataIf(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(3);
	if (arg.length() < 7) {
		throw WrongParamCountException(CurrentSourceLocation);
	}

	if (!arg[0].getBool()) {
		retval.set(false);
	} else {
		arg.collapseToArray(6);
		if (arg[4].empty(false)) {
			retval.setEmpty();
		} else {
			StringArrayArray LockingArea;
			retval.set(SpreadsheetFuncsBase::FPaloSetdata(arg[3].getConnection(), arg[4].getString(), arg[5].getString(), arg[6].getStringArray(), arg[1].getCellValue(), opts.splashingAllowed() ? arg[2].getSplashMode() : jedox::palo::MODE_SPLASH_NONE, LockingArea));
		}
	}
}

void SpreadsheetFuncs::FPaloGetdata(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() < 4) {
		throw WrongParamCountException(CurrentSourceLocation);
	}
	arg.collapseToArray(3);
	FPaloGetdataA(retval, opts, arg);
}

void SpreadsheetFuncs::FPaloGetdataC(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	FPaloGetdataACTIntern(retval, opts, arg, true, false);
}

void SpreadsheetFuncs::FPaloGetdataT(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	FPaloGetdataACTIntern(retval, opts, arg, true, true);
}

void SpreadsheetFuncs::FPaloGetdataTC(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() < 4)
		throw WrongParamCountException(CurrentSourceLocation);
	arg.collapseToArray(3);
	FPaloGetdataATC(retval, opts, arg);
}

void SpreadsheetFuncs::FPaloGetdataV(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() < 4)
		throw WrongParamCountException(CurrentSourceLocation);
	arg.collapseToArray(3);
	FPaloGetdataAV(retval, opts, arg);
}

void SpreadsheetFuncs::FPaloCellCopy(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);

	CellValue cv;
	bool use_rules = false;

	if (arg.length() == 7) {
		cv = CellValue(arg[5].getDouble());
		use_rules = arg[6].getBool();
	} else {
		if (arg.length() == 6) {
			if (arg[5].isBool()) {
				use_rules = arg[5].getBool();
			} else {
				cv = CellValue(arg[5].getDouble());
			}
		} else {
			if (arg.length() != 5) {
				throw WrongParamCountException(CurrentSourceLocation);
			}
		}
	}

	retval.set(SpreadsheetFuncsBase::FPaloCellCopy(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getStringArray(), arg[4].getStringArray(), cv, use_rules));
}

void SpreadsheetFuncs::FPaloGoalSeek(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);

	StringArrayArray area;
	if ((arg.length() > 7) || (arg.length() < 6)) {
		throw WrongParamCountException(CurrentSourceLocation);
	}

	if (arg.length() == 7) {
		area = arg[6].getStringArrayArray();
	}

	retval.set(SpreadsheetFuncsBase::FPaloGoalSeek(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getStringArray(), arg[4].getDouble(), (GoalSeekType)arg[5].getUInt(), area));
}

void SpreadsheetFuncs::FPaloElementCreateBulk(Palo::SpreadsheetFuncs::GenericCell &retval, Palo::SpreadsheetFuncs::GenericContext &opts, Palo::SpreadsheetFuncs::GenericArgumentArray &arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);

	if (arg.length() < 5) {
		throw WrongParamCountException(CurrentSourceLocation);
	}

	if (arg[4].empty(false)) {
		retval.setEmpty();
	} else {
		bool return_empty = false;
		if (arg.length() > 8 && !arg[8].isMissing())
			return_empty = arg[8].getBool();

		try {
			if (arg.length() > 5) {
				SpreadsheetFuncsBase::FPaloElementCreateBulk(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getStringArray(), arg[4].getDimensionElementType(), arg[5].getStringArrayArray(), arg[6].getStringArrayArray());
			} else {
				StringArrayArray dummy;
				SpreadsheetFuncsBase::FPaloElementCreateBulk(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getStringArray(), arg[4].getDimensionElementType(), dummy, dummy);
			}
		} catch (...) {
			if (!return_empty) {
				throw;
			} else {
				retval.setEmpty();
			}
		}
	}
}

void SpreadsheetFuncs::FPaloElementAdd(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() != 8 && arg.length() != 9) {
		throw WrongParamCountException(CurrentSourceLocation);
	}

	if (arg[4].empty(false)) {
		retval.setEmpty();
	} else {
		bool return_empty = false;
		if (arg.length() > 8 && !arg[8].isMissing())
			return_empty = arg[8].getBool();

		try {
			int clear = 0;
			if (arg[7].isNumber()) {
				clear = arg[7].getUInt();
			} else {
				clear = arg[7].getBool();
			}
			retval.set(SpreadsheetFuncsBase::FPaloElementAdd(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getDimensionElementType(), arg[4].getString(), arg[5].getString(), arg[6].getDouble(), clear));
		} catch (...) {
			if (!return_empty) {
				throw;
			} else {
				retval.setEmpty();
			}
		}
	}
}

void SpreadsheetFuncs::FPaloElementDelete(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() != 4 && arg.length() != 5) {
		throw WrongParamCountException(CurrentSourceLocation);
	}
	if (arg[3].empty(false))
		retval.setEmpty();
	else {
		bool return_empty = false;
		if (arg.length() > 4 && !arg[4].isMissing())
			return_empty = arg[4].getBool();
		try {
			retval.set(SpreadsheetFuncsBase::FPaloElementDelete(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString()));
		} catch (...) {
			if (!return_empty) {
				throw;
			} else {
				retval.setEmpty();
			}
		}
	}
}

void SpreadsheetFuncs::FPaloElementDeleteBulk(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() != 4 && arg.length() != 5) {
		throw WrongParamCountException(CurrentSourceLocation);
	}
	if (arg[3].empty(false))
		retval.setEmpty();
	else {
		bool return_empty = false;
		if (arg.length() > 4 && !arg[4].isMissing())
			return_empty = arg[4].getBool();
		try {
			SpreadsheetFuncsBase::FPaloElementDeleteBulk(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getStringArray());
			retval.set(true);
		} catch (...) {
			if (!return_empty) {
				throw;
			} else {
				retval.setEmpty();
			}
		}
	}
}

void SpreadsheetFuncs::FPaloElementMove(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(5);
	if (!arg[3].empty(false)) {
		SpreadsheetFuncsBase::FPaloElementMove(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString(), arg[4].getUInt());
	}
	retval.set(true);
}

void SpreadsheetFuncs::FPaloElementMoveBulk(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(5);
	if (!arg[3].empty(false)) {
		SpreadsheetFuncsBase::FPaloElementMoveBulk(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getStringArray(), arg[4].getIntArray());
	}
	retval.set(true);
}

void SpreadsheetFuncs::FPaloElementRename(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() != 5 && arg.length() != 6) {
		throw WrongParamCountException(CurrentSourceLocation);
	}

	if (arg[3].empty(false) || arg[4].empty(false))
		retval.setEmpty();
	else {
		bool return_empty = false;
		if (arg.length() > 5 && !arg[5].isMissing())
			return_empty = arg[5].getBool();
		try {
			SpreadsheetFuncsBase::FPaloElementRename(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString(), arg[4].getString());
			retval.set(true);
		} catch (...) {
			if (!return_empty) {
				throw;
			} else {
				retval.setEmpty();
			}
		}
	}
}

void SpreadsheetFuncs::FPaloElementUpdate(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() != 6 && arg.length() != 7) {
		throw WrongParamCountException(CurrentSourceLocation);
	}
	if (arg[3].empty(false)) {
		retval.setEmpty();
	} else {
		unsigned int mode = ((arg.length() > 6) && !arg[6].isMissing());

		if (mode) {
			mode = arg[6].getUInt();
		}

		if ((mode == 0) || (mode == 1)) {
			SpreadsheetFuncsBase::FPaloElementUpdate(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString(), arg[4].getDimensionElementType(), arg[5].getConsolidationElementArray(), mode == 1);
		} else {
			if (mode == 2) {
				SpreadsheetFuncsBase::FPaloChildrenDelete(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString(), arg[5].getStringArray());
			} else {
				throw ArgumentException(CurrentSourceLocation, SpreadsheetFuncsErrors::ERROR_INVALID_ARGUMENT_VALUE);
			}
		}
	}
	retval.set(true);
}

void SpreadsheetFuncs::FPaloElementChildcount(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() != 4 && arg.length() != 5) {
		throw WrongParamCountException(CurrentSourceLocation);
	}

	if (arg[3].empty(false))
		retval.setEmpty();
	else {
		bool return_empty = false;
		if (arg.length() > 4 && !arg[4].isMissing())
			return_empty = arg[4].getBool();
		try {
			retval.set(SpreadsheetFuncsBase::FPaloElementChildcount(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString()));
		} catch (...) {
			if (!return_empty) {
				throw;
			} else {
				retval.setEmpty();
			}
		}
	}
}

void SpreadsheetFuncs::FPaloElementChildname(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() != 5 && arg.length() != 6) {
		throw WrongParamCountException(CurrentSourceLocation);
	}

	if (arg[3].empty(false))
		retval.setEmpty();
	else {
		bool return_empty = false;
		if (arg.length() > 5 && !arg[5].isMissing())
			return_empty = arg[5].getBool();

		try {
			retval.set(SpreadsheetFuncsBase::FPaloElementChildname(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString(), arg[4].getUInt()));
		} catch (...) {
			if (!return_empty) {
				throw;
			} else {
				retval.setEmpty();
			}
		}
	}
}

void SpreadsheetFuncs::FPaloElementCount(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() != 3 && arg.length() != 4) {
		throw WrongParamCountException(CurrentSourceLocation);
	}

	bool return_empty = false;
	if (arg.length() > 3 && !arg[3].isMissing())
		return_empty = arg[3].getBool();

	try {
		retval.set(SpreadsheetFuncsBase::FPaloElementCount(arg[0].getConnection(), arg[1].getString(), arg[2].getString()));
	} catch (...) {
		if (!return_empty) {
			throw;
		} else {
			retval.setEmpty();
		}
	}
}

void SpreadsheetFuncs::FPaloElementFirst(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() != 3 && arg.length() != 4) {
		throw WrongParamCountException(CurrentSourceLocation);
	}

	bool return_empty = false;
	if (arg.length() > 3 && !arg[3].isMissing())
		return_empty = arg[3].getBool();

	try {
		retval.set(SpreadsheetFuncsBase::FPaloElementFirst(arg[0].getConnection(), arg[1].getString(), arg[2].getString()));
	} catch (...) {
		if (!return_empty) {
			throw;
		} else {
			retval.setEmpty();
		}
	}
}

void SpreadsheetFuncs::FPaloElementIndex(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() != 4 && arg.length() != 5) {
		throw WrongParamCountException(CurrentSourceLocation);
	}

	if (arg[3].empty(false))
		retval.setEmpty();
	else {
		bool return_empty = false;
		if (arg.length() > 4 && !arg[4].isMissing())
			return_empty = arg[4].getBool();

		try {
			retval.set(SpreadsheetFuncsBase::FPaloElementIndex(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString()));
		} catch (...) {
			if (!return_empty) {
				throw;
			} else {
				retval.setEmpty();
			}
		}
	}
}

void SpreadsheetFuncs::FPaloElementIsChild(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() != 5 && arg.length() != 6) {
		throw WrongParamCountException(CurrentSourceLocation);
	}
	if (arg[3].empty(false) || arg[4].empty(false))
		retval.setEmpty();
	else {
		bool return_empty = false;
		if (arg.length() > 5 && !arg[5].isMissing())
			return_empty = arg[5].getBool();

		try {
			retval.set(SpreadsheetFuncsBase::FPaloElementIsChild(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString(), arg[4].getString()));
		} catch (...) {
			if (!return_empty) {
				throw;
			} else {
				retval.setEmpty();
			}
		}
	}
}

void SpreadsheetFuncs::FPaloElementLevel(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() != 4 && arg.length() != 5) {
		throw WrongParamCountException(CurrentSourceLocation);
	}
	if (arg[3].empty(false))
		retval.setEmpty();
	else {
		bool return_empty = false;
		if (arg.length() > 4 && !arg[4].isMissing())
			return_empty = arg[4].getBool();

		try {
			retval.set(SpreadsheetFuncsBase::FPaloElementLevel(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString()));
		} catch (...) {
			if (!return_empty) {
				throw;
			} else {
				retval.setEmpty();
			}
		}
	}
}

void SpreadsheetFuncs::FPaloElementIndent(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() != 4 && arg.length() != 5) {
		throw WrongParamCountException(CurrentSourceLocation);
	}
	if (arg[3].empty(false))
		retval.setEmpty();
	else {
		bool return_empty = false;
		if (arg.length() > 4 && !arg[4].isMissing())
			return_empty = arg[4].getBool();

		try {
			retval.set(SpreadsheetFuncsBase::FPaloElementIndent(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString()));
		} catch (...) {
			if (!return_empty) {
				throw;
			} else {
				retval.setEmpty();
			}
		}
	}
}

void SpreadsheetFuncs::FPaloElementName(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	size_t arglength = arg.length();
	if (arglength >= 3 && (arg[2].getType() == GenericCell::TArray || arg[2].getType() == GenericCell::TMatrix)) {
		GenericCell::Iterator it = arg[2].getArray();
		if (it.end()) {
			throw WrongParamCountException(CurrentSourceLocation);
		}
		std::string viewHandle = it->getString();
		if ((++it).end()) {
			throw WrongParamCountException(CurrentSourceLocation);
		}
		std::string dimname = it->getString();
		if ((++it).end()) {
			throw WrongParamCountException(CurrentSourceLocation);
		}
		unsigned int elemPos = it->getUInt();
		jedox::palo::AxisElement::MEMBERS selector = jedox::palo::AxisElement::NAME;
		if (!(++it).end()) {
			selector = (jedox::palo::AxisElement::MEMBERS)it->getUInt();
		}
		retval.set(SpreadsheetFuncsBase::FPaloViewAxisGet(arg[0].getConnection(), arg[1].getString(), viewHandle, dimname, elemPos), selector);
	} else {
		if (arglength < 4 || arglength > 8)
			throw WrongParamCountException(CurrentSourceLocation);
		if (arg[3].empty(false)) {
			retval.setEmpty();
		} else {
			int fig = -1;

			if (arglength >= 5 && !arg[4].empty(false)) {
				fig = arg[4].getSInt();
			}

			if (arglength < 7 || (fig < 0) || (fig > 3) || (fig == 1) || (arglength >= 7 && arg[6].empty(false))) {
				if (arg[3].getType() == GenericCell::TDouble || arg[3].getType() == GenericCell::TInt) {
					retval.set(SpreadsheetFuncsBase::FPaloElementName(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getUInt()));
				} else {
					retval.set(SpreadsheetFuncsBase::FPaloElementName(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString()));
				}
			} else {
				retval.set(SpreadsheetFuncsBase::FPaloElementName(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString(), arg[6].getString()));
			}
		}
	}
}

void SpreadsheetFuncs::FPaloElementParentcount(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() != 4 && arg.length() != 5) {
		throw WrongParamCountException(CurrentSourceLocation);
	}

	if (arg[3].empty(false))
		retval.setEmpty();
	else {
		bool return_empty = false;
		if (arg.length() > 4 && !arg[4].isMissing())
			return_empty = arg[4].getBool();

		try {
			retval.set(SpreadsheetFuncsBase::FPaloElementParentcount(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString()));
		} catch (...) {
			if (!return_empty) {
				throw;
			} else {
				retval.setEmpty();
			}
		}
	}
}

void SpreadsheetFuncs::FPaloElementParentname(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() != 5 && arg.length() != 6) {
		throw WrongParamCountException(CurrentSourceLocation);
	}

	if (arg[3].empty(false))
		retval.setEmpty();
	else {
		bool return_empty = false;
		if (arg.length() > 5 && !arg[5].isMissing())
			return_empty = arg[5].getBool();

		try {
			retval.set(SpreadsheetFuncsBase::FPaloElementParentname(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString(), arg[4].getUInt()));
		} catch (...) {
			if (!return_empty) {
				throw;
			} else {
				retval.setEmpty();
			}
		}
	}
}

void SpreadsheetFuncs::FPaloElementPrev(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() != 4 && arg.length() != 5) {
		throw WrongParamCountException(CurrentSourceLocation);
	}
	if (arg[3].empty(false))
		retval.setEmpty();
	else {
		bool return_empty = false;
		if (arg.length() > 4 && !arg[4].isMissing())
			return_empty = arg[4].getBool();

		try {
			retval.set(SpreadsheetFuncsBase::FPaloElementPrev(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString()));
		} catch (...) {
			if (!return_empty) {
				throw;
			} else {
				retval.setEmpty();
			}
		}
	}
}

void SpreadsheetFuncs::FPaloElementListParents(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() != 4 && arg.length() != 5) {
		throw WrongParamCountException(CurrentSourceLocation);
	}
	if (arg[3].empty(false))
		retval.setEmpty();
	else {
		bool return_empty = false;
		if (arg.length() > 4 && !arg[4].isMissing())
			return_empty = arg[4].getBool();

		try {
			retval.set(SpreadsheetFuncsBase::FPaloElementListParents(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString()));
		} catch (...) {
			if (!return_empty) {
				throw;
			} else {
				retval.setEmpty();
			}
		}
	}
}

void SpreadsheetFuncs::FPaloElementListSiblings(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() != 4 && arg.length() != 5) {
		throw WrongParamCountException(CurrentSourceLocation);
	}
	if (arg[3].empty(false))
		retval.setEmpty();
	else {
		bool return_empty = false;
		if (arg.length() > 4 && !arg[4].isMissing())
			return_empty = arg[4].getBool();
		try {
			retval.set(SpreadsheetFuncsBase::FPaloElementListSiblings(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString()));
		} catch (...) {
			if (!return_empty) {
				throw;
			} else {
				retval.setEmpty();
			}
		}
	}
}

void SpreadsheetFuncs::FPaloElementListAncestors(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() != 4 && arg.length() != 5) {
		throw WrongParamCountException(CurrentSourceLocation);
	}
	if (arg[3].empty(false))
		retval.setEmpty();
	else {
		bool return_empty = false;
		if (arg.length() > 4 && !arg[4].isMissing())
			return_empty = arg[4].getBool();

		try {
			retval.set(SpreadsheetFuncsBase::FPaloElementListAncestors(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString()));
		} catch (...) {
			if (!return_empty) {
				throw;
			} else {
				retval.setEmpty();
			}
		}
	}
}

void SpreadsheetFuncs::FPaloElementListDescendants(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() != 4 && arg.length() != 5) {
		throw WrongParamCountException(CurrentSourceLocation);
	}
	if (arg[3].empty(false))
		retval.setEmpty();
	else {
		bool return_empty = false;
		if (arg.length() > 4 && !arg[4].isMissing())
			return_empty = arg[4].getBool();

		try {
			retval.set(SpreadsheetFuncsBase::FPaloElementListDescendants(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString()));
		} catch (...) {
			if (!return_empty) {
				throw;
			} else {
				retval.setEmpty();
			}
		}
	}
}

void SpreadsheetFuncs::FPaloElementNext(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);

	if (arg.length() != 4 && arg.length() != 5)
		throw WrongParamCountException(CurrentSourceLocation);

	if (arg[3].empty(false)) {
		retval.setEmpty();
	} else {
		bool return_empty = false;
		if ((arg.length() > 4) && !arg[4].isMissing())
			return_empty = arg[4].getBool();

		try {
			retval.set(SpreadsheetFuncsBase::FPaloElementNext(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString()));
		} catch (...) {
			if (!return_empty)
				throw;
			else
				retval.setEmpty();
		}
	}
}

void SpreadsheetFuncs::FPaloElementSibling(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() != 5 && arg.length() != 6)
		throw WrongParamCountException(CurrentSourceLocation);
	if (arg[3].empty(false))
		retval.setEmpty();
	else {
		bool return_empty = false;
		if ((arg.length() > 5) && !arg[5].isMissing())
			return_empty = arg[5].getBool();

		try {
			retval.set(SpreadsheetFuncsBase::FPaloElementSibling(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString(), arg[4].getSInt()));
		} catch (...) {
			if (!return_empty)
				throw;
			else
				retval.setEmpty();
		}
	}
}

void SpreadsheetFuncs::FPaloDimensionMaxLevel(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(3);
	retval.set(SpreadsheetFuncsBase::FPaloDimensionMaxLevel(arg[0].getConnection(), arg[1].getString(), arg[2].getString()));
}

void SpreadsheetFuncs::FPaloElementType(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() != 4 && arg.length() != 5)
		throw WrongParamCountException(CurrentSourceLocation);

	if (arg[3].empty(false))
		retval.setEmpty();
	else {
		bool return_empty = false;
		if ((arg.length() > 4) && !arg[4].isMissing())
			return_empty = arg[4].getBool();

		try {
			retval.set(SpreadsheetFuncsBase::FPaloElementType(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString()));
		} catch (...) {
			if (!return_empty)
				throw;
			else
				retval.setEmpty();
		}
	}
}

void SpreadsheetFuncs::FPaloElementWeight(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() != 5 && arg.length() != 6)
		throw WrongParamCountException(CurrentSourceLocation);

	if (arg[3].empty(false) || arg[4].empty(false))
		retval.setEmpty();
	else {
		bool return_empty = false;
		if ((arg.length() > 5) && !arg[5].isMissing())
			return_empty = arg[5].getBool();

		try {
			retval.set(SpreadsheetFuncsBase::FPaloElementWeight(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString(), arg[4].getString()));
		} catch (...) {
			if (!return_empty)
				throw;
			else
				retval.setEmpty();
		}
	}
}

void SpreadsheetFuncs::FPaloEventLockBegin(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.checkArgCount(3);
	// void
	SpreadsheetFuncsBase::FPaloEventLockBegin(arg[0].getConnection(), arg[1].getString(), arg[2].getString());
	retval.set(true);
}

void SpreadsheetFuncs::FPaloEventLockEnd(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.checkArgCount(1);
	SpreadsheetFuncsBase::FPaloEventLockEnd(arg[0].getConnection());
	retval.set(true);
}

void SpreadsheetFuncs::FPaloStartCacheCollect(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	arg.checkArgCount(0);
	SpreadsheetFuncsBase::FPaloStartCacheCollect();
	retval.set(true);
}

void SpreadsheetFuncs::FPaloEndCacheCollect(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	arg.checkArgCount(0);
	SpreadsheetFuncsBase::FPaloEndCacheCollect();
	retval.set(true);
}

void SpreadsheetFuncs::FPaloCubeRuleCreate(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);

	switch (arg.length()) {
	case 4:
		retval.set(SpreadsheetFuncsBase::FPaloCubeRuleCreate(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString()));
		break;

	case 5:
		retval.set(SpreadsheetFuncsBase::FPaloCubeRuleCreate(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString(), arg[4].getString()));
		break;

	case 6:
		retval.set(SpreadsheetFuncsBase::FPaloCubeRuleCreate(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString(), arg[4].getString(), arg[5].getString()));
		break;

	case 7:
		retval.set(SpreadsheetFuncsBase::FPaloCubeRuleCreate(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString(), arg[4].getString(), arg[5].getString(), arg[6].getBool()));
		break;

	case 8:
		retval.set(SpreadsheetFuncsBase::FPaloCubeRuleCreate(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString(), arg[4].getString(), arg[5].getString(), arg[6].getBool(), arg[7].getDouble()));
		break;

	default:
		throw WrongParamCountException(CurrentSourceLocation);
	}
}

void SpreadsheetFuncs::FPaloCubeRuleModify(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);

	switch (arg.length()) {
	case 5:
		retval.set(SpreadsheetFuncsBase::FPaloCubeRuleModify(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getSLong(), arg[4].getString()));
		break;

	case 6:
		retval.set(SpreadsheetFuncsBase::FPaloCubeRuleModify(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getSLong(), arg[4].getString(), arg[5].getString()));
		break;

	case 7:
		retval.set(SpreadsheetFuncsBase::FPaloCubeRuleModify(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getSLong(), arg[4].getString(), arg[5].getString(), arg[6].getString()));
		break;

	case 8:
		retval.set(SpreadsheetFuncsBase::FPaloCubeRuleModify(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getSLong(), arg[4].getString(), arg[5].getString(), arg[6].getString(), arg[7].getBool()));
		break;

	case 9:
		retval.set(SpreadsheetFuncsBase::FPaloCubeRuleModify(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getSLong(), arg[4].getString(), arg[5].getString(), arg[6].getString(), arg[7].getBool(), arg[7].getDouble()));
		break;

	default:
		throw WrongParamCountException(CurrentSourceLocation);
	}
}

void SpreadsheetFuncs::FPaloCubeRuleParse(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(4);
	retval.set(SpreadsheetFuncsBase::FPaloCubeRuleParse(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString()));
}

void SpreadsheetFuncs::FPaloCubeRulesMove(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	switch (arg.length()) {
	case 5:
		retval.set(SpreadsheetFuncsBase::FPaloCubeRulesMove(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getIntArray(), arg[4].getDouble(), 0));
		break;
	case 6:
		retval.set(SpreadsheetFuncsBase::FPaloCubeRulesMove(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getIntArray(), arg[4].getDouble(), arg[5].getDouble()));
		break;
	default:
		throw WrongParamCountException(CurrentSourceLocation);
	}

}

void SpreadsheetFuncs::FPaloCubeRuleDelete(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(4);
	retval.set(SpreadsheetFuncsBase::FPaloCubeRuleDelete(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getSLong()));
}

void SpreadsheetFuncs::FPaloCubeRulesDelete(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	switch (arg.length()) {
	case 3:
		retval.set(SpreadsheetFuncsBase::FPaloCubeRulesDelete(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), IntArray()));
		break;
	case 4:
		retval.set(SpreadsheetFuncsBase::FPaloCubeRulesDelete(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getIntArray()));
		break;
	default:
		throw WrongParamCountException(CurrentSourceLocation);
	}
}

void SpreadsheetFuncs::FPaloCubeRules(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(3);
	retval.set(SpreadsheetFuncsBase::FPaloCubeRules(arg[0].getConnection(), arg[1].getString(), arg[2].getString()));
}

void SpreadsheetFuncs::FPaloCubeConvert(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(4);
	SpreadsheetFuncsBase::FPaloCubeConvert(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), (arg[3].getBool()) ? CubeInfo::GPU : CubeInfo::NORMAL );
	retval.set(true);
}

void SpreadsheetFuncs::FPaloCellDrillTrough(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(4);
	retval.set(SpreadsheetFuncsBase::FPaloCellDrillTrough(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getStringArray()));
}

void SpreadsheetFuncs::FPaloSVSRestart(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	size_t arg_len = arg.length();
	int mode = 0;
	if (arg_len > 2) {
		throw WrongParamCountException(CurrentSourceLocation);
	}
	if (arg_len == 2) {
		mode = arg[1].getSInt();
	}
	SpreadsheetFuncsBase::FPaloSVSRestart(arg[0].getConnection(), mode);
	retval.set(true);
}

void SpreadsheetFuncs::FPaloSVSInfo(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(1);
	retval.set(SpreadsheetFuncsBase::FPaloSVSInfo(arg[0].getConnection()));
}

void SpreadsheetFuncs::FPaloActivateLicense(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(3);
	SpreadsheetFuncsBase::FPaloActivateLicense(arg[0].getConnection(), arg[1].getString(), arg[2].getString());
	retval.set(true);
}

void SpreadsheetFuncs::FPaloSetClientDescription(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.checkArgCount(1);
	SpreadsheetFuncsBase::FPaloSetClientDescription(arg[0].getString());
}

void SpreadsheetFuncs::FPaloGetdataExport(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);

	GetdataExportResult ger;

	unsigned short use_rules = 0;
	bool use_rules_specified = false;
	size_t arg_len = arg.length();

	if (arg[arg_len - 1].getType() != GenericCell::TArray) {
		use_rules = arg[arg_len - 1].getUInt();
		use_rules_specified = true;
	}
	if (arg_len == 13 || (arg_len == 14 && use_rules_specified))
		ger = SpreadsheetFuncsBase::FPaloGetdataExport(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getBool(), arg[4].getBool(), arg[5].getCellValue(), arg[6].getCellValue(), arg[7].getString(), arg[8].getString(), arg[9].getString(), arg[10].getUInt(), arg[11].getStringArray(), arg[12].getElementListArray(), use_rules);
	else if (arg.length() == 12 || (arg_len == 13 && use_rules_specified))
		ger = SpreadsheetFuncsBase::FPaloGetdataExport(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getBool(), arg[4].getBool(), arg[5].getCellValue(), arg[6].getCellValue(), arg[7].getString(), arg[8].getString(), arg[9].getString(), arg[10].getUInt(), arg[11].getStringArray(), use_rules);
	else if (arg.length() == 9 || (arg_len == 10 && use_rules_specified))
		ger = SpreadsheetFuncsBase::FPaloGetdataExport(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getBool(), arg[4].getBool(), arg[5].getString(), arg[6].getUInt(), arg[7].getStringArray(), arg[8].getElementListArray(), use_rules);
	else if (arg.length() == 8 || (arg_len == 9 && use_rules_specified))
		ger = SpreadsheetFuncsBase::FPaloGetdataExport(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getBool(), arg[4].getBool(), arg[5].getString(), arg[6].getUInt(), arg[7].getStringArray(), use_rules);
	else
		throw WrongParamCountException(CurrentSourceLocation);

	GetdataHelper::_returnGetdataExportResult(retval, ger);
}

void SpreadsheetFuncs::FPaloParseSubsetParams(size_t start, GenericArgumentArray& arg, std::vector<jedox::palo::BasicFilterSettings> &basics, jedox::palo::TextFilterSettings &text, jedox::palo::SortingFilterSettings &sorting, jedox::palo::AliasFilterSettings &alias, jedox::palo::FieldFilterSettings &field, std::vector<jedox::palo::StructuralFilterSettings> &structurals, std::vector<jedox::palo::DataFilterSettings> &datas)
{
	std::string dummy;

	if (arg.length() > start) {
		if ((alias.active = !arg[start].isMissing())) {
			StringArray as = arg[4].getStringArray();

			if (as.size() == 1 || as.size() == 2) {
				if (as.size() == 1) {
					alias.flags |= jedox::palo::AliasFilterBase::SEARCH_ONE;
				} else {
					alias.flags |= jedox::palo::AliasFilterBase::SEARCH_TWO;
				}
				as.resize(2);
				alias.attribute1 = as[0];
				alias.attribute2 = as[1];
			} else {
				throw SpreadsheetFuncsException(CurrentSourceLocation, SpreadsheetFuncsErrors::ERROR_INVALID_ARGUMENT_FORMAT);
			}
		}
	}

	for (size_t i = start + 1; i < arg.length(); ++i) {
		if (arg[i].isMissing()) {
			continue;
		}

		GenericCellCellDeserializer d(arg[i]);

		std::string filter_type = d.get(dummy).getString();

		if (filter_type == "Field") {
			field.active = true;

			field.flags = jedox::palo::AliasFilterBase::USE_FILTEREXP;

			// AFILTER
			unsigned int rows = d.get(dummy).getUInt();
			unsigned int cols = d.get(dummy).getUInt();

			StringArray sa;
			sa.reserve(rows * cols);
			for (unsigned int i = 0; i < rows * cols; i++) {
				sa.push_back(d.get(dummy).getString());
			}

			StringArray col;
			for (unsigned int i = 0; i < cols; i++) {
				col.reserve(rows);

				for (unsigned int j = 0; j < rows; j++) {
					col.push_back(sa[j * cols + i]);
				}

				field.advanced.push_back(col);
				col.clear();
			}
		} else if (filter_type == "Structural") {
			jedox::palo::StructuralFilterSettings structural;

			structural.active = true;

			structural.flags = d.get(dummy).getUInt();
			structural.bound = d.get(dummy).getString();
			structural.indent = arg[3].isMissing() ? 1 : arg[3].getSInt();
			if (d.isUnset(dummy)) {
				structural.revolve = false;
				d.get(dummy);
				d.get(dummy);
			} else {
				structural.revolve = true;
				structural.revolve_elem = d.get(dummy).getString();
				structural.revolve_count = d.get(dummy).getSInt();
			}
			if (d.isUnset(dummy)) {
				structural.level = false;
				d.get(dummy);
				d.get(dummy);
			} else {
				structural.level = true;
				structural.level_start = d.get(dummy).getSInt();
				structural.level_end = d.get(dummy).getSInt();
			}
			structurals.push_back(structural);
		} else if (filter_type == "Text") {
			text.flags = d.get(dummy).getUInt();
			text.regexps = d.get(dummy).getStringArray();
			text.active = (text.regexps.size() > 0);
		} else if (filter_type == "Data") {
			jedox::palo::DataFilterSettings data;

			data.active = true;

			/* cube */
			data.cube = d.get(dummy).getString();

			/* coords */
			if (d.isUnset(dummy)) {
				data.coords_set = false;
				d.get(dummy);
			} else {
				data.coords_set = true;

				size_t count = d.get(dummy).getSInt();
				for (size_t i = 0; i < count; ++i) {
					ElementList el = d.get(dummy).getElementList();
					data.coords.push_back(std::pair<bool, StringArray>(el.all(), el.all() ? StringArray() : el.getArray()));
				}
			}

			CellValue par1, par2;

			/* ops */
			if (d.isUnset(dummy)) {
				d.get(dummy);
				d.get(dummy);
			} else {
				data.cmp.op1 = d.get(dummy).getString();
				par1 = d.get(dummy).getCellValue();
			}
			if (d.isUnset(dummy)) {
				d.get(dummy);
				d.get(dummy);
			} else {
				data.cmp.op2 = d.get(dummy).getString();
				par2 = d.get(dummy).getCellValue();
			}

			/* top */
			if (!d.isUnset(dummy)) {
				data.top = d.get(dummy).getUInt();
				data.flags |= jedox::palo::DataFilterBase::TOP;
			} else {
				d.get(dummy);
				data.top = -1;
			}

			/* percentage */
			if (d.isUnset(dummy)) {
				data.upper_percentage_set = false;
				d.get(dummy);
				data.upper_percentage = 0;
			} else {
				data.upper_percentage_set = true;
				data.upper_percentage = d.get(dummy).getDouble();
			}
			if (d.isUnset(dummy)) {
				data.lower_percentage_set = false;
				d.get(dummy);
				data.lower_percentage = 0;
			} else {
				data.lower_percentage_set = true;
				data.lower_percentage = d.get(dummy).getDouble();
			}

			/* rules */
			if (!d.get(dummy).getBool()) {
				data.flags |= jedox::palo::DataFilterBase::NORULES;
			}

			/* flags */
			data.flags |= d.get(dummy).getUInt();

			if (data.flags & jedox::palo::DataFilterBase::DATA_STRING) {
				data.cmp.use_strings = true;
				data.cmp.par1s = par1.val.s;
				data.cmp.par2s = par2.val.s;
			} else {
				data.cmp.use_strings = false;
				data.cmp.par1d = par1.val.d;
				data.cmp.par2d = par2.val.d;
			}

			if (data.lower_percentage_set && data.upper_percentage_set)
				data.flags |= jedox::palo::DataFilterBase::MID_PERCENTAGE;
			else if (data.lower_percentage_set)
				data.flags |= jedox::palo::DataFilterBase::LOWER_PERCENTAGE;
			else if (data.upper_percentage_set)
				data.flags |= jedox::palo::DataFilterBase::UPPER_PERCENTAGE;

			datas.push_back(data);
		} else if (filter_type == "Basic") {
			jedox::palo::BasicFilterSettings basic;
			basic.active = true;

			basic.flags = d.get(dummy).getUInt();
			if (d.isUnset(dummy)) {
				basic.manual_subset_set = false;
				d.get(dummy);
			} else {
				basic.manual_subset_set = true;
				basic.manual_subset = d.get(dummy).getStringArray();
			}

			if (d.isUnset(dummy)) {
				d.get(dummy);
			} else {
				basic.manual_paths = d.get(dummy).getStringArray();
			}

			if (basic.manual_subset.size() == 0) {
				basic.active = false;
			}
			basics.push_back(basic);
		} else if (filter_type == "Sorting") {
			sorting.active = true;
			sorting.attribute = d.get(dummy).getString();
			bool level_set = false;
			if (d.isUnset(dummy)) {
				sorting.level = 0;
				d.get(dummy);
			} else {
				level_set = true;
				sorting.level = d.get(dummy).getUInt();
			}

			sorting.limit_count = d.get(dummy).getUInt();
			sorting.limit_start = d.get(dummy).getUInt();

			sorting.flags = d.get(dummy).getUInt();
			sorting.indent = arg[3].isMissing() ? 1 : arg[3].getSInt();
			if (level_set) {
				sorting.flags |= jedox::palo::SortingFilterBase::SORT_ONE_LEVEL;
			}
		}
	}
}

SubsetResults SpreadsheetFuncs::FPaloSubset(GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);

	if (arg.length() < 3) {
		throw WrongParamCountException(CurrentSourceLocation);
	}

	jedox::palo::AliasFilterSettings alias;
	jedox::palo::FieldFilterSettings field;
	std::vector<jedox::palo::StructuralFilterSettings> structural;
	jedox::palo::TextFilterSettings text;
	std::vector<jedox::palo::DataFilterSettings> data;
	std::vector<jedox::palo::BasicFilterSettings> basic;
	jedox::palo::SortingFilterSettings sorting;

	FPaloParseSubsetParams(4, arg, basic, text, sorting, alias, field, structural, data);
	return SpreadsheetFuncsBase::FPaloSubset(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].isMissing() ? 1 : arg[3].getSInt(),
		basic.empty() ? jedox::palo::BasicFilterSettings() : basic[0],
		text,
		sorting,
		alias,
		field,
		structural.empty() ? jedox::palo::StructuralFilterSettings() : structural[0],
		data.empty() ? jedox::palo::DataFilterSettings() : data[0]
	);
}

void SpreadsheetFuncs::FPaloSubset(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	retval.set(FPaloSubset(opts, arg));
}

void SpreadsheetFuncs::FPaloSubsetSize(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	retval.set(FPaloSubset(opts, arg).size());
}

void SpreadsheetFuncs::FPaloSubsetBasicFilter(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	if ((arg.length() > 2) || (arg.length() == 0)) {
		throw WrongParamCountException(CurrentSourceLocation);
	}

	int flags = 0;

	if ((arg.length() >= 1) && !arg[1].empty()) {
		switch (arg[1].getUInt()) {
		case 0:
			flags = jedox::palo::PickListBase::INSERT_FRONT;
			break;
		case 1:
			flags = jedox::palo::PickListBase::INSERT_BACK;
			break;
		case 2:
			flags = jedox::palo::PickListBase::MERGE;
			break;
		case 3:
			flags = jedox::palo::PickListBase::SUB;
			break;
		default:
			throw SpreadsheetFuncsException(CurrentSourceLocation, SpreadsheetFuncsErrors::ERROR_INVALID_ARGUMENT_VALUE);
		}
	}

	GenericCellCellSerializer s(retval);

	s.put("filter_type", s.createGenericCell()->set("Basic"));

	s.put("flags", s.createGenericCell()->set(flags));

	if (!arg[0].isMissing()) {
		if (arg[0].getType() == GenericCell::TMatrix) {
			size_t rows, cols;
			GenericCell::Iterator it = arg[0].getMatrix(rows, cols);
			if (!it.end()) {
				s.put("manual_subset", s.createGenericCell()->set(it->getStringArray(true)));
			} else {
				s.putUnset("manual_subset");
			}
			++it;
			if (!it.end()) {
				s.put("manual_paths", s.createGenericCell()->set(it->getStringArray(true)));
			} else {
				s.putUnset("manual_paths");
			}
		} else {
			s.put("manual_subset", s.createGenericCell()->set(arg[0].getStringArray(true)));
			s.putUnset("manual_paths");
		}
	} else {
		s.putUnset("manual_subset");
		s.putUnset("manual_paths");
	}
}

void SpreadsheetFuncs::FPaloSubsetTextFilter(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	if ((arg.length() < 1) && (arg.length() > 2)) {
		throw WrongParamCountException(CurrentSourceLocation);
	}

	int flags = 0;
	if ((arg.length() > 1) && !arg[1].isMissing()) {

		if (arg[1].getType() != GenericCell::TBool) {
			throw ArgumentException(CurrentSourceLocation, SpreadsheetFuncsErrors::ERROR_INVALID_ARGUMENT_TYPE);
		}

		if (arg[1].getBool()) {
			flags |= jedox::palo::TextFilterBase::EXTENDED;
		}
	}

	GenericCellCellSerializer s(retval);

	s.put("filter_type", s.createGenericCell()->set("Text"));

	s.put("flags", s.createGenericCell()->set(flags));

	s.put("regexps", s.createGenericCell()->set(arg[0].getStringArray()));
}

void SpreadsheetFuncs::FPaloSubsetAliasFilter(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	GenericCellCellSerializer s(retval);

	s.put("filter_type", s.createGenericCell()->set("Field"));

	arg.checkArgCount(1);

	size_t rows, cols;

	GenericCell::Iterator i = arg[0].getMatrix(rows, cols);

	s.put("rows", s.createGenericCell()->set(rows));
	s.put("cols", s.createGenericCell()->set(cols));

	while (!i.end()) {
		s.put("data", s.createGenericCell()->set(i->getString()));
		++i;
	}
}

void SpreadsheetFuncs::FPaloSubsetSortingFilter(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	if (arg.length() > 10) {
		throw WrongParamCountException(CurrentSourceLocation);
	}

	int flags = 0;

	GenericCellCellSerializer s(retval);

	s.put("filter_type", s.createGenericCell()->set("Sorting"));

	/* whole */
	if ((arg.length() > 0) && !arg[0].isMissing()) {
		switch (arg[0].getUInt()) {
		case 0:
			break;
		case 1:
			flags |= jedox::palo::SortingFilterBase::WHOLE;
			break;
		case 2:
			flags |= jedox::palo::SortingFilterBase::WHOLE;
			flags |= jedox::palo::SortingFilterBase::NO_CHILDREN;
			break;
		default:
			throw SpreadsheetFuncsException(CurrentSourceLocation, SpreadsheetFuncsErrors::ERROR_INVALID_ARGUMENT_VALUE);
		}
	}

	/* criteria */
	if ((arg.length() > 1) && !arg[1].isMissing()) {
		switch (arg[1].getUInt()) {
		case 0:
			flags |= jedox::palo::SortingFilterBase::POSITION;
			break;
		case 1:
			flags |= jedox::palo::SortingFilterBase::NUMERIC;
			break;
		case 2:
			flags |= jedox::palo::SortingFilterBase::TEXT;
			break;
		case 3:
			flags |= jedox::palo::SortingFilterBase::USE_ALIAS;
			break;
		case 4:
			flags |= jedox::palo::SortingFilterBase::CONSOLIDATION_ORDER;
			break;
		default:
			throw SpreadsheetFuncsException(CurrentSourceLocation, SpreadsheetFuncsErrors::ERROR_INVALID_ARGUMENT_VALUE);
		}
	}

	/* attribute */
	if ((arg.length() > 2) && !arg[2].isMissing()) {
		s.put("attribute", s.createGenericCell()->set(arg[2].getString()));
	} else {
		s.put("attribute", s.createGenericCell()->set(""));
	}

	/* type_limitation */
	if ((arg.length() > 3) && !arg[3].isMissing()) {
		switch (arg[3].getUInt()) {
		case 0:
			break;
		case 1:
			flags |= jedox::palo::SortingFilterBase::LEAVES_ONLY;
			break;
		case 2:
			flags |= jedox::palo::SortingFilterBase::CONSOLIDATED_ONLY;
			break;
		default:
			throw SpreadsheetFuncsException(CurrentSourceLocation, SpreadsheetFuncsErrors::ERROR_INVALID_ARGUMENT_VALUE);
		}
	}

	/* sorting level*/
	if ((arg.length() > 4) && !arg[4].isMissing()) {
		s.put("level", s.createGenericCell()->set(arg[4].getUInt()));
	} else {
		s.putUnset("level");
		//s.put( "level", s.createGenericCell()->set( 0 ) );
	}

	/* reverse */
	if ((arg.length() > 5) && !arg[5].isMissing()) {
		switch (arg[5].getUInt()) {
		case 0:
			break;
		case 1:
			flags |= jedox::palo::SortingFilterBase::REVERSE_ORDER;
			break;
		case 2:
			flags |= jedox::palo::SortingFilterBase::REVERSE_TOTAL;
			break;
		case 3:
			flags |= jedox::palo::SortingFilterBase::REVERSE_TOTAL;
			flags |= jedox::palo::SortingFilterBase::REVERSE_ORDER;
			break;
			//the following cases are for future use, they are supported but not accessible from everywhere
		case 4:
			flags |= jedox::palo::SortingFilterBase::REVERSE_TOTAL_EX;
			break;
		case 5:
			flags |= jedox::palo::SortingFilterBase::REVERSE_TOTAL;
			flags |= jedox::palo::SortingFilterBase::REVERSE_TOTAL_EX;
			break;
		case 6:
			flags |= jedox::palo::SortingFilterBase::REVERSE_TOTAL;
			flags |= jedox::palo::SortingFilterBase::REVERSE_ORDER;
			flags |= jedox::palo::SortingFilterBase::REVERSE_TOTAL_EX;
			break;
		default:
			throw SpreadsheetFuncsException(CurrentSourceLocation, SpreadsheetFuncsErrors::ERROR_INVALID_ARGUMENT_VALUE);
		}
	}

	/* show duplicates */
	if ((arg.length() > 6) && !arg[6].isMissing()) {
		switch (arg[6].getUInt()) {
		case 0:
			break;
		case 1:
			flags |= jedox::palo::SortingFilterBase::SHOW_DUPLICATES;
			break;
		}
	}

	/* Limit */
	unsigned int limit_count = (unsigned int)LONG_MAX, limit_start = 0;
	if ((arg.length() > 7) && !arg[7].isMissing()) {
		flags |= jedox::palo::SortingFilterBase::LIMIT;
		limit_count = arg[7].getUInt();

		if ((arg.length() > 8) && !arg[8].isMissing()) {
			limit_start = arg[8].getUInt();
		}
	}
	if ((arg.length() > 9) && !arg[9].isMissing()) {
		if (arg[9].getBool()) {
			flags |= jedox::palo::SortingFilterBase::PATH;
		}
	}

	s.put("limit_count", s.createGenericCell()->set(limit_count));
	s.put("limit_start", s.createGenericCell()->set(limit_start));

	s.put("flags", s.createGenericCell()->set(flags));
}

void SpreadsheetFuncs::FPaloSubsetStructuralFilter(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	if (arg.length() > 9) {
		throw WrongParamCountException(CurrentSourceLocation);
	}

	std::string element;
	if (!arg[0].isMissing()) {
		element = arg[0].getString();
	}
	bool above = false;
	if (!arg[1].isMissing()) {
		above = arg[1].getBool();
	}
	bool exclusive = false;
	if (!arg[2].isMissing()) {
		exclusive = arg[2].getBool();
	}

	int flags = 0;

	if (!element.empty()) {
		if (above) {
			if (exclusive) {
				flags |= jedox::palo::StructuralFilterBase::ABOVE_EXCLUSIVE;
			} else {
				flags |= jedox::palo::StructuralFilterBase::ABOVE_INCLUSIVE;
			}
		} else {
			if (exclusive) {
				flags |= jedox::palo::StructuralFilterBase::BELOW_EXCLUSIVE;
			} else {
				flags |= jedox::palo::StructuralFilterBase::BELOW_INCLUSIVE;
			}
		}
	}

	if (arg.length() > 3) {
		if (!arg[3].isMissing()) {
			switch (arg[3].getUInt()) {
			case 0:
				break;
			case 1:
				flags |= jedox::palo::StructuralFilterBase::HIDE_LEAVES;
				break;
			case 2:
				flags |= jedox::palo::StructuralFilterBase::HIDE_CONSOLIDATED;
				break;
			default:
				throw SpreadsheetFuncsException(CurrentSourceLocation, SpreadsheetFuncsErrors::ERROR_INVALID_ARGUMENT_VALUE);
			}
		}
	}

	if ((arg.length() >= 7) && !arg[6].isMissing()) {
		/* revolve_add */
		switch (arg[6].getUInt()) {
		case 0:
			/*! \todo REVOLVING? */
			break;
		case 1:
			flags |= jedox::palo::StructuralFilterBase::REVOLVE_ADD_BELOW;
			break;
		case 2:
			flags |= jedox::palo::StructuralFilterBase::REVOLVE_ADD_ABOVE;
			break;
		default:
			throw SpreadsheetFuncsException(CurrentSourceLocation, SpreadsheetFuncsErrors::ERROR_INVALID_ARGUMENT_VALUE);
		}
	}

	GenericCellCellSerializer s(retval);

	s.put("filter_type", s.createGenericCell()->set("Structural"));

	s.put("flags", s.createGenericCell()->set(flags));
	s.put("bound", s.createGenericCell()->set(element));

	if ((arg.length() >= 6) && !arg[5].empty()) {
		s.put("revolve_elem", s.createGenericCell()->set(arg[4].empty() ? "" : arg[4].getString()));
		s.put("revolve_count", s.createGenericCell()->set(arg[5].getSInt()));
	} else {
		s.putUnset("revolve_elem");
		s.putUnset("revolve_count");
	}

	if ((arg.length() >= 9) && !arg[7].empty()) {
		s.put("level_start", s.createGenericCell()->set(arg[7].getSInt()));
		s.put("level_end", s.createGenericCell()->set(arg[8].getSInt()));
	} else {
		s.putUnset("level_start");
		s.putUnset("level_end");
	}
}

void SpreadsheetFuncs::FPaloSubsetDataFilter(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	if ((arg.length() < 1) || (arg.length() > 7)) {
		throw WrongParamCountException(CurrentSourceLocation);
	}

	int flags = 0;

	GenericCellCellSerializer s(retval);

	s.put("filter_type", s.createGenericCell()->set("Data"));

	std::string dummy;

	/* subcube */
	if (arg[0].empty()) {
		throw SpreadsheetFuncsException(CurrentSourceLocation, SpreadsheetFuncsErrors::ERROR_INVALID_ARGUMENT_TYPE);
	}
	GenericCellCellDeserializer d(arg[0]);
	s.put("cube", s.createGenericCell()->set(d.get(dummy).getString()));
	size_t count = d.get(dummy).getSInt();
	if (count > 0) {
		s.put("coordscount", s.createGenericCell()->set(count));
		for (size_t i = 0; i < count; ++i) {
			s.put("coordsx", s.createGenericCell()->set(d.get(dummy).getElementList()));
		}
	} else {
		s.putUnset("coords");
	}

	/* operators */
	if (arg[1].empty()) {
		s.putUnset("op1");
		s.putUnset("par1");
		s.putUnset("op2");
		s.putUnset("par2");
	} else {
		GenericCell::Iterator i = arg[1].getArray();
		if ((i.minRemaining() == 2) || (i.minRemaining() == 4)) {
			s.put("op1", s.createGenericCell()->set(i->getString()));
			++i;
			s.put("par1", s.createGenericCell()->set(i->getCellValue()));
			++i;
			if (i.minRemaining() == 2) {
				s.put("op2", s.createGenericCell()->set(i->getString()));
				++i;
				s.put("par2", s.createGenericCell()->set(i->getCellValue()));
			} else {
				s.putUnset("op2");
				s.putUnset("par2");
			}
		} else {
			throw SpreadsheetFuncsException(CurrentSourceLocation, SpreadsheetFuncsErrors::ERROR_INVALID_ARGUMENT_FORMAT);
		}
	}

	/* top */
	if ((arg.length() >= 3) && !arg[2].isMissing()) {
		s.put("top", s.createGenericCell()->set(arg[2].getUInt()));
	} else {
		s.putUnset("top");
	}

	/* percentage */
	if ((arg.length() >= 4) && !arg[3].isMissing()) {
		s.put("percentage1", s.createGenericCell()->set(arg[3].getDouble()));
	} else {
		s.putUnset("percentage1");
	}

	if ((arg.length() >= 5) && !arg[4].isMissing()) {
		s.put("percentage2", s.createGenericCell()->set(arg[4].getDouble()));
	} else {
		s.putUnset("percentage2");
	}

	/* cell_operator */
	if ((arg.length() >= 6) && !arg[5].isMissing()) {
		switch (arg[5].getUInt()) {
		case 0:
			flags |= jedox::palo::DataFilterBase::DATA_SUM;
			break;
		case 1:
			flags |= jedox::palo::DataFilterBase::DATA_ALL;
			break;
		case 2:
			flags |= jedox::palo::DataFilterBase::DATA_AVERAGE;
			break;
		case 3:
			flags |= jedox::palo::DataFilterBase::DATA_MAX;
			break;
		case 4:
			flags |= jedox::palo::DataFilterBase::DATA_ANY;
			break;
		case 5:
			flags |= jedox::palo::DataFilterBase::DATA_MIN;
			break;
		case 6:
			flags |= jedox::palo::DataFilterBase::DATA_STRING;
			break;
		default:
			throw SpreadsheetFuncsException(CurrentSourceLocation, SpreadsheetFuncsErrors::ERROR_INVALID_ARGUMENT_VALUE);
		}
	} else {
		flags |= jedox::palo::DataFilterBase::DATA_SUM;
	}

	/* rules */
	if ((arg.length() >= 7) && !arg[6].isMissing()) {
		s.put("rules", s.createGenericCell()->set(!arg[6].getBool()));
	} else {
		s.put("rules", s.createGenericCell()->set(!false));
	}

	s.put("flags", s.createGenericCell()->set(flags));
}

void SpreadsheetFuncs::FPaloSubcube(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	GenericCellCellSerializer s(retval);

	if (arg.length() == 0) {
		throw WrongParamCountException(CurrentSourceLocation);
	}

	s.put("cube", s.createGenericCell()->set(arg[0].getString()));

	s.put("coordscount", s.createGenericCell()->set(arg.length() - 1));

	for (size_t i = 1; i < arg.length(); ++i) {
		if (!arg[i].isMissing()) {
			s.put("coordsx", s.createGenericCell()->set(arg[i].getElementList()));
		} else {
			s.putUnset("coordsx");
		}
	}
}

void SpreadsheetFuncs::FPaloRemoveConnection(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO

	if (arg.length() < 1 || arg.length() > 2) {
		throw WrongParamCountException(CurrentSourceLocation);
	}

	if (arg.length() == 1) {
		SpreadsheetFuncsBase::FPaloRemoveConnection(arg[0].getString());
	} else {
		SpreadsheetFuncsBase::FPaloRemoveConnection(arg[0].getString(), arg[1].getBool());
	}
	retval.set(true);
}

CellValue SpreadsheetFuncs::_FPaloGetdataC(GenericCell& server, GenericCell& database, GenericCell& cube, GenericCell& path, const CellValue* * const ptr)
{
	StringArray *properties;
	return SpreadsheetFuncsBase::FPaloGetdataC(server.getConnection(), database.getString(), cube.getString(), path.getStringArray(), properties, ptr);
}

void SpreadsheetFuncs::FPaloGetElementId(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(4);
	retval.set(SpreadsheetFuncsBase::FPaloGetElementId(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString()));
}

void SpreadsheetFuncs::FPaloGetElementName(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(4);
	retval.set(SpreadsheetFuncsBase::FPaloGetElementName(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getSLong()));
}

void SpreadsheetFuncs::FPaloGetDimensionId(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(3);
	retval.set(SpreadsheetFuncsBase::FPaloGetDimensionId(arg[0].getConnection(), arg[1].getString(), arg[2].getString()));
}

void SpreadsheetFuncs::FPaloGetDimensionName(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(3);
	retval.set(SpreadsheetFuncsBase::FPaloGetDimensionName(arg[0].getConnection(), arg[1].getString(), arg[2].getSLong()));
}

void SpreadsheetFuncs::FPaloGetCubeId(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(3);
	retval.set(SpreadsheetFuncsBase::FPaloGetCubeId(arg[0].getConnection(), arg[1].getString(), arg[2].getString()));
}

void SpreadsheetFuncs::FPaloGetCubeName(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(3);
	retval.set(SpreadsheetFuncsBase::FPaloGetCubeName(arg[0].getConnection(), arg[1].getString(), arg[2].getSLong()));
}

void SpreadsheetFuncs::FPaloConnectionUser(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(1);
	retval.set(SpreadsheetFuncsBase::FPaloConnectionUser(arg[0].getConnection()));
}

void SpreadsheetFuncs::FPaloGetUserForSID(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() == 2 || (arg.length() == 3 && arg[2].getBool() == false)) {
		retval.set(SpreadsheetFuncsBase::FPaloGetUserForSID(arg[0].getConnection(), arg[1].getString()));
	} else if (arg.length() == 3) {
		retval.set(SpreadsheetFuncsBase::FPaloGetUserInfoForSID(arg[0].getConnection(), arg[1].getString()));
	} else {
		throw WrongParamCountException(CurrentSourceLocation);
	}
}

void SpreadsheetFuncs::FPaloGetGroupsForSID(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(2);
	retval.set(SpreadsheetFuncsBase::FPaloGetGroupsForSID(arg[0].getConnection(), arg[1].getString()));
}

void SpreadsheetFuncs::FPaloGetGroups(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	arg.checkArgCount(1);
	retval.set(SpreadsheetFuncsBase::FPaloGetGroups(arg[0].getConnection()));
}

void SpreadsheetFuncs::FPaloElementAlias(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if ((arg.length() < 5) || (arg.length() > 7)) {
		throw WrongParamCountException(CurrentSourceLocation);
	}

	unsigned long index = 0;

	if (arg.length() > 5 && !arg[5].isMissing()) {
		index = arg[5].getULong();
	}

	retval.set(SpreadsheetFuncsBase::FPaloElementAlias(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getString(), arg[4].getString(), index));
}

void SpreadsheetFuncs::FPaloViewSubsetDefinition(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);

	if (arg.length() < 3) {
		throw WrongParamCountException(CurrentSourceLocation);
	}

	jedox::palo::AliasFilterSettings alias;
	jedox::palo::FieldFilterSettings field;
	std::vector<jedox::palo::StructuralFilterSettings> structural;
	jedox::palo::TextFilterSettings text;
	std::vector<jedox::palo::DataFilterSettings> data;
	std::vector<jedox::palo::BasicFilterSettings> basic;
	jedox::palo::SortingFilterSettings sorting;

	FPaloParseSubsetParams(4, arg, basic, text, sorting, alias, field, structural, data);
	retval.set(SpreadsheetFuncsBase::FPaloViewSubsetDefinition(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].isMissing() ? 1 : arg[3].getSInt(), basic, text, sorting, alias, field, structural, data));
}

void SpreadsheetFuncs::FPaloViewAxisDefinition(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);

	if (arg.length() < 7 || (arg.length() - 3) % 4) {
		throw WrongParamCountException(CurrentSourceLocation);
	}

	jedox::palo::AxisSubsets as;

	for (size_t i = 3; i < arg.length(); i += 4) {
		as.push_back(jedox::palo::AXIS_SUBSET_DEF(arg[i].getString(), arg[i + 1].getString(), arg[i + 2].getString(), arg[i + 3].getBool()));
	}

	retval.set(SpreadsheetFuncsBase::FPaloViewAxisDefinition(arg[0].getConnection(), arg[1].getString(), arg[2].getSInt(), as));
}

void SpreadsheetFuncs::FPaloViewAreaDefinition(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);

	if (arg.length() < 5) {
		throw WrongParamCountException(CurrentSourceLocation);
	}

	StringArray axes;
	for (size_t i = 4; i < arg.length(); ++i) {
		axes.push_back(arg[i].getString());
	}

	retval.set(SpreadsheetFuncsBase::FPaloViewAreaDefinition(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), axes, arg[3].getStringArray(true)));
}

void SpreadsheetFuncs::FPaloViewAxisGet(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);

	if (arg.length() < 6) {
		throw WrongParamCountException(CurrentSourceLocation);
	}

	retval.set(SpreadsheetFuncsBase::FPaloViewAxisGet(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getSInt(), arg[4].getUInt(), arg[5].getUInt()));
}

void SpreadsheetFuncs::FPaloViewAxisGetIndex(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);

	if (arg.length() < 7) {
		throw WrongParamCountException(CurrentSourceLocation);
	}

	SubsetResult sr = SpreadsheetFuncsBase::FPaloViewAxisGet(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getSInt(), arg[4].getUInt(), arg[5].getUInt());

	switch (arg[6].getUInt()) {
	case 0:
		retval.set(sr.element.get_name());
		break;
	case 1:
		retval.set(sr.element.get_alias());
		break;
	case 2:
		retval.set(sr.idx);
		break;
	case 3:
		retval.set(sr.element.path);
		break;
	case 4:
		retval.set(sr.element.m_einf.element);
		break;
	case 5:
		retval.set(sr.element.m_einf.position);
		break;
	case 6:
		retval.set(sr.element.m_einf.level);
		break;
	case 7:
		retval.set(sr.element.m_einf.indent);
		break;
	case 8:
		retval.set(sr.element.m_einf.depth);
		break;
	case 9:
		retval.set(sr.element.m_einf.type);
		break;
	case 10:
		retval.set(sr.element.m_einf.number_parents);
		break;
	case 11:
		retval.set(sr.element.m_einf.number_children);
		break;
	case 12:
		{
			GenericCell::ArrayBuilder par = retval.setArray(sr.element.m_einf.parents.size());
			for (jedox::palo::ELEMENT_LIST::const_iterator it = sr.element.m_einf.parents.begin(); it != sr.element.m_einf.parents.end(); ++it) {
				par.append(par.createGenericCell()->set(*it));
			}
		}
		break;
	case 13:
		{
			GenericCell::ArrayBuilder chi = retval.setArray(sr.element.m_einf.children.size());
			for (jedox::palo::ELEMENT_LIST::const_iterator it = sr.element.m_einf.children.begin(); it != sr.element.m_einf.children.end(); ++it) {
				chi.append(chi.createGenericCell()->set(*it));
			}
		}
		break;
	case 14:
		{
			GenericCell::ArrayBuilder chw = retval.setArray(sr.element.m_einf.weights.size());
			for (jedox::palo::ELEMENT_WEIGHT_LIST::const_iterator it = sr.element.m_einf.weights.begin(); it != sr.element.m_einf.weights.end(); ++it) {
				chw.append(chw.createGenericCell()->set(*it));
			}
		}
		break;
	default:
		throw ArgumentException(CurrentSourceLocation, SpreadsheetFuncsErrors::ERROR_INVALID_OFFSET);
	}
}

void SpreadsheetFuncs::FPaloViewAreaGet(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);

	if (arg.length() < 4) {
		throw WrongParamCountException(CurrentSourceLocation);
	}

	std::vector<std::string> coord;
	for (size_t i = 3; i < arg.length(); ++i) {
		coord.push_back(arg[i].getString());
	}
	StringArray *properties;
	CellValueWithProperties val = SpreadsheetFuncsBase::FPaloViewAreaGet(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), coord, properties);
	retval.set(val, *properties);
}

void SpreadsheetFuncs::FPaloViewAxisGetSize(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);

	if (arg.length() < 4) {
		throw WrongParamCountException(CurrentSourceLocation);
	}
	retval.set(SpreadsheetFuncsBase::FPaloViewAxisGetSize(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getSInt()));
}

void SpreadsheetFuncs::FPaloViewDimension(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	if (arg.length() < 3) {
		throw WrongParamCountException(CurrentSourceLocation);
	}
	std::string arg1 = arg[0].getString();
	std::string arg2 = arg[1].getString();
	unsigned int arg3 = arg[2].getUInt();
	bool is4 = arg.length() > 3;
	unsigned int arg4 = 0;
	if (is4) {
		arg4 = arg[3].getUInt();
	}

	GenericCell::ArrayBuilder a = retval.setArray(is4 ? 4 : 3);
	a.append(a.createGenericCell()->set(arg1));
	a.append(a.createGenericCell()->set(arg2));
	a.append(a.createGenericCell()->set(arg3));
	if (is4) {
		a.append(a.createGenericCell()->set(arg4));
	}
}

void SpreadsheetFuncs::FPaloCubeLock(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() < 3) {
		throw WrongParamCountException(CurrentSourceLocation);
	}
	bool complete = false;
	StringArrayArray coords;
	if (arg.length() == 3 || arg[3].isMissing()) {
		complete = true;
	} else {
		coords = arg[3].getStringArrayArray();
	}
	retval.set(SpreadsheetFuncsBase::FPaloCubeLock(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), coords, complete));
}

void SpreadsheetFuncs::FPaloCubeLocks(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() != 3) {
		throw WrongParamCountException(CurrentSourceLocation);
	}
	retval.set(SpreadsheetFuncsBase::FPaloCubeLocks(arg[0].getConnection(), arg[1].getString(), arg[2].getString()));
}

void SpreadsheetFuncs::FPaloCubeRollback(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() < 4) {
		throw WrongParamCountException(CurrentSourceLocation);
	}
	long steps = -1;
	if (arg.length() > 4) {
		steps = arg[4].getSLong();
	}
	retval.set(SpreadsheetFuncsBase::FPaloCubeRollback(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getUInt(), steps));
}

void SpreadsheetFuncs::FPaloCubeCommit(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (arg.length() != 4) {
		throw WrongParamCountException(CurrentSourceLocation);
	}
	retval.set(SpreadsheetFuncsBase::FPaloCubeCommit(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getUInt()));
}

void SpreadsheetFuncs::FPaloGetdataACTIntern(GenericCell& retval, GenericContext& opts, GenericArgumentArray& arg, bool collapse, bool T)
{
	CLEAR_ERRORINFO
	arg.fixConnection(0);
	if (collapse) {
		if (arg.length() < 4) {
			throw WrongParamCountException(CurrentSourceLocation);
		}
		arg.collapseToArray(3);
	} else {
		arg.checkArgCount(4);
	}
	if (arg[3].empty(false)) {
		retval.setEmpty();
	} else {
		if (T) {
			StringArray *properties;
			GetdataHelper::_GetdataT(retval, SpreadsheetFuncsBase::FPaloGetdata(arg[0].getConnection(), arg[1].getString(), arg[2].getString(), arg[3].getStringArray(), properties));
		} else {
			retval.set(_FPaloGetdataC(arg[0], arg[1], arg[2], arg[3]));
		}
	}
}
