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

#include <PaloSpreadsheetFuncs/InvalidRequestException.h>

#include "SimpleCell.h"

using namespace Palo::SpreadsheetFuncs::SimpleCell;
using namespace Palo::SpreadsheetFuncs;
using namespace Palo::Types;

DummyCell::~DummyCell()
{
}

bool DummyCell::isMissing()
{
	throw InvalidRequestException(CurrentSourceLocation);
}

bool DummyCell::empty()
{
	throw InvalidRequestException(CurrentSourceLocation);
}

GenericCell& DummyCell::setEmpty()
{
	throw InvalidRequestException(CurrentSourceLocation);
}

GenericCell::Iterator DummyCell::getArray()
{
	throw InvalidRequestException(CurrentSourceLocation);
}

boost::shared_ptr<jedox::palo::Server> DummyCell::getConnection(std::string& database)
{
	throw InvalidRequestException(CurrentSourceLocation);
}

std::string DummyCell::getString()
{
	throw InvalidRequestException(CurrentSourceLocation);
}

unsigned int DummyCell::getUInt()
{
	throw InvalidRequestException(CurrentSourceLocation);
}

int DummyCell::getSInt()
{
	throw InvalidRequestException(CurrentSourceLocation);
}

unsigned long int DummyCell::getULong()
{
	throw InvalidRequestException(CurrentSourceLocation);
}

long int DummyCell::getSLong()
{
	throw InvalidRequestException(CurrentSourceLocation);
}

bool DummyCell::getBool()
{
	throw InvalidRequestException(CurrentSourceLocation);
}

double DummyCell::getDouble()
{
	throw InvalidRequestException(CurrentSourceLocation);
}

GenericCell& DummyCell::set(boost::shared_ptr<jedox::palo::Server> s)
{
	throw InvalidRequestException(CurrentSourceLocation);
}

GenericCell& DummyCell::set(int i)
{
	throw InvalidRequestException(CurrentSourceLocation);
}

GenericCell& DummyCell::set(unsigned int i)
{
	throw InvalidRequestException(CurrentSourceLocation);
}

GenericCell& DummyCell::set(long int i)
{
	throw InvalidRequestException(CurrentSourceLocation);
}

GenericCell& DummyCell::set(unsigned long int i)
{
	throw InvalidRequestException(CurrentSourceLocation);
}

GenericCell& DummyCell::set(double d)
{
	throw InvalidRequestException(CurrentSourceLocation);
}

GenericCell& DummyCell::set(long double d)
{
	throw InvalidRequestException(CurrentSourceLocation);
}

GenericCell& DummyCell::set(const std::string& s)
{
	throw InvalidRequestException(CurrentSourceLocation);
}

GenericCell& DummyCell::set(bool b)
{
	throw InvalidRequestException(CurrentSourceLocation);
}

GenericCell& DummyCell::set(const DimensionElementInfo& ei)
{
	throw InvalidRequestException(CurrentSourceLocation);
}

GenericCell& DummyCell::setError(const ErrorInfo& ei, bool set_error_desc)
{
	throw InvalidRequestException(CurrentSourceLocation);
}

GenericCell& DummyCell::setNull()
{
	throw InvalidRequestException(CurrentSourceLocation);
}

GenericCell::ArrayBuilder DummyCell::setArray(size_t length, bool pad)
{
	throw InvalidRequestException(CurrentSourceLocation);
}

StringCell::StringCell(const std::string& s) :
		s(s)
{
}

StringCell::~StringCell()
{
}

std::string StringCell::getString()
{
	return s;
}

bool StringCell::empty(bool dummy)
{
	return s.empty();
}

GenericCell::Type StringCell::getType()
{
	return GenericCell::TString;
}

ConnectionCell::ConnectionCell(boost::shared_ptr<jedox::palo::Server> s) :
		s(s)
{
}

ConnectionCell::~ConnectionCell()
{
}

boost::shared_ptr<jedox::palo::Server> ConnectionCell::getConnection(std::string& database)
{
	return s;
}

GenericCell::Type ConnectionCell::getType()
{
	return GenericCell::TConnection;
}

ArrayCell::ArrayCell(size_t reserved_size)
{
	a.reserve(reserved_size);
}

ArrayCell::~ArrayCell()
{
}

GenericCell::Iterator ArrayCell::getArray()
{
	return GenericCellIterator(new ArrayCellIteratorImpl(*this));
}

GenericCell::Type ArrayCell::getType()
{
	return GenericCell::TArray;
}

bool ArrayCell::empty(bool allements)
{
	for (Iterator i = getArray(); !i.end(); ++i) {
		if (i->empty(allements))
			return true;
	}

	return false;
}

void ArrayCell::append(GenericCell* gc)
{
	a.push_back(gc);
}

ArrayCellIteratorImpl::ArrayCellIteratorImpl(ArrayCell& a) :
		a(a), pos(0)
{
}

ArrayCellIteratorImpl::~ArrayCellIteratorImpl()
{
}

GenericCell* ArrayCellIteratorImpl::release()
{
	throw InvalidRequestException(CurrentSourceLocation);
}

GenericCell* ArrayCellIteratorImpl::operator ->()
{
	return &(a.a[pos]);
}

GenericCellIteratorImpl& ArrayCellIteratorImpl::operator ++()
{
	pos++;
	return *this;
}

bool ArrayCellIteratorImpl::end() const
{
	return pos >= a.a.size();
}

size_t ArrayCellIteratorImpl::minRemaining() const
{
	return a.a.size() - pos;
}

MissingCell::MissingCell()
{
}

MissingCell::~MissingCell()
{
}

bool MissingCell::empty(bool dummy)
{
	return true;
}

bool MissingCell::isMissing()
{
	return true;
}

GenericCell::Type MissingCell::getType()
{
	return GenericCell::TNull;
}

bool ArrayCell::isMissing()
{
	return false;
}

std::unique_ptr<GenericCell> SimpleCell::ConnectionCell::clone() const
{
	return std::unique_ptr < GenericCell > (new ConnectionCell(*this));
}

bool SimpleCell::ConnectionCell::empty(bool dummy /*= true*/)
{
	return s == 0;
}

std::unique_ptr<GenericCell> SimpleCell::ConnectionCell::create() const
{
	throw InvalidRequestException(CurrentSourceLocation);
}

std::unique_ptr<GenericCell> SimpleCell::MissingCell::clone() const
{
	return std::unique_ptr < GenericCell > (new MissingCell());
}

std::unique_ptr<GenericCell> SimpleCell::MissingCell::create() const
{
	return std::unique_ptr < GenericCell > (new MissingCell());
}

std::unique_ptr<GenericCell> SimpleCell::StringCell::clone() const
{
	return std::unique_ptr < GenericCell > (new StringCell(*this));
}

std::unique_ptr<GenericCell> SimpleCell::StringCell::create() const
{
	throw InvalidRequestException(CurrentSourceLocation);
}

std::unique_ptr<GenericCell> SimpleCell::ArrayCell::create() const
{
	throw InvalidRequestException(CurrentSourceLocation);
}

std::unique_ptr<GenericCell> SimpleCell::ArrayCell::clone() const
{
	return std::unique_ptr < GenericCell > (new ArrayCell(*this));
}
