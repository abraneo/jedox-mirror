#
# Author: Oliver Kania
# Copyright (c) 2006-2014, Jedox AG, Freiburg i.Br., Germany
#

AUTOMAKE_OPTIONS = gnu
ACLOCAL_AMFLAGS = -I config
INCLUDES = -I../libpalo_ng/source/include -I../JedoxXlHelper
AM_CPPFLAGS = -std=c++0x -D__UNIX__ -D_MT

#use static boost libraries
#TODO: Is this really necessary/correct ?
AM_LDFLAGS  = --static

#generate shared library 
lib_LTLIBRARIES    = libpalo2.la

libpalo2_la_SOURCES = \
  ../JedoxXlHelper/JedoxXLHelper.cpp \
  arg.cpp \
  connect.cpp \
  errstack.cpp \
  hash_table.cpp \
  helper.cpp \
  interface.cpp \
  interface_generic.cpp \
  libpalo2.cpp \
  stdaliases.cpp \
  strtools.cpp

libpalo2_la_LIBADD = \
  -lpalo_ng

