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

#ifndef ZEND_ARRAY_BUILDER_IMPL_H
#define ZEND_ARRAY_BUILDER_IMPL_H

#include <zend.h>

#include <PaloSpreadsheetFuncs/GenericContext.h>
#include <PaloSpreadsheetFuncs/GenericArrayBuilder.h>

namespace Palo {
namespace PHPPalo {
using namespace SpreadsheetFuncs;

class ZendArrayBuilderImpl : public GenericArrayBuilderImpl {
private:
	zval *p;
	GenericContext& settings;

public:
	ZendArrayBuilderImpl(GenericContext& settings, zval *z) :
			p(z), settings(settings)
	{
	}

	~ZendArrayBuilderImpl()
	{
	}

	void append(GenericCell& v);
	void append(const std::string key, GenericCell& v);

	std::unique_ptr<GenericCell> createGenericCell();
};
}
}
#endif
