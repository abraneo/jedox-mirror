#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(libpalo_ng,5.1,bugs@jedox.com)
AC_CONFIG_SRCDIR([libpalo_ng.cpp])
#AC_CONFIG_HEADER([config.h])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign subdir-objects])

CXXFLAGS="$CXXFLAGS -std=c++0x"

AC_ARG_ENABLE(32bit,
  AS_HELP_STRING([--enable-32bit], [force 32bit compilation]),
  if test "$enableval" == "yes";  then
    CXXFLAGS="$CXXFLAGS -m32"
    CFLAGS="$CFLAGS -m32"
    LDFLAGS="$LDFLAGS -m32"
  fi
)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
#AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_LN_S
AC_PROG_MAKE_SET
LT_INIT
AM_PROG_CC_C_O

BOOST_REQUIRE([1.34.1])
BOOST_REGEX
BOOST_SYSTEM
BOOST_THREADS

ACX_PTHREAD
##check for openssl
CHECK_SSL

AC_CHECK_LIB([m], [pow])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h string.h sys/socket.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

AC_ARG_ENABLE(opt, AS_HELP_STRING([--enable-opt], [create optimized version]), CXXFLAGS  ="$CXXFLAGS   -O2" CFLAGS  ="$CFLAGS   -O2")
# Checks for library functions.
AC_FUNC_MEMCMP
AC_FUNC_SELECT_ARGTYPES
AC_CHECK_FUNCS([getaddrinfo inet_ntoa memset pow select socket strdup])

AC_DEFINE([__UNIX__])
AC_DEFINE([_MT])

AC_ARG_ENABLE([install-headers],[AC_HELP_STRING([--enable-install-headers],[Also install header files. [no]])],[headers=yes],[headers=no])
AM_CONDITIONAL([INSTALL_LIBPALO_HEADERS], [test x$headers = xyes])

AC_CONFIG_FILES([Makefile])

AC_PREFIX_DEFAULT(/usr)
AC_OUTPUT
