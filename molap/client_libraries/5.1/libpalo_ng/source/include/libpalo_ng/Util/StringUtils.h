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
 * \author Frieder Hofmann <frieder.hofmann@jedox.com>
 * \author Hendrik Schmieder <hendrik.schmieder@jedox.com>
 * \author Martin Jakl <martin.jakl@qbicon.cz>
 * 
 *
 */

#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <iosfwd>
#include <vector>
#include <algorithm>

// to avoid macro redefinition induced by boost
#include <boost/version.hpp>
#if BOOST_VERSION >= 104600
#include <stdint.h>
#endif

#include <boost/lexical_cast.hpp>

#include <libpalo_ng/Palo/types.h>
#include <libpalo_ng/Palo/Exception/LibPaloNGExceptionFactory.h>
#include <libpalo_ng/Palo/Exception/PaloException.h>

namespace jedox {
namespace util {
namespace internal {

/*!
 * \brief
 * url encode the character and add it to the stringstream
 *
 * \author
 * Frieder Hofmann <frieder.hofmann@jedox.com>
 */
inline void URLencode(std::ostream& strs, const char val)
{
	switch (val) {
	case ' ':
		strs << "%20";
		break;
	case '#':
		strs << "%23";
		break;
	case '+':
		strs << "%2B";
		break;
	case '&':
		strs << "%26";
		break;
	case '=':
		strs << "%3D";
		break;
	case '\n':
		strs << "%0A";
		break;
	case '\r':
		strs << "%0D";
		break;
	case '%':
		strs << "%25";
		break;
	default:
		strs << val;
		break;
	}
}

/*!
 * \brief
 * don't set anything. see specialization for doubles below
 *
 * \author
 * Frieder Hofmann <frieder.hofmann@jedox.com>
 */
template<class T>
class set_precision {
public:
	set_precision(std::stringstream& /*strs*/)
	{
	}
};

/*!
 * \brief
 * set precision of the stringstream and remove those settings when going out of scope.
 *
 * \author
 * Frieder Hofmann <frieder.hofmann@jedox.com>
 */
template<>
class set_precision<double> {
public:
	set_precision(std::stringstream& strs) :
		m_strs(strs), m_old_precision(strs.precision())
	{
		m_strs.setf(std::ios_base::fixed, std::ios_base::floatfield);
		m_strs.precision(PRECISION);
	}

