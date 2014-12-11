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

#include "ZendValue.h"
#include "ZendArrayBuilderImpl.h"
#include "ZendValueException.h"

#include <zend_API.h>

// for php 5.3.0 and above
#if PHP_VERSION_ID >= 50300
#	define ZVAL_ADDREF(pz)	Z_ADDREF_P(pz)
#endif

using namespace Palo::PHPPalo;

void ZendArrayBuilderImpl::append(GenericCell& v)
{
	if (add_next_index_zval(p, ((ZendValue&)v).p) == FAILURE)
		throw ZendValueException(CurrentSourceLocation, ZendValueErrors::ERROR_ZEND_ADD_ARRAYENTRY);
	else
		ZVAL_ADDREF((( ZendValue& )v ).p);
}

void ZendArrayBuilderImpl::append(const std::string key, GenericCell& v)
{
	if (add_assoc_zval( p, ( char* )key.c_str(), (( ZendValue& )v ).p ) == FAILURE)
		throw ZendValueException(CurrentSourceLocation, ZendValueErrors::ERROR_ZEND_ADD_ARRAYENTRY);
	else
		ZVAL_ADDREF((( ZendValue& )v ).p);
}

std::unique_ptr<GenericCell> ZendArrayBuilderImpl::createGenericCell()
{
	std::unique_ptr < GenericCell > p;
	p.reset(new ZendValue(settings));
	return p;
}
