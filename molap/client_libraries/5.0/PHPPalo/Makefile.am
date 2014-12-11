################################################################################
###
### Copyright (C) 2006-2013 Jedox AG
###
### This program is free software; you can redistribute it and/or modify it
### under the terms of the GNU General Public License (Version 2) as published
### by the Free Software Foundation at http://www.gnu.org/copyleft/gpl.html.
###
### This program is distributed in the hope that it will be useful, but WITHOUT
### ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
### FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
### more details.
###
### You should have received a copy of the GNU General Public License along with
### this program; if not, write to the Free Software Foundation, Inc., 59 Temple
### Place, Suite 330, Boston, MA 02111-1307 USA
###
### If you are developing and distributing open source applications under the
### GPL License, then you are free to use Palo under the GPL License.  For OEMs,
### ISVs, and VARs who distribute Palo with their products, and do not license
### and distribute their source code under the GPL, Jedox provides a flexible
### OEM Commercial License.
###
### \author Oliver Kania
###
###
################################################################################

AUTOMAKE_OPTIONS = foreign

#add defines (is there a better way ?) 

INCLUDES = -I@PHPHEADERS@/main -I@PHPHEADERS@/Zend \
	   -I@PHPHEADERS@/TSRM -I${top_srcdir}/../PaloSpreadsheetFuncs/include \
           -I@PHPHEADERS@ -I${top_srcdir}/../libpalo_ng/source/include -I${top_srcdir}/../libpalo_ng/include

#compilation flags
ACLOCAL_AMFLAGS = -I config
AM_CPPFLAGS  = "-D__UNIX__" "-DPHP_PALO_EXPORTS" \
"-DCOMPILE_DL_PHP_PALO" "-DPTHREADS" -rdynamic
AM_CFLAGS =${AM_CPPFLAGS}

#use lpthread 
AM_LDFLAGS  = -lpthread -licuuc

#generate shared library 
lib_LTLIBRARIES    = libphp_jedox_palo.la

libphp_jedox_palo_la_SOURCES = \
php_jedox_palo.c \
PHPPaloLogger.cpp \
PHPPaloVersion.cpp \
php_jedox_palo_cpp.cpp \
c2c_plus_plus.cpp \
PHPPaloContext.cpp \
PHPPaloSpreadsheetFuncs.cpp \
PHPPaloSpreadsheetFuncsWrapper.cpp \
wrap.cpp \
ZendArrayBuilderImpl.cpp \
ZendValueIteratorImpl.cpp \
ZendValue.cpp \
ZendValueMessages.cpp \
PHPPaloMessages.cpp \
../PaloSpreadsheetFuncs/Converter.cpp \
../PaloSpreadsheetFuncs/Conversion.cpp \
../PaloSpreadsheetFuncs/SpreadsheetFuncsBase.cpp \
../PaloSpreadsheetFuncs/GenericCellCellSerializer.cpp \
../PaloSpreadsheetFuncs/SpreadsheetFuncsMessages.cpp \
../PaloSpreadsheetFuncs/GenericArgumentArray.cpp \
../PaloSpreadsheetFuncs/Wrapper.cpp \
../PaloSpreadsheetFuncs/QueryCache.cpp \
../PaloSpreadsheetFuncs/ElementListArray.cpp \
../PaloSpreadsheetFuncs/StringTokenizer.cpp \
../PaloSpreadsheetFuncs/CellValue.cpp \
../PaloSpreadsheetFuncs/XLError.cpp \
../PaloSpreadsheetFuncs/GenericCell.cpp \
../PaloSpreadsheetFuncs/ConvertingCell.cpp \
../PaloSpreadsheetFuncs/SpreadsheetFuncs.cpp \
../PaloSpreadsheetFuncs/SimpleCell.cpp

# link (dynamically, if possible) with ibpalo_ng
libphp_jedox_palo_la_LIBADD = \
  -lpalo_ng
