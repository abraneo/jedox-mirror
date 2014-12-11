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

#ifndef LIBPALO_NG_CONFIG_NG_H
#define LIBPALO_NG_CONFIG_NG_H

// #define USE_SINGLE_THREADED_CONNECTION_POOL 1

typedef unsigned long ULONG;
typedef unsigned int UINT;
typedef unsigned short USHORT;
typedef ULONG DWORD;
typedef USHORT PORT;
#if defined( WIN32) && ! defined( WIN64 )
typedef UINT SOCKET;
#elif WIN64
typedef unsigned __int64 SOCKET;
#elif __UNIX__
typedef int SOCKET;
#else
#error No System definition found
#endif

typedef ULONG IP;

// MD5

/* PROTOTYPES should be set to one if and only if the compiler supports
 function argument prototyping.
 The following makes PROTOTYPES default to 0 if it has not already
 been defined with C compiler flags.
 */

#ifndef PROTOTYPES
#define PROTOTYPES 1
#endif

/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;

/* UINT2 defines a two byte word */
typedef unsigned short int UINT2;

/* UINT4 defines a four byte word */
typedef unsigned long int UINT4;

/* PROTO_LIST is defined depending on how PROTOTYPES is defined above.
 If using PROTOTYPES, then PROTO_LIST returns the list, otherwise it
 returns an empty list.
 */
#if PROTOTYPES
#define PROTO_LIST(list) list
#else
#define PROTO_LIST(list) ()
#endif

#if defined(WIN32) || defined(WIN64)
#if defined(LIBPALO_NG)
#   define LIBPALO_NG_CLASS_EXPORT __declspec(dllexport)
#else
#   define LIBPALO_NG_CLASS_EXPORT __declspec(dllimport)
#endif
#else
#   define LIBPALO_NG_CLASS_EXPORT
#endif

#define LIBPALO_NG_EXPORT LIBPALO_NG_CLASS_EXPORT

#define PRECISION               18

#endif							 // LIBPALO_NG_CONFIG_NG_H
