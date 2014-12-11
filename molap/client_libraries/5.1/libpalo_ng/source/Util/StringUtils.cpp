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
 * \author Hendrik Schmieder <hendrik.schmieder@jedox.com>
 * \author Martin Jakl <martin.jakl@qbicon.cz>
 * 
 *
 */

#include <sstream>
#include <libpalo_ng/Util/StringUtils.h>
#include <boost/shared_array.hpp>
#include <unicode/coll.h>
#include <unicode/ucasemap.h>
#include <boost/shared_ptr.hpp>
#include <boost/thread/tss.hpp>

namespace jedox {
namespace util {

struct UTF8ComparerInternal {
	U_NAMESPACE_QUALIFIER Collator *col;
	UCaseMap *cm;
	UTF8ComparerInternal();
	~UTF8ComparerInternal();
};

boost::thread_specific_ptr<UTF8ComparerInternal> utf8impl;

std::string StringUtils::escapeString(const std::string& text)
{
	std::stringstream sb;

	std::string buffer = text;
	size_t begin = 0;
	size_t end = buffer.find("\"");

	sb << "\"";

	while (end != std::string::npos) {
		std::string result = text.substr(begin, end - begin);

		sb << result;
		sb << "\"\"";

		begin = end + 1;
		end = buffer.find("\"", begin);
	}

	sb << text.substr(begin);
	sb << "\"";

	return sb.str();
}

std::string StringUtils::Numeric2String(double value)
{
	std::stringstream helperstr;

	helperstr.setf(std::ios_base::fixed, std::ios_base::floatfield);
	helperstr.precision(PRECISION);

	helperstr << value;

	std::string tmpstr = helperstr.str();

	if (tmpstr.find('.') != std::string::npos) {
		size_t lastindex = tmpstr.length() - 1;

		while (tmpstr[lastindex] == '0') {
			tmpstr = tmpstr.erase(lastindex);
			lastindex = tmpstr.length() - 1;
		}

		if (tmpstr[lastindex] == '.') {
			tmpstr = tmpstr.erase(lastindex);
		}

	}

	return tmpstr;

}

std::string StringUtils::CSVencode(const std::string& val, const char delimeter)
{
	size_t vsize = val.size();
	std::stringstream idstr;

	idstr << delimeter;

	for (size_t i = 0; i < vsize; i++) {
		idstr << val[i];
		if (val[i] == delimeter) {
			idstr << delimeter;
		}
	}

	idstr << delimeter;

	return idstr.str();

}

std::string StringUtils::URLencode(const std::string& val)
{
	size_t vsize = val.size();
	std::stringstream idstr;

	for (size_t i = 0; i < vsize; i++) {

		switch (val[i]) {
		case ' ':
			idstr << "%20";
			break;
		case '#':
			idstr << "%23";
			break;
		case '+':
			idstr << "%2B";
			break;
		case '&':
			idstr << "%26";
			break;
		case '=':
			idstr << "%3D";
			break;
		case '\n':
			idstr << "%0A";
			break;
		case '\r':
			idstr << "%0D";
			break;
		case '%':
			idstr << "%25";
			break;
		default:
			idstr << val[i];
			break;
		}
	}

	return idstr.str();

}

std::string UTF8Comparer::toUpper(const std::string &s)
{
	UTF8ComparerInternal *u8 = utf8impl.get();
	if (!u8) {
		u8 = new UTF8ComparerInternal();
		utf8impl.reset(u8);
	}
	UErrorCode er = U_ZERO_ERROR;
	int ret = (int32_t)s.size();
	boost::shared_array<char> buf(new char[ret]);
	ret = ucasemap_utf8ToUpper(u8->cm, buf.get(), ret, s.c_str(), (int32_t)s.size(), &er);
	if (er == U_BUFFER_OVERFLOW_ERROR) {
		er = U_ZERO_ERROR;
		buf.reset(new char[ret]);
		ret = ucasemap_utf8ToUpper(u8->cm, buf.get(), ret, s.c_str(), (int32_t)s.size(), &er);
	}
	if (U_FAILURE(er)) {
		std::stringstream str;
		str << "ICU returned unexpected error: " << u_errorName(er);
		throw jedox::palo::PaloException(str.str(), "ICU failed.", 0);
	}
	return std::string(buf.get(), ret);
}

std::string UTF8Comparer::toLower(const std::string &s)
{
	UTF8ComparerInternal *u8 = utf8impl.get();
	if (!u8) {
		u8 = new UTF8ComparerInternal();
		utf8impl.reset(u8);
	}
	UErrorCode er = U_ZERO_ERROR;
	int ret = (int32_t)s.size();
	boost::shared_array<char> buf(new char[ret]);
	ret = ucasemap_utf8ToLower(u8->cm, buf.get(), ret, s.c_str(), (int32_t)s.size(), &er);
	if (er == U_BUFFER_OVERFLOW_ERROR) {
		er = U_ZERO_ERROR;
		buf.reset(new char[ret]);
		ret = ucasemap_utf8ToLower(u8->cm, buf.get(), ret, s.c_str(), (int32_t)s.size(), &er);
	}
	if (U_FAILURE(er)) {
		std::stringstream str;
		str << "ICU returned unexpected error: " << u_errorName(er);
		throw jedox::palo::PaloException(str.str(), "ICU failed.", 0);
	}
	return std::string(buf.get(), ret);
}

UTF8ComparerInternal::UTF8ComparerInternal() : col(0), cm(0)
{
	UErrorCode er = U_ZERO_ERROR;
	U_NAMESPACE_QUALIFIER Locale loc(setlocale(LC_COLLATE, 0));
	col = Collator::createInstance(std::string(loc.getName()) == "c" ? U_NAMESPACE_QUALIFIER Locale::getEnglish() : loc, er);
	if (U_FAILURE(er)) {
		std::stringstream str;
		str << "Initialization of ICU failed with error: " << u_errorName(er);
		throw jedox::palo::PaloException(str.str(), "ICU failed.", 0);
	}
	col->setAttribute(UCOL_STRENGTH, UCOL_SECONDARY, er);
	if (U_FAILURE(er)) {
		delete col;
		std::stringstream str;
		str << "Initialization of ICU failed with error: " << u_errorName(er);
		throw jedox::palo::PaloException(str.str(), "ICU failed.", 0);
	}
	cm = ucasemap_open(0, U_FOLD_CASE_DEFAULT, &er);
	if (U_FAILURE(er)) {
		delete col;
		std::stringstream str;
		str << "Initialization of ICU failed with error: " << u_errorName(er);
		throw jedox::palo::PaloException(str.str(), "ICU failed.", 0);
	}
}

UTF8ComparerInternal::~UTF8ComparerInternal()
{
	if (col) {
		delete col;
	}
	if (cm) {
		ucasemap_close(cm);
	}
}

int UTF8Comparer::compare(const std::string &s1, const std::string &s2)
{
	UTF8ComparerInternal *u8 = utf8impl.get();
	if (!u8) {
		u8 = new UTF8ComparerInternal();
		utf8impl.reset(u8);
	}
	UErrorCode st = U_ZERO_ERROR;
	return u8->col->compareUTF8(s1.c_str(), s2.c_str(), st);
}

bool UTF8Comparer::operator()(const std::string& x, const std::string& y) const
{
	return compare(x, y) == UCOL_LESS;
}

} /* namespace util */
} /* namespace  jedox */
