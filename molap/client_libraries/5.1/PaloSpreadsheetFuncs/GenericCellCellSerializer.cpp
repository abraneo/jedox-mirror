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
 * 
 *
 */

#include "GenericCellCellSerializer.h"

#include "SimpleCell.h"

using namespace Palo::SpreadsheetFuncs;

GenericCellCellSerializer::GenericCellCellSerializer(GenericCell& gc) :
		a(gc.setArray(0, false))
{
}

GenericCellCellSerializer::~GenericCellCellSerializer()
{
}

void GenericCellCellSerializer::put(GenericCell& c)
{
	put("", c);
}

void GenericCellCellSerializer::put(const std::string& name, GenericCell& c)
{
	a.append(a.createGenericCell()->set(name));
	if (c.getType() == GenericCell::TArray) {
		a.append(a.createGenericCell()->setNull());
		for (GenericCell::Iterator in = c.getArray(); !in.end(); ++in)
			put(*in);
		a.append(a.createGenericCell()->setNull());
	} else {
		a.append(c);
	}
}

void GenericCellCellSerializer::putUnset(const std::string& name)
{
	put("UNSET|" + name, a.createGenericCell()->setNull());
}

std::unique_ptr<GenericCell> GenericCellCellSerializer::createGenericCell()
{
	std::unique_ptr < GenericCell > c = a.createGenericCell();
	c->supressPadding();
	return c;
}

void GenericCellCellSerializer::putStringArrayArray(const std::string& name, const StringArrayArray& sa)
{
	put(name + "|Count", createGenericCell()->set(sa.size()));
	for (StringArrayArray::const_iterator i = sa.begin(); i != sa.end(); ++i)
		put(name + "|Subarray", createGenericCell()->set(*i));
}

GenericCellCellDeserializer::GenericCellCellDeserializer(GenericCell& gc) :
		iter(gc.getArray()), got(false), inc(false)
{
}

GenericCellCellDeserializer::~GenericCellCellDeserializer()
{
}

GenericCell& GenericCellCellDeserializer::get(std::string& name, bool release, bool in_array)
{
	std::unique_ptr < GenericCell > p;

	if (got) {
		got = false;
		inc = true;
		if (temp.get() != NULL)
			return release ? *temp.release() : *temp.get();
		else
			return release ? *iter.release() : *iter;
	} else {
		if (inc) {
			++iter;
			inc = false;
		}

		name = iter->getString();
		if (name.substr(0, 6) == "UNSET|") {
			name = name.substr(6);
			p.reset(new SimpleCell::MissingCell());
			// skip following empty cell
			++iter;
			inc = true;
		} else {
			++iter;
			if (iter->getType() == GenericCell::TNull) {
				if (!in_array) {
					p.reset(new SimpleCell::ArrayCell());
					std::string dummy;
					while ((++iter)->getType() != GenericCell::TNull) {
						inc = false;
						static_cast<SimpleCell::ArrayCell*>(p.get())->append(&get(dummy, true, true));
					}
					inc = true;
				}
			} else {
				inc = true;
			}
		}

		if (p.get() == NULL) {
			if (!release)
				temp.reset();
			return release ? *iter.release() : *iter;
		} else {
			if (!release) {
				temp.reset(p.release());
				return *temp;
			} else {
				return *p.release();
			}
		}
	}
}

bool GenericCellCellDeserializer::isUnset(std::string& name)
{
	got = false;
	get(name);
	got = true;
	if (temp.get() == NULL) {
		return iter->isMissing() || iter->getType() == GenericCell::TNull;
	} else {
		return temp->isMissing() || temp->getType() == GenericCell::TNull;
	}
}

StringArrayArray GenericCellCellDeserializer::getStringArrayArray(std::string& name)
{
	StringArrayArray sa;
	unsigned int count;

	count = get(name).getUInt();

	sa.reserve(count);

	for (unsigned int i = 0; i < count; i++)
		sa.push_back(get(name).getStringArray());

	return sa;
}
