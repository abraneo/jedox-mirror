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

#ifndef HAS_LIBPALO_H
#define HAS_LIBPALO_H

#include <wchar.h>

#include "const.h"
#include "error.h"
#include "errstack.h"

#ifdef __cplusplus

#include <libpalo_ng/Palo/types.h>
#include <libpalo_ng/Palo/Server.h>

using namespace jedox::palo;
#endif

#define LIBEXPORT

typedef struct libpalo_result_struct {
	palo_bool succeeded;
}

libpalo_result;

#define LIBPALO_SUCCESSFUL(libpalo_res) ((libpalo_res).succeeded)

typedef struct libpalo_err_struct {
	libpalo_result result;
	palo_err palo_error;
	long original_error;
	struct errstack errs;
}

libpalo_err;

#include "conversions.h"

extern struct conversions libpalo_convs;

struct sock_obj {

#if defined(WIN32) || defined(WIN64)
	SOCKET socket;
#else
	int socket;
#endif

	char *hostname;
	unsigned int port;
	char *username;
	char *pw;
	char *key;
	int *version;

#ifdef __cplusplus
	Server *myServer;
#else
	void *myServer;
#endif

};

#ifdef __cplusplus
extern "C" {
#endif

#include "interface.h"

	/** @ingroup gen_funcs_mt
	 *  @brief
	 *  Initializes a conversion object for future calls to *_r() functions
	 *
	 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free()
	 *  @param convs pointer to a memory location which is at least sizoef(struct conversions) long
	 *
	 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
	 */
	LIBEXPORT libpalo_result libpalo_init_r(libpalo_err *err, struct conversions *convs);

	/** @ingroup gen_funcs_mt
	 *  @brief
	 *  Releases any data associated with the conversion object. The memory pointed to by convs is not free'd.
	 *
	 *  @param convs pointer to a initialized struct conversions
	 */
	LIBEXPORT void libpalo_cleanup_r(struct conversions *convs);

	/** @ingroup gen_funcs_st
	 *  @brief
	 *  Initializes an internal conversion object for future calls to not *_r() functions which are not multithread-safe.
	 *
	 *  @param err will contain error information if the function fails; it should be free'd using libpalo_err_free()
	 *
	 *  @return check whether execution was successful using the macro LIBPALO_SUCCESSFUL(libpalo_result)
	 */
	LIBEXPORT libpalo_result libpalo_init(libpalo_err *err);

	/** @ingroup gen_funcs_st
	 *  @brief
	 *  Releases any data associated with the internal conversion object.
	 *
	 *  ...
	 */
	LIBEXPORT void libpalo_cleanup();

	/** @ingroup gen_funcs
	 *  @brief
	 *  Closes a connection. The memory pointed to by so will not be free'd.
	 *
	 *  @param so pointer to a struct sock_obj initialized by a preceding call to e.g. palo_connect().
	 */
	LIBEXPORT void palo_disconnect(struct sock_obj *so);

	LIBEXPORT void palo_disconnect2(struct sock_obj *so, unsigned short logout);

	LIBEXPORT unsigned short palo_is_same_connection(struct sock_obj *so, struct sock_obj *so2);

	/** @ingroup gen_funcs
	 *  @brief
	 *  Free's resources allocated by previous calls to libpalo functions.
	 *
	 * @param p pointer to the memory that should be free'd.
	 */
	LIBEXPORT void libpalo_free(void *p);

	/** @ingroup gen_funcs
	 *  @brief
	 *  Free's error information returned by library functions.
	 *
	 * @param err pointer to the error descriptor
	 */
	LIBEXPORT void libpalo_err_free_contents(libpalo_err *err);

#ifdef __cplusplus
}
#endif
#endif
