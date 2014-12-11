/* 
 *
 * Copyright (C) 2006-2014 Jedox AG
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 *
 *
 */

#include "BasicAuthentication.h"

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>

namespace jedox {
namespace palo {

BasicAuthentication::BasicAuthentication(const std::string& username, const std::string& password)
{
	std::string key = username + ":" + password;

	BIO *bmem, *b64;
	BUF_MEM *bptr;
	b64 = BIO_new(BIO_f_base64());
	bmem = BIO_new(BIO_s_mem());
	b64 = BIO_push(b64, bmem);
	BIO_write(b64, key.c_str(), (int)key.size());
	(void)BIO_flush(b64);
	BIO_get_mem_ptr(b64, &bptr);
	m_Header.append("Authorization: Basic ").append(std::string(bptr->data, bptr->length)).append("\r\n");
	BIO_free_all(b64);
}

const std::string& BasicAuthentication::getHeader() const
{
	return m_Header;
}

} /* palo */
} /* jedox */
