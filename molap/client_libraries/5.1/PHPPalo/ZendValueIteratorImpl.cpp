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
#include "ZendValueIteratorImpl.h"
#include "ZendValueException.h"

using namespace Palo::PHPPalo;

#if PHP_MAJOR_VERSION < 5
// see zend_hash.h

#define zend_hash_has_more_elements_ex(ht, pos) \
	(zend_hash_get_current_key_type_ex(ht, pos) == HASH_KEY_NON_EXISTANT ? FAILURE : SUCCESS)
#endif

ZendValueIteratorImpl::ZendValueIteratorImpl(GenericContext& settings, ZendValue* v) :
		z(v->p), zdata(NULL), pos(0), int_pos(0), settings(settings), endReached(false)
{
	zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P( z ), &pos);

	getCurrentData();
}

ZendValueIteratorImpl::~ZendValueIteratorImpl()
{
}

void ZendValueIteratorImpl::getCurrentData()
{
	data.reset();

	if (zend_hash_has_more_elements_ex( Z_ARRVAL_P( z ), &pos ) == FAILURE) {
		endReached = true;
	} else {
		if (zend_hash_get_current_data_ex(Z_ARRVAL_P( z ), (void**)&zdata, &pos) == FAILURE) {
			throw ZendValueException(CurrentSourceLocation, ZendValueErrors::ERROR_ZEND_ZEND_HASH);
		}
	}
}

GenericCellIteratorImpl& ZendValueIteratorImpl::operator ++()
{
	if (end())
		throw ZendValueException(CurrentSourceLocation, ZendValueErrors::ERROR_ZENDVALUE_END_REACHED);

	if (zend_hash_move_forward_ex(Z_ARRVAL_P( z ), &pos) == FAILURE)
		throw ZendValueException(CurrentSourceLocation, ZendValueErrors::ERROR_ZEND_ZEND_HASH);

	getCurrentData();

	int_pos++;

	return *this;
}

GenericCell* ZendValueIteratorImpl::get(bool release)
{
	if (end())
		throw ZendValueException(CurrentSourceLocation, ZendValueErrors::ERROR_ZENDVALUE_END_REACHED);

	if (data.get() == NULL)
		data.reset(new ZendValue(settings, *zdata));

	return release ? data.release() : data.get();
}

GenericCell* ZendValueIteratorImpl::operator ->()
{
	return get();
}

GenericCell* ZendValueIteratorImpl::release()
{
	return get(true);
}

bool ZendValueIteratorImpl::end() const
{
	return endReached;
}

size_t ZendValueIteratorImpl::minRemaining() const
{
	return zend_hash_num_elements(Z_ARRVAL_P( z )) - int_pos;
}
