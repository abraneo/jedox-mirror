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

#ifndef ZEND_VALUE_MESSAGES_H
#define ZEND_VALUE_MESSAGES_H

#include <PaloSpreadsheetFuncs/Messages.h>

namespace Palo {
namespace Types {
class ZendValueErrors {
public:
	enum Errors {
		ERROR_ZEND_ADD_ARRAYENTRY, ERROR_ZEND_ZEND_HASH, ERROR_ZENDVALUE_INTERNAL_ERROR, ERROR_ZENDVALUE_INVALID_CONNECTION, ERROR_ZENDVALUE_END_REACHED, ERROR_VALUE_INT_WOULD_TRUNCATE
	};
};

class ZendValueMessages : public Palo::Util::Messages<ZendValueErrors::Errors> {
private:
	ZendValueMessages();

public:
	static ZendValueMessages& getInstance()
	{
		static ZendValueMessages m;
		return m;
	}
};
}
}
#endif
