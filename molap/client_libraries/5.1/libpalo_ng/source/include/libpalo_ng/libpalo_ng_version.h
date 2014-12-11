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

#ifndef LIBPALO_NG_VERSION_H
#define LIBPALO_NG_VERSION_H

#include <libpalo_ng/config_ng.h>

struct VERSION_INFO {
	UINT major_version;
	UINT minor_version;
	UINT bugfix_version;
	UINT build_number;
};

#ifdef __cplusplus
extern "C"
#endif
struct VERSION_INFO LIBPALO_NG_EXPORT libpalo_ng_getversion(void);
#endif							 // LIBPALO_NG_VERSION_H
