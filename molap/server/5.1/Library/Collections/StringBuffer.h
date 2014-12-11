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
 * 
 *
 */

#ifndef COLLECTIONS_STRING_BUFFER_H
#define COLLECTIONS_STRING_BUFFER_H 1

#include <stdio.h>

#include "palo.h"

namespace palo {

#define DEFSTR(a,b) static const char __STRING_ ## a [] = b
#define STR(a) __STRING_ ## a
#define LENSTR(a) (sizeof(__STRING_ ## a) - 1)

////////////////////////////////////////////////////////////////////////////////
/// @brief string buffer with formatting routines
///
/// @warning You must initialize the classes by calling initialize or by
/// setting everything to 0. This can be done by using "new
/// StringBuffer()". You must call free to free the allocated memory.
////////////////////////////////////////////////////////////////////////////////

struct StringBuffer {

	////////////////////////////////////////////////////////////////////////////////
	/// @brief initializes the string buffer
	///
	/// This method is implemented here in order to allow inlining.
	///
	/// @warning You must call initialize before using the string buffer.
	////////////////////////////////////////////////////////////////////////////////

	StringBuffer() {
		buffer = 0;
		bufferPtr = 0;
		bufferEnd = 0;

		reserve(1);
		*bufferPtr = 0;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief frees the string buffer
	///
	/// This method is implemented here in order to allow inlining.
	///
	/// @warning You must call free after using the string buffer.
	////////////////////////////////////////////////////////////////////////////////

	~StringBuffer() {
		if (buffer != 0) {
			delete[] buffer;

			buffer = 0;
			bufferPtr = 0;
			bufferEnd = 0;
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief swaps content with another string buffer
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	void swap(StringBuffer * other) {
		char * otherBuffer = other->buffer;
		char * otherBufferPtr = other->bufferPtr;
		char * otherBufferEnd = other->bufferEnd;

		other->buffer = buffer;
		other->bufferPtr = bufferPtr;
		other->bufferEnd = bufferEnd;

		buffer = otherBuffer;
		bufferPtr = otherBufferPtr;
		bufferEnd = otherBufferEnd;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns pointer to the character buffer
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	const char * c_str() const {
		return buffer;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns pointer to the character buffer
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	char * str() {
		return buffer;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns pointer to the beginning of the character buffer
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	const char * begin() const {
		return buffer;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns pointer to the end of the character buffer
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	const char * end() const {
		return bufferPtr;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns length of the character buffer
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	size_t length() const {
		return bufferPtr - buffer;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns true if buffer is empty
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	bool empty() const {
		return bufferPtr == buffer;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief clears the buffer
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	void clear() {
		bufferPtr = buffer;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief assigns text from a string
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	StringBuffer& operator=(const string& str) {
		replaceText(str.c_str(), str.length());

		return *this;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief copies the string buffer
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	void copy(const StringBuffer& copy) {
		replaceText(copy.c_str(), copy.length());
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief removes the first characters
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	void erase_front(size_t len) {
		if (length() <= len) {
			clear();
		} else if (0 < len) {
			memmove(buffer, buffer + len, bufferPtr - buffer - len);
			bufferPtr -= len;
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief appends eol character
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	void appendEol() {
		appendChar('\n');
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief appends character
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	void appendChar(char chr) {
		reserve(1);
		*bufferPtr++ = chr;
		*bufferPtr = '\0';
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief replaces characters
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	void replaceText(const char * str, size_t len) {
		clear();
		appendText(str, len);
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief replaces characters
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	void replaceText(const StringBuffer& buffer) {
		clear();
		appendText(buffer.c_str(), buffer.length());
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief appends blob
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	void appendData(const void * str, size_t len) {
		reserve(len);
		memcpy(bufferPtr, str, len);
		bufferPtr += len;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief appends characters
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	void appendText(const char * str, size_t len) {
		reserve(len + 1);
		memcpy(bufferPtr, str, len);

		bufferPtr += len;
		*bufferPtr = '\0';
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief appends characters
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	void appendText(const char * str) {
		size_t len = strlen(str);

		reserve(len + 1);
		memcpy(bufferPtr, str, len + 1);
		bufferPtr += len;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief appends string
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	void appendText(const string& str) {
		appendText(str.c_str(), str.length());
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief appends a string buffer
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	void appendText(const StringBuffer& buffer) {
		appendText(buffer.c_str(), buffer.length());
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief appends integer with two digits
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	void appendInteger2(uint32_t attr) {
		reserve(2);
		appendChar0((char)((attr / 10L) % 10 + '0'));
		appendChar0((char)(char(attr % 10 + '0')));
		*bufferPtr = '\0';
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief appends integer with four digits
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	void appendInteger4(uint32_t attr) {
		reserve(4);
		appendChar0((char)((attr / 1000L) % 10 + '0'));
		appendChar0((char)((attr / 100L) % 10 + '0'));
		appendChar0((char)((attr / 10L) % 10 + '0'));
		appendChar0((char)((attr % 10 + '0')));
		*bufferPtr = '\0';
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief appends unsigned integer with 8 bits
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	void appendInteger(uint8_t attr) {
		reserve(3);

		if (100L <= attr) {
			appendChar0((char)((attr / 100L) % 10 + '0'));
		}
		if (10L <= attr) {
			appendChar0((char)((attr / 10L) % 10 + '0'));
		}

		appendChar0((char)(attr % 10 + '0'));
		*bufferPtr = '\0';
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief appends unsigned integer with 32 bits
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	void appendInteger(uint32_t attr) {
		reserve(10);

		if (1000000000L <= attr) {
			appendChar0((char)((attr / 1000000000L) % 10 + '0'));
		}
		if (100000000L <= attr) {
			appendChar0((char)((attr / 100000000L) % 10 + '0'));
		}
		if (10000000L <= attr) {
			appendChar0((char)((attr / 10000000L) % 10 + '0'));
		}
		if (1000000L <= attr) {
			appendChar0((char)((attr / 1000000L) % 10 + '0'));
		}
		if (100000L <= attr) {
			appendChar0((char)((attr / 100000L) % 10 + '0'));
		}
		if (10000L <= attr) {
			appendChar0((char)((attr / 10000L) % 10 + '0'));
		}
		if (1000L <= attr) {
			appendChar0((char)((attr / 1000L) % 10 + '0'));
		}
		if (100L <= attr) {
			appendChar0((char)((attr / 100L) % 10 + '0'));
		}
		if (10L <= attr) {
			appendChar0((char)((attr / 10L) % 10 + '0'));
		}

		appendChar0((char)(attr % 10 + '0'));
		*bufferPtr = '\0';
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief appends unsigned integer with 64 bits
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	void appendInteger(uint64_t attr) {
		if ((attr >> 32) == 0) {
			appendInteger(uint32_t(attr));
			return;
		}

		reserve(20);

		// uint64_t has one more decimal than int64_t
		if (10000000000000000000ULL <= attr) {
			appendChar0((char)((attr / 10000000000000000000ULL) % 10 + '0'));
		}
		if (1000000000000000000ULL <= attr) {
			appendChar0((char)((attr / 1000000000000000000ULL) % 10 + '0'));
		}
		if (100000000000000000ULL <= attr) {
			appendChar0((char)((attr / 100000000000000000ULL) % 10 + '0'));
		}
		if (10000000000000000ULL <= attr) {
			appendChar0((char)((attr / 10000000000000000ULL) % 10 + '0'));
		}
		if (1000000000000000ULL <= attr) {
			appendChar0((char)((attr / 1000000000000000ULL) % 10 + '0'));
		}
		if (100000000000000ULL <= attr) {
			appendChar0((char)((attr / 100000000000000ULL) % 10 + '0'));
		}
		if (10000000000000ULL <= attr) {
			appendChar0((char)((attr / 10000000000000ULL) % 10 + '0'));
		}
		if (1000000000000ULL <= attr) {
			appendChar0((char)((attr / 1000000000000ULL) % 10 + '0'));
		}
		if (100000000000ULL <= attr) {
			appendChar0((char)((attr / 100000000000ULL) % 10 + '0'));
		}
		if (10000000000ULL <= attr) {
			appendChar0((char)((attr / 10000000000ULL) % 10 + '0'));
		}
		if (1000000000ULL <= attr) {
			appendChar0((char)((attr / 1000000000ULL) % 10 + '0'));
		}
		if (100000000ULL <= attr) {
			appendChar0((char)((attr / 100000000ULL) % 10 + '0'));
		}
		if (10000000ULL <= attr) {
			appendChar0((char)((attr / 10000000ULL) % 10 + '0'));
		}
		if (1000000ULL <= attr) {
			appendChar0((char)((attr / 1000000ULL) % 10 + '0'));
		}
		if (100000ULL <= attr) {
			appendChar0((char)((attr / 100000ULL) % 10 + '0'));
		}
		if (10000ULL <= attr) {
			appendChar0((char)((attr / 10000ULL) % 10 + '0'));
		}
		if (1000ULL <= attr) {
			appendChar0((char)((attr / 1000ULL) % 10 + '0'));
		}
		if (100ULL <= attr) {
			appendChar0((char)((attr / 100ULL) % 10 + '0'));
		}
		if (10ULL <= attr) {
			appendChar0((char)((attr / 10ULL) % 10 + '0'));
		}

		appendChar0((char)(attr % 10 + '0'));
		*bufferPtr = '\0';
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief appends integer with 64 bits
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	void appendInteger(int64_t attr) {
		if (attr < 0) {
			reserve(1);
			appendChar0('-');
			attr = -attr;
		}

		if ((attr >> 32) == 0) {
			appendInteger(uint32_t(attr));
			return;
		}

		reserve(19);

		// uint64_t has one more decimal than int64_t
		if (1000000000000000000LL <= attr) {
			appendChar0((char)((attr / 1000000000000000000LL) % 10 + '0'));
		}
		if (100000000000000000LL <= attr) {
			appendChar0((char)((attr / 100000000000000000LL) % 10 + '0'));
		}
		if (10000000000000000LL <= attr) {
			appendChar0((char)((attr / 10000000000000000LL) % 10 + '0'));
		}
		if (1000000000000000LL <= attr) {
			appendChar0((char)((attr / 1000000000000000LL) % 10 + '0'));
		}
		if (100000000000000LL <= attr) {
			appendChar0((char)((attr / 100000000000000LL) % 10 + '0'));
		}
		if (10000000000000LL <= attr) {
			appendChar0((char)((attr / 10000000000000LL) % 10 + '0'));
		}
		if (1000000000000LL <= attr) {
			appendChar0((char)((attr / 1000000000000LL) % 10 + '0'));
		}
		if (100000000000LL <= attr) {
			appendChar0((char)((attr / 100000000000LL) % 10 + '0'));
		}
		if (10000000000LL <= attr) {
			appendChar0((char)((attr / 10000000000LL) % 10 + '0'));
		}
		if (1000000000LL <= attr) {
			appendChar0((char)((attr / 1000000000LL) % 10 + '0'));
		}
		if (100000000LL <= attr) {
			appendChar0((char)((attr / 100000000LL) % 10 + '0'));
		}
		if (10000000LL <= attr) {
			appendChar0((char)((attr / 10000000LL) % 10 + '0'));
		}
		if (1000000LL <= attr) {
			appendChar0((char)((attr / 1000000LL) % 10 + '0'));
		}
		if (100000LL <= attr) {
			appendChar0((char)((attr / 100000LL) % 10 + '0'));
		}
		if (10000LL <= attr) {
			appendChar0((char)((attr / 10000LL) % 10 + '0'));
		}
		if (1000LL <= attr) {
			appendChar0((char)((attr / 1000LL) % 10 + '0'));
		}
		if (100LL <= attr) {
			appendChar0((char)((attr / 100LL) % 10 + '0'));
		}
		if (10LL <= attr) {
			appendChar0((char)((attr / 10LL) % 10 + '0'));
		}

		appendChar0((char)(attr % 10 + '0'));
		*bufferPtr = '\0';
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief appends integer with 32 bits
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	void appendInteger(int32_t attr) {
		reserve(11);

		if (attr < 0) {
			appendChar0('-');
			attr = -attr;
		}

		if (1000000000L <= attr) {
			appendChar0((char)((attr / 1000000000L) % 10 + '0'));
		}
		if (100000000L <= attr) {
			appendChar0((char)((attr / 100000000L) % 10 + '0'));
		}
		if (10000000L <= attr) {
			appendChar0((char)((attr / 10000000L) % 10 + '0'));
		}
		if (1000000L <= attr) {
			appendChar0((char)((attr / 1000000L) % 10 + '0'));
		}
		if (100000L <= attr) {
			appendChar0((char)((attr / 100000L) % 10 + '0'));
		}
		if (10000L <= attr) {
			appendChar0((char)((attr / 10000L) % 10 + '0'));
		}
		if (1000L <= attr) {
			appendChar0((char)((attr / 1000L) % 10 + '0'));
		}
		if (100L <= attr) {
			appendChar0((char)((attr / 100L) % 10 + '0'));
		}
		if (10L <= attr) {
			appendChar0((char)((attr / 10L) % 10 + '0'));
		}

		appendChar0((char)(attr % 10 + '0'));
		*bufferPtr = '\0';
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief appends integer with 8 bits
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	void appendInteger(int8_t attr) {
		reserve(4);

		if (attr < 0) {
			appendChar0('-');
			attr = -attr;
		}

		if (100L <= attr) {
			appendChar0((char)((attr / 100L) % 10 + '0'));
		}
		if (10L <= attr) {
			appendChar0((char)((attr / 10L) % 10 + '0'));
		}

		appendChar0((char)(attr % 10 + '0'));
		*bufferPtr = '\0';
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief appends size_t
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

#ifdef OVERLOAD_FUNCS_SIZE_T

#if SIZEOF_SIZE_T == 4

	void appendInteger(size_t attr) {
		appendInteger(uint32_t(attr));
	}

#elif SIZEOF_SIZE_T == 8

	void appendInteger(size_t attr) {
		appendInteger(uint64_t(attr));
	}

#endif

#endif

	////////////////////////////////////////////////////////////////////////////////
	/// @brief appends integer with 8 bits
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	void appendDecimal(double attr) 
	{
		char b[1024];
		size_t written = snprintf(b, sizeof(b) - 1, "%.15g", attr);

		// What should be done in case of an overflow? Should not happen
		if (sizeof(b) <= written) {
			written = sizeof(b) - 1;
			b[written] = '\0';
		}

		appendText(b, written);
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief appends csv string
	////////////////////////////////////////////////////////////////////////////////

	void appendCsvString(const string& text) {
		// do not escape here, because some string - i.e. lists of identifier - have no special characters
		appendText(text);
		appendChar(';');
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief appends csv integer
	////////////////////////////////////////////////////////////////////////////////

	void appendCsvInteger(int32_t i) {
		appendInteger(i);
		appendChar(';');
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief appends csv integer
	////////////////////////////////////////////////////////////////////////////////

	void appendCsvInteger(uint32_t i) {
		appendInteger(i);
		appendChar(';');
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief appends csv integer
	////////////////////////////////////////////////////////////////////////////////

	void appendCsvInteger(uint64_t i) {
		appendInteger(i);
		appendChar(';');
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief appends csv double
	////////////////////////////////////////////////////////////////////////////////

	void appendCsvDouble(double d) {
		appendDecimal(d);
		appendChar(';');
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief reserves space
	////////////////////////////////////////////////////////////////////////////////

	void reserve(size_t size) {
		if (buffer == 0) {
			buffer = new char[size + 1];
			bufferPtr = buffer;
			bufferEnd = buffer + size;
		} else if (size_t(bufferEnd - bufferPtr) < size) {
			size_t newlen = size_t(1.2 * ((bufferEnd - buffer) + size));
			char * b = new char[newlen + 1];

			memcpy(b, buffer, bufferEnd - buffer + 1);

			delete[] buffer;

			bufferPtr = b + (bufferPtr - buffer);
			bufferEnd = b + newlen;
			buffer = b;
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief appends a character without boundary check
	////////////////////////////////////////////////////////////////////////////////

	void appendChar0(char chr) {
		*bufferPtr++ = chr;
	}

private:

	// /////////////////////////////////////////////////////////////////////////////
	// private data
	// /////////////////////////////////////////////////////////////////////////////

	char * buffer;
	char * bufferPtr;
	char * bufferEnd;
};
}

#endif
