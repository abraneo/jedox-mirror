////////////////////////////////////////////////////////////////////////////////
/// @brief
///
/// @file
///
/// Copyright (C) 2006-2014 Jedox AG
///
/// This program is free software; you can redistribute it and/or modify it
/// under the terms of the GNU General Public License (Version 2) as published
/// by the Free Software Foundation at http://www.gnu.org/copyleft/gpl.html.
///
/// This program is distributed in the hope that it will be useful, but WITHOUT
/// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
/// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
/// more details.
///
/// You should have received a copy of the GNU General Public License along with
/// this program; if not, write to the Free Software Foundation, Inc., 59 Temple
/// Place, Suite 330, Boston, MA 02111-1307 USA
///
/// If you are developing and distributing open source applications under the
/// GPL License, then you are free to use Palo under the GPL License.  For OEMs,
/// ISVs, and VARs who distribute Palo with their products, and do not license
/// and distribute their source code under the GPL, Jedox provides a flexible
/// OEM Commercial License.
///
/// @author
////////////////////////////////////////////////////////////////////////////////

#if defined(WIN32) || defined(WIN64)
#pragma warning(disable : 4996)
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>

#include "../JedoxXlHelper/JedoxXLHelper.h"

#include "libpalo2.h"
#include "strtools.h"
#include "stdaliases.h"
#include "error.h"
#include "helper.h"


LIBEXPORT palo_err _palo_connect_a(struct errstack *errs, const char *hostname, const char *service, struct sock_obj *so) {
	assert(hostname != NULL);
	assert(service != NULL);
	assert(so != NULL);
	assert(errs != NULL);

	initso(so);

	so->hostname = strdup(hostname);

	if (so->hostname == NULL) {
		ERRSTACK_RETURN(errs, PALO_ERR_NO_MEM, __FUNCTION__, __LINE__, __FILE__, MSG_PALO_ERR_NO_MEM);
	}

	so->port = atoi(service);

	// the connection is made in PALO_SUFFIX(auth)

	return PALO_SUCCESS;
}

LIBEXPORT void palo_disconnect2(struct sock_obj *so, unsigned short logout) {

	if (so->myServer != NULL) {
		try {
			JedoxXLHelper::getInstance().removeServer(so->key, logout != 0);
		} catch (const std::exception&) {
#ifndef _DEBUG
		} catch (...) {
#endif
		}
	}

	free(so->hostname);
	so->hostname = NULL;

	free(so->username);
	so->username = NULL;

	free(so->pw);
	so->pw = NULL;

	free(so->key);
	so->key = NULL;

	free(so->version);
	so->version = NULL;

	// don't free so->myServer. this is done in the helper libpalo_ng
	so->myServer = NULL;
}

LIBEXPORT void palo_disconnect(struct sock_obj *so) {
	palo_disconnect2(so, 1);
}


LIBEXPORT unsigned short palo_is_same_connection(struct sock_obj *so, struct sock_obj *so2) {
	return (stricmp(so->key, so2->key) == 0) ? 1 : 0;
}
