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

#include <libpalo_ng/libpalo_ng_version.h>

#define PALO_NG_MAJOR_VERSION   5
#define PALO_NG_MINOR_VERSION   1
#define PALO_NG_BUILD_NUMBER    2279

LIBPALO_NG_EXPORT struct VERSION_INFO libpalo_ng_getversion(void)
{
	struct VERSION_INFO version_info;

	version_info.major_version = PALO_NG_MAJOR_VERSION;
	version_info.minor_version = PALO_NG_MINOR_VERSION;
	version_info.bugfix_version = (sizeof(void*) == 8 ? 4 : 2);
	version_info.build_number = PALO_NG_BUILD_NUMBER;

	return version_info;
}
