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
 * \author Frieder Hofmann <frieder.hofmann@jedox.com>
 * 
 *
 */

#include <iosfwd>
#include <string>
#include <istream>

namespace jedox {
namespace util {

template<typename T>
struct pass_by_value_or_reference {
	typedef const T& type;
};

template<>
struct pass_by_value_or_reference<int> {
	typedef const int type;
};

template<>
struct pass_by_value_or_reference<unsigned int> {
	typedef const unsigned int type;
};

template<>
struct pass_by_value_or_reference<long> {
	typedef const long type;
};

template<>
struct pass_by_value_or_reference<double> {
	typedef const double type;
};

template<>
struct pass_by_value_or_reference<bool> {
	typedef const bool type;
};

class CsvTokenFromStream {
public:
	CsvTokenFromStream(std::istream& stream, char seperator = ';');

	/*!
	 * \brief
	 * get a value, this will set value_to_set to either the next value given by the stream or the default value
	 * the format that is parsed (and consumed) is the value, followed by either nothing, ';' , ";\n" or ";\r"
	 * in case we have more values following this one, true is returned, false otherwise
	 *
	 * \author
	 * Frieder Hofmann <frieder.hofmann@jedox.com>
	 */
	template<class T>
	inline void get(T& value_to_set, typename pass_by_value_or_reference<T>::type default_value)
	{
		char c;
		if (!m_Done) {
			if (m_Stream.get(c) && m_Stream.eof() == false) {
				if (c != m_Seperator) {
					m_Stream.unget();
					m_Stream >> value_to_set;
					char dummy; //dummy to remove the trailing separator
					if (m_Stream.get(dummy)) {
						if (dummy == m_Seperator) {
							check_if_done();
							return;
						}
					}
				} else {
					value_to_set = default_value;
					check_if_done();
					return;
				}
			} else {
				m_Done = true;
			}
		}
		value_to_set = default_value;
	}

	inline void get(unsigned long& value_to_set, unsigned long default_value) {
		if (!m_Done) {
			char c;
			if (m_Stream.get(c) && m_Stream.eof() == false) {
				if (c != m_Seperator) {
					value_to_set = 0;
					while (c >= '0' && c <= '9' && !m_Stream.eof() ) {
						value_to_set *= 10;
						value_to_set += ( c - '0' );
						m_Stream.get(c);
					}
					if (c == m_Seperator) {
						check_if_done();
						return;
					}
					return;
				} else {
					value_to_set = default_value;
					check_if_done();
					return;
				}
			} else {
				m_Done = true;
			}
			value_to_set = default_value;
		}
	}

	inline void get(unsigned int& value_to_set, unsigned int default_value){
		unsigned long v = 0;
		get(v, default_value);
		value_to_set = (unsigned int)v;
	}

	inline void get(long& value_to_set, long default_value) {
		if (!m_Done) {
			char c;
			if (m_Stream.get(c) && m_Stream.eof() == false) {
				if (c != m_Seperator) {
					value_to_set = 0;
					int sign = 1;
					if (!m_Stream.eof() && c == '-') {
						sign = -1;
						m_Stream.get(c);
					}
					while (c >= '0' && c <= '9' && !m_Stream.eof()) {
						value_to_set *= 10;
						value_to_set += ( c - '0' );
						m_Stream.get(c);
					}
					value_to_set *= sign;
					if (c == m_Seperator) {
						check_if_done();
						return;
					}
					return;
				} else {
					value_to_set = default_value;
					check_if_done();
					return;
				}
			} else {
				m_Done = true;
			}
			value_to_set = default_value;
		}
	}

	inline void get(int& value_to_set, int default_value){
		long v;
		get(v, default_value);
		value_to_set = (int)v;
	}

	inline void get(std::string& value_to_set, char sep = 0)
	{
		value_to_set.clear();
		if (m_Done) {
			return;
		}
		char current;
		bool inQuotes = false;

		while (m_Stream.get(current) && m_Stream.eof() == false) {
			if (inQuotes == false && current == '\r') {
				continue;
			}
			if (current == '"') {
				m_Stream.get(current);
				if (m_Stream.eof()) {
					m_Done = true;
					return;
				}
				if (current != '"') {

					inQuotes = !inQuotes;
				}
			}
			if (inQuotes == false) {
				if (current == m_Seperator || current == sep) {
					if (m_Stream.eof() == false && m_Stream.peek() == '\n') {
						m_Stream.get();
						m_Done = true;
						return;
					} else {
						return;
					}
				}
			}
			value_to_set.push_back(current);
		}
		//this is a special case, where we get "" as string (which doesn't make much sense to begin with)
		if (value_to_set.size() == 1 && value_to_set[0] == '"') {
			value_to_set.clear();
		}
		return;
	}

	template<class Vec>
	inline void get_list(Vec& list, const typename Vec::size_type size_to_expect = 0)
	{
		typedef typename Vec::value_type v_type;
		char c;
		list.reserve(size_to_expect);
		while (m_Stream.get(c) && m_Stream.eof() == false) {
			if (c != m_Seperator) {
				m_Stream.unget();
				v_type val;
				m_Stream >> val;
				char dummy; //dummy to remove the trailing separator
				list.push_back(val);
				if (m_Stream.get(dummy)) {
					if (dummy != ',') {
						break;
					}
				}
			} else {
				break;
			}
		}
		check_if_done();
		return;

	}

	inline void clear_done()
	{
		m_Done = false;
	}

private:
	inline void check_if_done()
	{
		if (m_Stream.peek() == '\n') {
			m_Stream.get();
			m_Done = true;
		}
	}
private:
	std::istream& m_Stream;
	char m_Seperator;
	bool m_Done;
};

std::ostream& operator<<(std::ostream& out, CsvTokenFromStream& tokenStream);

} /* util */
} /* jedox */
