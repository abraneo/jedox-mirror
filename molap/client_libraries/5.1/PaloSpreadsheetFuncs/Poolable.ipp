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

#include <boost/pool/singleton_pool.hpp>

#include <PaloSpreadsheetFuncs/Poolable.h>

namespace Palo {
	namespace Util {
		template<typename T>
		void* Poolable<T>::operator new(size_t size, const typename Poolable<T>::_Pool&) {
			typedef boost::singleton_pool<Poolable<T>, sizeof(T)> MemoryPool;

			void *p = MemoryPool::malloc();

			if (p == NULL) {
				throw std::bad_alloc();
			}

			return p;
		}

		template<typename T>
		void Poolable<T>::operator delete(void *p, const typename Poolable<T>::_Pool&) {
			typedef boost::singleton_pool<Poolable<T>, sizeof(T)> MemoryPool;
			
			if (p != NULL) {
				static_cast<T*>(p)->~T();
			
				MemoryPool::free(p);
			}
		}
		
		template<typename T>
		typename Poolable<T>::_Pool Poolable<T>::Pool;
	}
}
