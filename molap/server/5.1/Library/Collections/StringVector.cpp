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

#include "Collections/StringVector.h"
#include "Collections/StringUtils.h"
#include "Logger/Logger.h"


namespace palo {

StringVector::StringVector(uint32_t pageMemorySize) : SlimVector<char>(pageMemorySize), filled(0), erased(0)
{
}


StringVector::StringVector(const StringVector &v) : SlimVector<char>(v), filled(v.filled), erased(v.erased)
{
}


StringVector::StringId StringVector::push(const string &str)
{
	checkCheckedOut();

	Slim<char>::PSlimPage lastPage;
	if (!pages.empty()) {
		lastPage = pages[pages.size() - 1];
		if (lastPage->full()) {
			lastPage = addPage();
		} else {
			if (!lastPage->isCheckedOut()) {
				pages[pages.size() - 1] = lastPage = lastPage->getCopy();
			}
		}
	} else {
		lastPage = addPage();
	}

	StringId id((uint32_t)pages.size() - 1, lastPage->count());

	for (size_t i = 0; i < str.size(); i++) {
		if (lastPage->full()) {
			lastPage = addPage();
		}
		lastPage->push_back(str[i]);
	}
	if (lastPage->full()) {
		lastPage = addPage();
	}
	lastPage->push_back('\0');
	filled++;

	return id;
}


string StringVector::getString(StringId id) const
{
	return getString(const_iterator(this, id.page, id.offset));
}


string StringVector::getString(const_iterator it) const
{
	const_iterator backup = it;
	size_t size = 1;
	while (*it != '\0') {
		++it;
		size++;
	}

	char *str = new char[size];
	for (size_t i = 0; i < size; ++i, ++backup) {
		str[i] = *backup;
	}

	string s(str);
	delete[] str;
	return s;
}


void StringVector::erase(StringId id)
{
	// nothing to do
	erased++;
}


void StringVector::erase(const iterator &it)
{
	// nothing to do
	erased++;
}


ostream& operator<<(ostream& output, const StringVector::StringId &id)
{
	output << '\"' << id.page << ":" << id.offset << '\"';
	return output;
}


bool StringVector::casecmp(StringId id1, StringId id2) const
{
	string s1 = getString(id1);
	string s2 = getString(id2);
	return UTF8Comparer::compare(s1.c_str(), s2.c_str()) < 0;
}


void StringVector::clear()
{
	filled = 0;
	erased = 0;
	SlimVector<char>::clear();
}


bool StringVector::merge(const CPCommitable &o, const PCommitable &p)
{
	return SlimVector<char>::merge(o, p);
}


PCommitable StringVector::copy() const
{
	checkNotCheckedOut();
	PStringVector newVector(new StringVector(*this));
	return newVector;
}

double StringVector::decode(StringId id)
{
	double d;
	uint8_t *p = (uint8_t *)&d;
	uint32_t i = id.getPage();
	memcpy(p, &i, sizeof(uint32_t));
	p += sizeof(uint32_t);
	i = id.getOffset();
	memcpy(p, &i, sizeof(uint32_t));
	return d;
}

StringVector::StringId StringVector::encode(double d)
{
	uint32_t page;
	uint32_t offset;
	uint8_t *p = (uint8_t *)&d;
	memcpy(&page, p, sizeof(uint32_t));
	p += sizeof(uint32_t);
	memcpy(&offset, p, sizeof(uint32_t));
	return StringId(page, offset);
}

bool StringVector::validate(double d) const
{
	StringId id = encode(d);
	bool ret = true;
	if (id.page >= pages.size()) {
		Logger::error << id.page << "," << id.offset << " " << pages.size() << endl;
		ret = false;
	} else {
		if (id.offset >= pages[id.page]->count()) {
			Logger::error << id.page << "," << id.offset << " " << pages.size() << "," << pages[id.page]->count() << endl;
			ret = false;
		}
	}
	return ret;
}


}
