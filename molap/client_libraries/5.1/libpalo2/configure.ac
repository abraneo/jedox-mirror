#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#TODO:: switches for debug etc

AC_PREREQ(2.60)
AC_INIT(libpalo2, 5.1, bugs@jedox.com)
AC_CONFIG_SRCDIR([conversions.h])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR(config)

#produce only a shared library 
AC_DISABLE_STATIC
#initialize automake
AM_INIT_AUTOMAKE(libpalo2,5.1)
# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

# Checks for libraries.
# TODO: Probe for libpalo_ng -- how to check for classes ?
#AC_CHECK_LIB([palo_ng],,,AC_MSG_ERROR(["No libpalo_ng"])

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([langinfo.h limits.h malloc.h memory.h netinet/in.h stddef.h stdlib.h string.h unistd.h wchar.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([mbrlen memmove memset nl_langinfo strdup strrchr])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
AC_PREFIX_DEFAULT(/usr)
