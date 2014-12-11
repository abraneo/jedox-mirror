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

#ifndef COLLECTIONS_VECTOR_H
#define COLLECTIONS_VECTOR_H 1

#include "palo.h"

namespace palo {

////////////////////////////////////////////////////////////////////////////////
/// @brief POD vector
///
/// @warning You must initialize the classes by calling initialize or by
/// setting everything to 0. This can be done by using "new Vector()". You must
/// call free to free the allocated memory.
////////////////////////////////////////////////////////////////////////////////

template<class EL> struct Vector {
	typedef EL value_type;

	////////////////////////////////////////////////////////////////////////////////
	/// @brief initializes a new vector
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	void initialize() {
		buffer = 0;
		length = 0;
		capacity = 0;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief frees the buffer
	///
	/// Frees the internal buffer. It is possible to call push_back
	/// afterwards. A new buffer will be allocated.
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	void free() {
		if (buffer != 0) {
			delete[] buffer;
			buffer = 0;
			length = 0;
			capacity = 0;
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief copies a vector
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	void copy(const Vector<EL> * copy) {
		if (length != copy->length) {
			free();
			buffer = new EL[copy->length];
			length = copy->length;
			capacity = length;
		}

		// cannot use memcpy in case of objects
		for (size_t i = 0; i < length; i++) {
			buffer[i] = copy->buffer[i];
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief copies a vector
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	void copy(const Vector<EL>& copy) {
		if (length != copy.length) {
			free();
			buffer = new EL[copy.length];
			length = copy.length;
			capacity = length;
		}

		// cannot use memcpy in case of objects
		for (size_t i = 0; i < length; i++) {
			buffer[i] = copy.buffer[i];
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief size of the vector
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	size_t size() const {
		return length;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief tests if vector is empty
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	bool empty() const {
		return length == 0;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief clears the vector
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	void clear() {
		length = 0;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief adds a new element at the end
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	void push_back(const EL& element) {
		if (length == capacity) {
			capacity = int32_t(1 + 1.2 * capacity);

			EL * newBuffer = new EL[capacity];

			if (buffer != 0) {
				memcpy(newBuffer, buffer, length * sizeof(EL));
				delete[] buffer;
			}

			buffer = newBuffer;
		}

		buffer[length++] = element;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief removes an element from the end
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	void pop_back() {
		if (0 < length) {
			length--;
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the element at a given position
	///
	/// This method is implemented here in order to allow inlining.
	///
	/// @warning No boundary checks are performed.
	////////////////////////////////////////////////////////////////////////////////

	EL& at(size_t pos) {
		return buffer[pos];
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns the element at a given position
	///
	/// This method is implemented here in order to allow inlining.
	///
	/// @warning No boundary checks are performed.
	////////////////////////////////////////////////////////////////////////////////

	const EL& at(size_t pos) const {
		return buffer[pos];
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns a pointer to element array
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	EL * begin() const {
		return buffer;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// @brief returns a pointer to behind the last element
	///
	/// This method is implemented here in order to allow inlining.
	////////////////////////////////////////////////////////////////////////////////

	EL * end() const {
		return buffer + length;
	}

	// private data
	EL * buffer;
	uint32_t length;
	uint32_t capacity;
};

}

#endif