	~set_precision()
	{
		m_strs.unsetf(std::ios_base::floatfield);
		m_strs.precision(m_old_precision);
	}

private:
	std::stringstream& m_strs;
	const std::streamsize m_old_precision;
};
} // namespace internal

/*!
 * \brief
 * convert a list of T's into a stringstream
 *
 *
 * \author
 * Frieder Hofmann <frieder.hofmann@jedox.com>
 */
template<class T, bool>
inline void TListe(std::stringstream& strs, const std::vector<T>& ids, const char separator, const bool with_asterix = false)
{
	const size_t vsize = ids.size();
	if (with_asterix && (vsize == 0)) {
		strs << '*';
	} else {
		if (vsize > 0) {
			const size_t vsize_minus_one = vsize - 1;
			for (size_t i = 0; i < vsize_minus_one; i++) {
				strs << ids[i] << separator;
			}
			//add the last one without separator2 at the end
			strs << ids[vsize_minus_one];
		}
	}
}

/*!
 * \brief
 * convert a list of T's into a stringstream
 * if we're dealing with doubles the precision
 * of the stringstream will be set before starting!
 *
 *
 * \author
 * Frieder Hofmann <frieder.hofmann@jedox.com>
 */
template<class T>
inline void TListe(std::stringstream& strs, const std::vector<T>& ids, const char separator, const bool with_asterix = false)
{
	internal::set_precision<T> set_p(strs);
	TListe<T, false> (strs, ids, separator, with_asterix);
}

/*!
 * \brief
 * convert a list of T's into a stringstream
 * specialization for CELL_VALUEs because we want to handle them differently
 *
 *
 * \author
 * Frieder Hofmann <frieder.hofmann@jedox.com>
 */
template<>
inline void TListe<jedox::palo::CELL_VALUE> (std::stringstream& strs, const std::vector<jedox::palo::CELL_VALUE>& cellvalues, const char separator, const bool with_asterix)
{
	internal::set_precision<double> set_p(strs);
	const size_t vsize = cellvalues.size();
	if (with_asterix && (vsize == 0)) {
		strs << '*';
	} else {
		for (size_t i = 0; i < vsize; i++) {
			if (cellvalues[i].type == jedox::palo::CELL_VALUE::NUMERIC) {
				strs << cellvalues[i].val.d << separator;
			} else {
				// this will add a '"' then the string value with all '"'
				// repeated twice to encode them followed by another '"' at the end
				// the string will be encoded by URLencode
				strs << '"';
				const std::string& val = cellvalues[i].val.s;
				const std::string::size_type val_size = val.size();
				for (size_t j = 0; j < val_size; ++j) {
					if (val[j] == '"') {
						strs << '"';
					}
					internal::URLencode(strs, val[j]);
				}
				strs << '"';
				strs << separator;
			}
		}
		//remove the last separator at the end
		strs.get();
	}
}

/*!
 * \brief
 * convert a vector of vectors of T's into a stringstream
 *
 *
 * \author
 * Frieder Hofmann <frieder.hofmann@jedox.com>
 */
template<class T>
inline void ListeTListe(std::stringstream& strs, const std::vector<std::vector<T> > & ids, const char separator1, char separator2, const bool with_asterix = false)
{
	internal::set_precision<T> set_p(strs);
	const size_t vsize = ids.size(), vsize_minus_one = vsize - 1;
	if (vsize > 0) {
		for (size_t i = 0; i < vsize_minus_one; i++) {
			TListe<T, false> (strs, ids[i], separator1, with_asterix);
			strs << separator2;
		}
		//add the last one without separator2 at the end
		TListe<T, false> (strs, ids[vsize_minus_one], separator1, with_asterix);
	}
}

/*!
 * \brief
 * encode all special chars
 *
 *
 * \author
 * Frieder Hofmann <frieder.hofmann@jedox.com>
 */
inline void URLencoder(std::ostream& strs, const std::string& val)
{
	const std::string::size_type val_size = val.size();
	for (size_t j = 0; j < val_size; ++j) {
		internal::URLencode(strs, val[j]);
	}
}

/*!
 * \brief
 * convert and url encode a vector of strings into a stringstream
 *
 *
 * \author
 * Frieder Hofmann <frieder.hofmann@jedox.com>
 */
inline void UrlEncodeTListe(std::stringstream& strs, const std::vector<std::string>& coded_names, const char separator, const bool with_asterix = false)
{
	const size_t vsize = coded_names.size();
	if (with_asterix && (vsize == 0)) {
		strs << '*' << separator;
	} else {
		for (size_t i = 0; i < vsize; i++) {
			URLencoder(strs, coded_names[i]);
			strs << separator;
		}
		//remove the last separator2 at the end
		strs.get();
	}
}

struct UTF8ComparerInternal;

class LIBPALO_NG_CLASS_EXPORT UTF8Comparer
{
public:
	bool operator()(const std::string& x, const std::string& y) const;
	static int compare(const std::string &s1, const std::string &s2);
	static std::string toUpper(const std::string &s);
	static std::string toLower(const std::string &s);
};

class LIBPALO_NG_CLASS_EXPORT StringUtils {
public:
	static std::string CSVencode(const std::string& val, const char delimeter = '"');
	static std::string URLencode(const std::string& val);
	static std::string escapeString(const std::string& text);
	static std::string Numeric2String(double value);
	static std::string toUpper(const std::string &s) {return UTF8Comparer::toUpper(s);}
	static std::string toLower(const std::string &s) {return UTF8Comparer::toLower(s);}
};

#define lexicalConversion(a, b, c) lexicalConversionInt<a, b>(c, #a, #b)
template<class T, class T2> inline T lexicalConversionInt(const T2 &s, const char *tName, const char *t2Name)
{
	try {
		return boost::lexical_cast<T>(s);
	} catch (boost::bad_lexical_cast &) {
		std::stringstream str;
		str << "Server response is syntactically invalid, boost::lexical_cast returned error. Can't convert :\"" << s << "\" from " << t2Name << " to " << tName;
		std::string desc = str.str();
		palo::LibPaloNGExceptionFactory::raise(palo::LibPaloNGExceptionFactory::PALO_NG_ERROR_SERVER_RESPONSE_SYNTACTICALLY_INVALID, &desc);
		throw;
	}
}

} /* namespace util */
} /* namespace  jedox */
#endif							 // STRINGUTILS_H
