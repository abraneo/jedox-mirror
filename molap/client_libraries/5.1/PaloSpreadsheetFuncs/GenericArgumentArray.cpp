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

#include <PaloSpreadsheetFuncs/GenericCellException.h>
#include <PaloSpreadsheetFuncs/WrongParamCountException.h>
#include <PaloSpreadsheetFuncs/GenericArgumentArray.h>

#include "SimpleCell.h"

using namespace Palo::SpreadsheetFuncs;
using namespace std;

GenericArgumentArray::GenericArgumentArray(std::vector<GenericCell*>& _args) :
		args(_args), connection_fixed(false)
{
}

GenericArgumentArray::~GenericArgumentArray()
{
}

GenericCell& GenericArgumentArray::operator [](size_t idx)
{
	if (idx >= args.size()) {
		throw WrongParamCountException(CurrentSourceLocation);
	} else {
		return *args[idx];
	}
}

size_t GenericArgumentArray::length() const
{
	return args.size();
}

void delete_and_set_null(GenericCell* & ptr)
{
	delete ptr;
	ptr = NULL;
}

void GenericArgumentArray::fixConnection(unsigned int idx)
{
	if (!connection_fixed && (*this)[idx].getType() == GenericCell::TString) {
		std::string connection_string = args[idx]->getString();
		string database_string;

		std::string::size_type i;
		if ((i = connection_string.find("/")) != std::string::npos) {
			database_string = connection_string.substr(i);
			connection_string.resize(i);
			if (database_string.length() <= 1)
				throw GenericCellException(CurrentSourceLocation, SpreadsheetFuncsErrors::ERROR_GENERICCELL_MISSING_DATABASE);
			else
				database_string = database_string.substr(1);

			connection_string = connection_string.substr(0, i);
		}

		std::unique_ptr < GenericCell > tmp_connection = args[idx]->create();
		tmp_connection->set(connection_string);
		//this position will be overwritten, so to avoid memory leaks we need to delete it here
		delete_and_set_null(args[idx]);
		// insert the newly created pointer
		// all the ptrs inside args will be deleted externally
		args[idx] = tmp_connection.release();

		if (i != std::string::npos) {
			std::unique_ptr < GenericCell > tmp_database = args[idx]->create();
			tmp_database->set(database_string);
			// insert the newly created pointer
			// all the ptrs inside args will be deleted externally
			args.insert(args.begin() + (idx + 1), tmp_database.release());
		}

		connection_fixed = true;
	}
}

void GenericArgumentArray::checkArgCount(unsigned int num_args)
{
	if (args.size() != num_args)
		throw WrongParamCountException(CurrentSourceLocation);
}

void GenericArgumentArray::lshift(unsigned int offset)
{
	size_t len = args.size();

	if (len > 1) {
		for (size_t j = 0; j < offset; j++) {
			// simple shift
			GenericCell* t = args[0];
			for (size_t i = 0; i < len - 1; i++)
				args[i] = args[i + 1];
			args[len - 1] = t;
		}
	}
}

void GenericArgumentArray::rshift(unsigned int offset)
{
	size_t len = args.size();

	if (len > 1) {
		for (size_t j = 0; j < offset; j++) {
			// simple shift
			GenericCell* t = args[len - 1];
			for (size_t i = 0; i < len - 1; i++)
				args[i + 1] = args[i];
			args[0] = t;
		}
	}
}

void GenericArgumentArray::insert(unsigned int idx, std::string val)
{
	if (!args.empty()) {
		std::unique_ptr < GenericCell > tmp = args[0]->create();
		tmp->set(val);
		args.insert(args.begin() + idx, tmp.release());
	}
}

void GenericArgumentArray::collapseToArray(unsigned int from_idx)
{
	if (args.size() < from_idx) {
		throw WrongParamCountException(CurrentSourceLocation);
	} else {
		unique_ptr < SimpleCell::ArrayCell > p;
		p.reset(new SimpleCell::ArrayCell(args.size() - from_idx));

		for (unsigned int i = from_idx; i < args.size(); i++) {
			p->append(args[i]);
			// after passing the pointer to the boost::ptr_vector (which is done by appending it)
			// we are no longer responsible for deleting it, so to avoid double frees we set it to NULL here!
			args[i] = NULL;
		}

		args.resize(from_idx + 1);
		args[from_idx] = p.release();
	}
}
