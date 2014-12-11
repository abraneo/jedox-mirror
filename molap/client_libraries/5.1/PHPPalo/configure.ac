#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.60)
AC_INIT(PHPPalo,5.1,bugs@jedox.com)
#Explicit Configuration Directory (default is ../../)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR(config)
AC_CONFIG_SRCDIR([PHPPaloVersion.cpp])

#dont use a special config-header
#(No changes to the source-code) 
#AC_CONFIG_HEADER([config.h])

#disable building static libraries (we only want a shared libpalo_ng)
AC_DISABLE_STATIC

#dont use Makefiles for each subdir
#use automake to generate Makefile.in
AM_INIT_AUTOMAKE([foreign subdir-objects])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
#use libtool to build libraries
AC_PROG_LIBTOOL

#variable for location of PHP-link.
AC_ARG_VAR([PHPHEADERS],[location of PHP includes])

CXXFLAGS="$CXXFLAGS -std=c++0x"

#ZTS switch
AC_MSG_CHECKING(whether to use ZTS)
AC_ARG_ENABLE(zts,
	AS_HELP_STRING([--enable-zts],[use Zend ZTS]),
	CPPFLAGS="$CPPFLAGS -DZTS=1"
        AC_MSG_RESULT(yes), 
        AC_MSG_RESULT(no) 
)

#debugging switch 
AC_MSG_CHECKING(whether to enable debugging)
AC_ARG_ENABLE(debug,
	AS_HELP_STRING([--enable-debug],[create debug version]),
	CPPFLAGS="$CPPFLAGS -g -ggdb -O0"
	CFLAGS  ="$CFLAGS   -g -ggdb -O0"
	AC_MSG_RESULT(yes), 
        AC_MSG_RESULT(no)        		    
)


#Check if PHP include directory is set and ,if not,
#set the variable to a default value
AC_MSG_CHECKING(PHP include directory)
echo "Value of PHPHEADERS: ${PHPHEADERS}"
if test ! -n "$PHPHEADERS"
then
PHPHEADERS=./PHP
echo "Setting default-value : PHPHEADERS to ${PHPHEADERS}"
fi

#Check if above PHP directory exists
#the value of PHPHEADERS is saved between 
#invocations of configure.
if test ! -e ${PHPHEADERS}
then
AC_MSG_ERROR( \
		["PHPHEADERS is not set correctly. Please create a symbolic link named 
		./PHP to your PHP includes inside the source directory. 
		Use ln -s to do this. (e.g. ln -s /path/to/php ./PHP "
		Alternatively, pass the path to configure as in
		./configure PHPHEADERS=/path/to/php/includes.])
fi

# We assume that the PHP directory is now correct.
AC_CHECK_HEADERS([locale.h  ${PHPHEADERS}/main/php.h])

# Checks for typedefs, structures, and compiler characteristics.i
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

# Checks for header files including libpalo_ng
# in our development tree.
AC_CHECK_FILE(${PALOINC}/libpalo_ng/Palo/Server.h)


AC_CONFIG_FILES([Makefile])
#we need some sourcefiles from libpalo2. libpalo_ng: headers only
AC_OUTPUT
#Put our lib in usr/lib
AC_PREFIX_DEFAULT(/usr)
