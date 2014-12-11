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
 * \author Zurab Khadikov, Jedox AG, Freiburg, Germany
 * 
 *
 */

/* enables https interface */
#cmakedefine ENABLE_HTTPS 1

/* enables https interface as shared library */
#cmakedefine ENABLE_HTTPS_MODULE 1

/* enables gpu server */
#cmakedefine ENABLE_GPU_SERVER 1

/* enables support for option --trace */
#cmakedefine ENABLE_TRACE_OPTION

/* Name of package */
#define PACKAGE "@PACKAGE@"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "@PACKAGE_BUGREPORT@"

/* Define to the full name of this package. */
#define PACKAGE_NAME "@PACKAGE_NAME@"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "@PACKAGE_STRING@"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "@PACKAGE_TARNAME@"

/* Define to the version of this package. */
#define PACKAGE_VERSION "@PACKAGE_VERSION@"

/* Define to the home page for this package. */
#define PACKAGE_URL "@PACKAGE_URL@"

/* Version number of package */
#define VERSION "@PACKAGE_VERSION@"

/* Enables mode for testing, e.g. Timer */
#cmakedefine ENABLE_TEST_MODE


