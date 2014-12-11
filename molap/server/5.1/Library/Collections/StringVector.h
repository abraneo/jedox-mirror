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
 * \author Martin Dolezal, qBicon s.r.o., Hradec Kralove, Czech Republic
 * 
 *
 */

#ifndef STRINGVECTOR_H_
#define STRINGVECTOR_H_


#include "SlimVector.h"
#include <iostream>

namespace palo {


class StringVector : public SlimVector<char> {

public:

	class StringId {
		friend class StringVector;
		friend ostream& operator<<(ostream& output, const StringId &id);

	public:
		StringId() : page(NO_IDENTIFIER), offset(NO_IDENTIFIER) {}

		bool operator==(const StringId &id) const {
			return (page == id.page && offset == id.offset);
		}
		bool notSet() const {
			return (page == NO_IDENTIFIER || offset == NO_IDENTIFIER);
		}
	private:
		StringId(uint32_t page, uint32_t offset) : page(page), offset(offset) {}
		uint32_t getPage() const {return page;}
		uint32_t getOffset() const {return offset;}

		uint32_t page;
		uint32_t offset;
	};

	StringVector(uint32_t pageMemorySize = 4096);

	StringId push(const string& str);
	string getString(StringId index) const;
	string getString(const_iterator it) const;
	void erase(StringId id);
	void erase(const iterator &it);
	bool casecmp(StringId id1, StringId id2) const;
	void clear();

	virtual bool merge(const CPCommitable &o, const PCommitable &p);
	virtual PCommitable copy() const;

	static double decode(StringId id);
	static StringId encode(double d);
	bool validate(double d) const;

private:
	size_t filled;
	size_t erased;

	StringVector(const StringVector &v);
};


typedef boost::shared_ptr<StringVector> PStringVector;
typedef boost::shared_ptr<const StringVector> CPStringVector;


}

#endif /* STRINGVECTOR_H_ */
