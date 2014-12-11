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

#ifndef COLLECTIONS_STRING_UTILS_H
#define COLLECTIONS_STRING_UTILS_H 1

#include "palo.h"

#include <sstream>
#include <time.h>
#include <unicode/coll.h>
#include <unicode/ucasemap.h>
#include <unicode/decimfmt.h>
#include <boost/thread/tss.hpp>
#include <boost/scoped_array.hpp>

#include "Exceptions/ErrorException.h"

namespace palo {

struct UTF8ComparerInternal {
	U_NAMESPACE_QUALIFIER Collator *col;
	UCaseMap *cm;
	U_NAMESPACE_QUALIFIER DecimalFormat *formatter;
	UTF8ComparerInternal();
	~UTF8ComparerInternal();
	static U_NAMESPACE_QUALIFIER Locale time;
	static U_NAMESPACE_QUALIFIER Locale num;
	static U_NAMESPACE_QUALIFIER Locale ctype;
	static U_NAMESPACE_QUALIFIER Locale coll;
};

class SERVER_CLASS UTF8Comparer
{
public:
	UTF8Comparer() {}
	UTF8Comparer(const std::string &locale);
	bool operator()(const std::string& x, const std::string& y) const {if (colshr) return colshr->compare(U_NAMESPACE_QUALIFIER UnicodeString::fromUTF8(x.c_str()), U_NAMESPACE_QUALIFIER UnicodeString::fromUTF8(y.c_str())) < 0; else return compare(x, y) < 0;}
	bool operator()(const U_NAMESPACE_QUALIFIER UnicodeString& x, const U_NAMESPACE_QUALIFIER UnicodeString& y) const {return colshr->compare(x, y) < 0;}
	static int compare(const char *s1, const char *s2) {UTF8ComparerInternal *u8 = check(); return u8->col->compare(U_NAMESPACE_QUALIFIER UnicodeString::fromUTF8(s1), U_NAMESPACE_QUALIFIER UnicodeString::fromUTF8(s2));}
	static int compare(const std::string &s1, const std::string &s2) {UTF8ComparerInternal *u8 = check(); return u8->col->compare(U_NAMESPACE_QUALIFIER UnicodeString::fromUTF8(s1.c_str()), U_NAMESPACE_QUALIFIER UnicodeString::fromUTF8(s2.c_str()));}
	static std::string toUpper(const std::string &s);
	static std::string toLower(const std::string &s);
	static std::string capitalize(const std::string &s);
	static std::string doubleToString(double d, int32_t padding, int32_t decimals);
	static void setDefault();
	static size_t len(std::string &s);
	static std::string left(std::string &s, int32_t l);
	static std::string right(std::string &s, int32_t r);
	static std::string mid(std::string &s, int32_t l, int32_t r);
private:
	static UTF8ComparerInternal *check() {UTF8ComparerInternal *u8 = u8impl.get(); if (!u8) {u8 = new UTF8ComparerInternal(); u8impl.reset(u8);} return u8;}
	static boost::thread_specific_ptr<UTF8ComparerInternal> u8impl;
	boost::shared_ptr<U_NAMESPACE_QUALIFIER Collator> colshr;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief string utilities
////////////////////////////////////////////////////////////////////////////////

class SERVER_CLASS StringUtils {
public:

	////////////////////////////////////////////////////////////////////////////////
	/// @brief checks for a suffix
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	static bool isSuffix(const string& str, const string& postfix) {
		if (postfix.length() > str.length()) {
			return false;
		} else if (postfix.length() == str.length()) {
			return str == postfix;
		} else {
			return str.compare(str.size() - postfix.length(), postfix.length(), postfix) == 0;
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief search text
	///
	/// Simple text search with wild cards.
	////////////////////////////////////////////////////////////////////////////////

	static size_t simpleSearch(string text, string search);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief converts double to string
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	static string convertToString(double i) {
		stringstream str;

		str << i;

		return str.str();
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief converts integer to string
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	static string convertToString(int64_t i) {
		stringstream str;

		str << i;

		return str.str();
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief converts integer to string
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	static string convertToString(int32_t i) {
		stringstream str;

		str << i;

		return str.str();
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief converts unsigned integer to string
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	static string convertToString(uint64_t i) {
		stringstream str;

		str << i;

		return str.str();
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief converts unsigned integer to string
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	static string convertToString(uint32_t i) {
		stringstream str;

		str << i;

		return str.str();
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief converts time_t to string using format %Y-%m-%d %H:%M:%S
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	static string convertTimeToString(uint64_t tt);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief converts size_t to string
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

#ifdef OVERLOAD_FUNCS_SIZE_T_LONG

	static string convertToString(size_t i) {
		stringstream str;

		str << i;

		return str.str();
	}

#endif

	////////////////////////////////////////////////////////////////////////////////
	/// @brief converts string to lower case
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	static string tolower(const string& str) {return UTF8Comparer::toLower(str);}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief converts string to upper case
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	static string toupper(const string& str) {return UTF8Comparer::toUpper(str);}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief capitalization of a string
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	static string capitalization(const string& str) {return UTF8Comparer::capitalize(str);}

	////////////////////////////////////////////////////////////////////////////////
	// @brief deletes white spaces at the beginning and the end of a string
	////////////////////////////////////////////////////////////////////////////////

	static string trim(const string& str) {
		size_t s = str.find_first_not_of(" \t\n\r");
		size_t e = str.find_last_not_of(" \t\n\r");

		if (s == std::string::npos) {
			return string();
		} else {
			return string(str, s, e - s + 1);
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief escapes html special characters
	////////////////////////////////////////////////////////////////////////////////

	static string escapeHtml(const string& str);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief escapes xml special characters
	////////////////////////////////////////////////////////////////////////////////

	static string escapeXml(const string& str);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief escapes special characters by "
	////////////////////////////////////////////////////////////////////////////////

	static string escapeString(const string& text);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief escapes special characters by '
	////////////////////////////////////////////////////////////////////////////////

	static string escapeStringSingle(const string& text);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief converts string to integer
	////////////////////////////////////////////////////////////////////////////////

	static long stringToInteger(const string& str);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief converts string to unsigned integer
	////////////////////////////////////////////////////////////////////////////////

	static unsigned long stringToUnsignedInteger(const string& str);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief converts string to double
	////////////////////////////////////////////////////////////////////////////////

	static double stringToDouble(const string& str);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief splits string along seperator
	////////////////////////////////////////////////////////////////////////////////

	static void splitString(const string& line, vector<string>* elements, char seperator);

	////////////////////////////////////////////////////////////////////////////////
	/// @brief extracts next element from string
	////////////////////////////////////////////////////////////////////////////////

	static string getNextElement(string& buffer, size_t& pos, const char seperator, bool quote, boost::scoped_array<char> &quoteBuffer);

	static void splitString2(const char* ss, const char* se, vector<vector<string> >* elements, char separator1, char separator2, bool quote);
	static void splitString3(const string& line, vector<string> &elements, char seperator, bool empty);
	static string unQuote(const std::string &s);
};

}
#endif
