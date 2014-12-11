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
 * \author Frank Celler, triagens GmbH, Cologne, Germany
 * \author Achim Brandt, triagens GmbH, Cologne, Germany
 * \author Jiri Junek, qBicon s.r.o., Prague, Czech Republic
 * \author Martin Jakl, qBicon s.r.o., Prague, Czech Republic
 * 
 *
 */

#include "Collections/StringUtils.h"

#include <iostream>
#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "boost/date_time/c_local_time_adjustor.hpp"

#include "Collections/StringBuffer.h"

#include "Exceptions/ParameterException.h"

#undef min

namespace palo {

U_NAMESPACE_QUALIFIER Locale UTF8ComparerInternal::time;
U_NAMESPACE_QUALIFIER Locale UTF8ComparerInternal::num;
U_NAMESPACE_QUALIFIER Locale UTF8ComparerInternal::ctype;
U_NAMESPACE_QUALIFIER Locale UTF8ComparerInternal::coll;
boost::thread_specific_ptr<UTF8ComparerInternal> UTF8Comparer::u8impl;

UTF8Comparer::UTF8Comparer(const std::string &locale)
{
	UErrorCode er = U_ZERO_ERROR;
	colshr.reset(U_NAMESPACE_QUALIFIER Collator::createInstance(U_NAMESPACE_QUALIFIER Locale(locale.c_str()), er));
	if (U_FAILURE(er)) {
		std::stringstream str;
		str << "Initialization of ICU failed with error: " << u_errorName(er);
		throw ErrorException(ErrorException::ERROR_CONVERSION_FAILED, str.str());
	}
	colshr->setAttribute(UCOL_STRENGTH, UCOL_SECONDARY, er);
	if (U_FAILURE(er)) {
		std::stringstream str;
		str << "Initialization of ICU failed with error: " << u_errorName(er);
		throw ErrorException(ErrorException::ERROR_CONVERSION_FAILED, str.str());
	}
}

std::string UTF8Comparer::toUpper(const std::string &s)
{
	UTF8ComparerInternal *u8 = check();
	UErrorCode er = U_ZERO_ERROR;
	int32_t ret = (int32_t)s.size();
	boost::shared_array<char> buf(new char[ret]);
	ret = ucasemap_utf8ToUpper(u8->cm, buf.get(), ret, s.c_str(), (int32_t)s.size(), &er);
	if (er == U_BUFFER_OVERFLOW_ERROR) {
		er = U_ZERO_ERROR;
		buf.reset(new char[ret]);
		ret = ucasemap_utf8ToUpper(u8->cm, buf.get(), ret, s.c_str(), (int32_t) s.size(), &er);
	}
	if (U_FAILURE(er)) {
		std::stringstream str;
		str << "ICU returned unexpected error: " << u_errorName(er);
		throw ErrorException(ErrorException::ERROR_CONVERSION_FAILED, str.str());
	}
	return std::string(buf.get(), ret);
}

std::string UTF8Comparer::toLower(const std::string &s)
{
	UTF8ComparerInternal *u8 = check();
	UErrorCode er = U_ZERO_ERROR;
	int32_t ret = (int32_t)s.size();
	boost::shared_array<char> buf(new char[ret]);
	ret = ucasemap_utf8ToLower(u8->cm, buf.get(), ret, s.c_str(), (int32_t)s.size(), &er);
	if (er == U_BUFFER_OVERFLOW_ERROR) {
		er = U_ZERO_ERROR;
		buf.reset(new char[ret]);
		ret = ucasemap_utf8ToLower(u8->cm, buf.get(), ret, s.c_str(), (int32_t) s.size(), &er);
	}
	if (U_FAILURE(er)) {
		std::stringstream str;
		str << "ICU returned unexpected error: " << u_errorName(er);
		throw ErrorException(ErrorException::ERROR_CONVERSION_FAILED, str.str());
	}
	return std::string(buf.get(), ret);
}

std::string UTF8Comparer::capitalize(const std::string &s)
{
	UTF8ComparerInternal *u8 = check();
	UErrorCode er = U_ZERO_ERROR;
	int32_t ret = (int32_t)s.size();
	boost::shared_array<char> buf(new char[ret]);
	ret = ucasemap_utf8ToTitle(u8->cm, buf.get(), ret, s.c_str(), (int32_t)s.size(), &er);
	if (er == U_BUFFER_OVERFLOW_ERROR) {
		er = U_ZERO_ERROR;
		buf.reset(new char[ret]);
		ret = ucasemap_utf8ToTitle(u8->cm, buf.get(), ret, s.c_str(), (int32_t) s.size(), &er);
	}
	if (U_FAILURE(er)) {
		std::stringstream str;
		str << "ICU returned unexpected error: " << u_errorName(er);
		throw ErrorException(ErrorException::ERROR_CONVERSION_FAILED, str.str());
	}
	return std::string(buf.get(), ret);
}

std::string UTF8Comparer::doubleToString(double d, int32_t padding, int32_t decimals)
{
	UTF8ComparerInternal *u8 = check();
	UErrorCode er = U_ZERO_ERROR;
	u8->formatter->setMinimumFractionDigits(decimals);
	u8->formatter->setMaximumFractionDigits(decimals);
	u8->formatter->setFormatWidth(padding);
	UnicodeString result;
	u8->formatter->format(d, result, er);
	if (U_FAILURE(er)) {
		std::stringstream str;
		str << "ICU returned unexpected error: " << u_errorName(er);
		throw ErrorException(ErrorException::ERROR_CONVERSION_FAILED, str.str());
	}
	string s;
	result.toUTF8String(s);
	return s;
}

void UTF8Comparer::setDefault()
{
	setlocale(LC_ALL, "");
	UTF8ComparerInternal::num = U_NAMESPACE_QUALIFIER Locale(setlocale(LC_NUMERIC, 0));
	UTF8ComparerInternal::time = U_NAMESPACE_QUALIFIER Locale(setlocale(LC_TIME, 0));
	UTF8ComparerInternal::ctype = U_NAMESPACE_QUALIFIER Locale(setlocale(LC_CTYPE, 0));
	UTF8ComparerInternal::coll = U_NAMESPACE_QUALIFIER Locale(setlocale(LC_COLLATE, 0));
	if (string(UTF8ComparerInternal::coll.getName()) == "c") {
		UTF8ComparerInternal::coll = U_NAMESPACE_QUALIFIER Locale::getEnglish();
	}
	setlocale(LC_ALL, "C");
}

size_t UTF8Comparer::len(string &s)
{
	return U_NAMESPACE_QUALIFIER UnicodeString::fromUTF8(s).length();
}

string UTF8Comparer::left(string &s, int32_t l)
{
	string res;
	U_NAMESPACE_QUALIFIER UnicodeString::fromUTF8(s).tempSubString(0, l).toUTF8String(res);
	return res;
}

string UTF8Comparer::right(string &s, int32_t r)
{
	string res;
	U_NAMESPACE_QUALIFIER UnicodeString str = U_NAMESPACE_QUALIFIER UnicodeString::fromUTF8(s);
	str.tempSubString(str.length() - r, r).toUTF8String(res);
	return res;
}

string UTF8Comparer::mid(string &s, int32_t l, int32_t r)
{
	string res;
	if (l >= 0) {
		U_NAMESPACE_QUALIFIER UnicodeString::fromUTF8(s).tempSubString(l - 1, r).toUTF8String(res);
	}
	return res;
}

UTF8ComparerInternal::UTF8ComparerInternal() : col(0), cm(0), formatter(0)
{
	UErrorCode er = U_ZERO_ERROR;
	col = U_NAMESPACE_QUALIFIER Collator::createInstance(coll, er);
	if (U_FAILURE(er)) {
		std::stringstream str;
		str << "Initialization of ICU failed with error: " << u_errorName(er);
		throw ErrorException(ErrorException::ERROR_CONVERSION_FAILED, str.str());
	}
	col->setAttribute(UCOL_STRENGTH, UCOL_SECONDARY, er);
	if (U_FAILURE(er)) {
		delete col;
		std::stringstream str;
		str << "Initialization of ICU failed with error: " << u_errorName(er);
		throw ErrorException(ErrorException::ERROR_CONVERSION_FAILED, str.str());
	}
	cm = ucasemap_open(UTF8ComparerInternal::ctype.getName(), U_FOLD_CASE_DEFAULT, &er);
	if (U_FAILURE(er)) {
		delete col;
		std::stringstream str;
		str << "Initialization of ICU failed with error: " << u_errorName(er);
		throw ErrorException(ErrorException::ERROR_CONVERSION_FAILED, str.str());
	}
	formatter = dynamic_cast<U_NAMESPACE_QUALIFIER DecimalFormat *>(U_NAMESPACE_QUALIFIER NumberFormat::createInstance(UTF8ComparerInternal::num, er));
	if (U_FAILURE(er)) {
		std::stringstream str;
		str << "Initialization of ICU failed with error: " << u_errorName(er);
		throw ErrorException(ErrorException::ERROR_CONVERSION_FAILED, str.str());
	}
	formatter->setPadCharacter(" ");
	formatter->setGroupingSize(0);
}

UTF8ComparerInternal::~UTF8ComparerInternal()
{
	if (col) {
		delete col;
	}
	if (cm) {
		ucasemap_close(cm);
	}
	if (formatter) {
		delete formatter;
	}
}

// /////////////////////////////////////////////////////////////////////////////
// utility functions
// /////////////////////////////////////////////////////////////////////////////

static bool simpleMatch(const char* ptr, const char* end, const char* qtr, const char* qnd)
{
	for (; qtr < qnd; qtr++) {
		if (*qtr == '?') {
			if (ptr >= end) {
				return false;
			}

			ptr++;
		} else if (*qtr == '*') {
			for (; ptr < end; ptr++) {
				if (simpleMatch(ptr, end, qtr + 1, qnd)) {
					return true;
				}
			}

			return false;
		} else if (*qtr == '~') {
			qtr++;

			if (qtr == qnd) {
				return true;
			}

			if (ptr >= end || *ptr != *qtr) {
				return false;
			}

			ptr++;
		} else {
			if (ptr >= end || *ptr != *qtr) {
				return false;
			}

			ptr++;
		}
	}

	return true;
}

// /////////////////////////////////////////////////////////////////////////////
// public methods
// /////////////////////////////////////////////////////////////////////////////

size_t StringUtils::simpleSearch(string text, string search)
{
	text = tolower(text);
	const char * ptr = text.c_str();
	const char * end = ptr + text.size();

	search = tolower(search);
	const char * qtr = search.c_str();
	const char * qnd = qtr + search.size();

	size_t position = 1;
	for (; ptr < end; ptr++, position++) {
		if (simpleMatch(ptr, end, qtr, qnd)) {
			return position;
		}
		if (*ptr) {

		}
	}
	return 0;
}

long StringUtils::stringToInteger(const string& str)
{
	if (str.empty()) {
		throw ParameterException(ErrorException::ERROR_CONVERSION_FAILED, "error converting a string to a number, string is empty", "str", str);
	}

	char *p;

	long i = strtol(str.c_str(), &p, 10);

	if (*p != '\0') {
		throw ParameterException(ErrorException::ERROR_CONVERSION_FAILED, "error converting a string to a number, string has illegal characters", "str", str);
	}

	return i;
}

unsigned long StringUtils::stringToUnsignedInteger(const string& str)
{
	if (str.empty()) {
		throw ParameterException(ErrorException::ERROR_CONVERSION_FAILED, "error converting a string to a number, string is empty", "str", str);
	}

	char *p;

	unsigned long i = strtoul(str.c_str(), &p, 10);

	if (*p != '\0') {
		throw ParameterException(ErrorException::ERROR_CONVERSION_FAILED, "error converting a string to a number, string has illegal characters", "str", str);
	}

	return i;
}

double StringUtils::stringToDouble(const string& str)
{
	if (str.empty()) {
		throw ParameterException(ErrorException::ERROR_CONVERSION_FAILED, "error converting a string to a number, string is empty", "str", str);
	}

	char *p;

	double d = strtod(str.c_str(), &p);

	if (*p != '\0') {
		throw ParameterException(ErrorException::ERROR_CONVERSION_FAILED, "error converting a string to a number, string has illegal characters", "str", str);
	}

	return d;
}

string StringUtils::convertTimeToString(uint64_t tt) {
	struct tm t;
#if defined(_MSC_VER)
	_localtime64_s(&t, (const __time64_t *)&tt);
#else
	if (sizeof(time_t) == 4 && tt > INT_MAX) {
		boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
		epoch = boost::date_time::c_local_adjustor<boost::posix_time::ptime>::utc_to_local(epoch);
		while (tt) {
			long c = (long)std::min(tt, (uint64_t)INT_MAX);
			epoch += boost::posix_time::seconds(c);
			tt -= c;
		}
		t = boost::posix_time::to_tm(epoch);
	} else {
		localtime_r((time_t *)&tt, &t);
	}
#endif
	char timeBuf[32];
	strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", &t);
	return string(timeBuf);
}

string StringUtils::escapeHtml(const string& str)
{
	size_t begin = 0;
	size_t end = str.find_first_of("<>&\n\"");

	if (end == std::string::npos) {
		return str;
	}

	string result = "";
	while (end != std::string::npos) {
		result += str.substr(begin, end - begin);

		switch (str[end]) {
		case '<':
			result += "&lt;";
			break;
		case '>':
			result += "&gt;";
			break;
		case '&':
			result += "&amp;";
			break;
		case '\n':
			result += "<br>";
			break;
		case '"':
			result += "&quot;";
			break;
		}
		begin = end + 1;
		end = str.find_first_of("<>&\n\"", begin);
	}
	result += str.substr(begin);

	return result;
}

string StringUtils::escapeXml(const string& str)
{
	size_t begin = 0;
	size_t end = str.find_first_of("<>&'\"");

	if (end == std::string::npos) {
		return str;
	}

	string result = "";
	while (end != std::string::npos) {
		result += str.substr(begin, end - begin);

		switch (str[end]) {
		case '<':
			result += "&lt;";
			break;
		case '>':
			result += "&gt;";
			break;
		case '&':
			result += "&amp;";
			break;
		case '\'':
			result += "&apos;";
			break;
		case '"':
			result += "&quot;";
			break;
		}
		begin = end + 1;
		end = str.find_first_of("<>&'\"", begin);
	}
	result += str.substr(begin);

	return result;
}

void StringUtils::splitString(const string& line, vector<string>* elements, char seperator)
{
	if (line.empty()) {
		return;
	}

	string s = "";
	bool first = true;
	bool escaped = false;

	size_t len = line.length();
	size_t pos = 0;

	while (pos < len) {
		char c = line[pos];

		if (first) {
			if (line[pos] == '"') {
				escaped = true;
				first = false;
			}

			// empty value found
			else if (line[pos] == seperator) {
				elements->push_back("");
			} else {
				s += c;
				first = false;
			}
		} else {
			if (escaped) {
				if (line[pos] == '"') {
					if (pos + 1 < len) {
						pos++;
						if (line[pos] == seperator) {
							elements->push_back(s);
							s = "";
							first = true;
							escaped = false;
						} else {
							s += c;
						}
					} else {
						elements->push_back(s);
					}
				} else {
					s += c;
				}
			} else {
				if (line[pos] == seperator) {
					elements->push_back(s);
					s = "";
					first = true;
				} else {
					s += c;
				}
			}
		}
		pos++;
	}
	if (!first) {
		elements->push_back(s);
	}
}

string StringUtils::escapeString(const string& text)
{
	StringBuffer sb;

	string buffer = text;
	size_t begin = 0;
	size_t end = buffer.find("\"");

	sb.appendText("\"");

	while (end != string::npos) {
		string result = text.substr(begin, end - begin);

		sb.appendText(result);
		sb.appendText("\"\"");

		begin = end + 1;
		end = buffer.find("\"", begin);
	}

	sb.appendText(text.substr(begin));
	sb.appendText("\"");

	string result(sb.c_str());

	return result;
}

string StringUtils::escapeStringSingle(const string& text)
{
	StringBuffer sb;

	string buffer = text;
	size_t begin = 0;
	size_t end = buffer.find("'");

	sb.appendText("'");

	while (end != string::npos) {
		string result = text.substr(begin, end - begin);

		sb.appendText(result);
		sb.appendText("''");

		begin = end + 1;
		end = buffer.find("'", begin);
	}

	sb.appendText(text.substr(begin));
	sb.appendText("'");

	string result = sb.c_str();

	return result;
}

string StringUtils::getNextElement(string& buffer, size_t& pos, char seperator, bool quote, boost::scoped_array<char> &quoteBuffer)
{
	if (pos >= buffer.size()) {
		return "";
	}

	string result;

	if (quote && buffer[pos] == '"') {
		const char * p = buffer.c_str() + (pos + 1);
		const char * l = p;
		const char * q = buffer.c_str() + (buffer.size());

		if (!quoteBuffer) {
			quoteBuffer.reset(new char[buffer.size()-pos+1]);
		}
		char * k = quoteBuffer.get();

		while (p < q) {
			if (p[0] == '"' && p[1] == '"') {
				*k++ = '"';
				p += 2;
			} else if (p[0] == '"' && p[1] == seperator) {
				p += 2;
				break;
			} else if (p[0] == '"' && p + 1 == q) {
				p += 1;
				break;
			} else {
				*k++ = *p++;
			}
		}

		*k = '\0';

		result = quoteBuffer.get();
		pos += (p - l) + 1;

		return result;
	} else {
		size_t end = buffer.find_first_of(seperator, pos);

		if (end != string::npos) {
			result = buffer.substr(pos, end - pos);
			pos = end + 1;
		} else {
			result = buffer.substr(pos);
			pos = buffer.size() + 1;
		}
	}

	return result;
}

void StringUtils::splitString2(const char* ss, const char* se, vector<vector<string> >* elements, char separator1, char separator2, bool quote)
{

	std::stringstream* sss = new std::stringstream();

	bool newline = true;
	const char* lq = 0;
	int q = 0;

	for (; ss <= se; ++ss) {
		if ((q % 2 == 0) && (ss == se || *ss == separator1 || *ss == separator2)) {
			if (newline)
				elements->push_back(vector<string> ());
			elements->back().push_back(sss->str());
			delete sss;
			newline = (ss == se) || *ss == separator1;
			if (ss == se)
				return;
			lq = 0;
			q = 0;
			sss = new std::stringstream();
		} else if (quote && *ss == '"') {
			++q;
			if (q % 2 == 1 && lq + 1 == ss)
				(*sss) << '"';
			lq = ss;
		} else
			(*sss) << *ss;
	}
	if (sss)
		delete sss;
}

void StringUtils::splitString3(const string& line, vector<string> &elements, char seperator, bool empty)
{
	stringstream part;
	char inquote = 0;
	for (string::const_iterator it = line.begin(); it != line.end(); ++it) {
		if (*it == seperator && !inquote) {
			elements.push_back(part.str());
			part.str("");
			continue;
		} else if (*it == '\'' || *it == '"') {
			if (inquote) {
				if (*it == inquote) {
					string::const_iterator itn(it);
					++itn;
					if (itn != line.end() && (*itn == inquote)) {
						part << *it++;
					} else {
						inquote = 0;
					}
				}
			} else {
				inquote = *it;
			}
		}
		part << *it;
	}
	if (!part.str().empty() || empty) {
		elements.push_back(part.str());
	}
}

string StringUtils::unQuote(const string &s)
{
	stringstream res;
	if (s == "\"\"") {
		return string();
	}
	for (string::const_iterator it = s.begin(); it != s.end(); ++it) {
		if (*it == '\"') {
			if (it + 1 != s.end() && *(it + 1) == '\"') {
				res << *(it++);
			}
		} else {
			res << *it;
		}
	}
	return res.str();
}

}
