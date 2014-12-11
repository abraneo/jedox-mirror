################################################################################
### 
### Copyright (C) 2006-2013 Jedox AG
###
### This library is free software; you can redistribute it and/or
### modify it under the terms of the GNU Library General Public
### License as published by the Free Software Foundation; either
### version 2 of the License, or (at your option) any later version.
### 
### This library is distributed in the hope that it will be useful,
### but WITHOUT ANY WARRANTY; without even the implied warranty of
### MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
### Library General Public License for more details.
###
### You should have received a copy of the GNU Library General Public
### License along with this library; if not, write to the
### Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
### Boston, MA  02110-1301, USA.
###
###
### \author Oliver Kania
###
################################################################################

AUTOMAKE_OPTIONS = foreign

INCLUDES = -I${top_srcdir}/include
ACLOCAL_AMFLAGS = -I m4 -I config


LIBS += $(PTHREAD_LIBS) 
CFLAGS += $(PTHREAD_CFLAGS)

AM_LDFLAGS=$(PTHREAD_LIBS) -lssl -licui18n -licuuc

#generate shared library 
#using libtool
lib_LTLIBRARIES= libpalo_ng.la

libpalo_ng_la_SOURCES = \
  libpalo_ng_version.c \
  Network/TcpDevice/TcpDevice.cpp \
  Network/HttpClient/Url.cpp \
  Network/HttpClient/HttpExceptionFactory.cpp \
  Network/HttpClient/HttpClientConnection.cpp \
  Network/HttpClient/HttpsClientConnection.cpp \
  Network/HttpClient/HttpClient.cpp \
  Network/HttpClient/BasicAuthentication.cpp \
  Network/HttpClient/HttpClientRequest.cpp \
  Network/HttpClient/HttpClientConnectionPool.cpp \
  Network/HttpClient/HttpsClientConnectionPool.cpp \
  Network/HttpClient/HttpClientResponse.cpp \
  Network/NetInitialisation.cpp \
  Network/SocketAddress.cpp \
  Network/SocketException.cpp \
  Network/ClientSocket.cpp \
  Network/Socket.cpp \
  Palo/Logging/Logging.cpp \
  Palo/Config/Config.cpp \
  Palo/Network/CubeToken.cpp \
  Palo/Network/ServerToken.cpp \
  Palo/Network/Token.cpp \
  Palo/Network/DatabaseToken.cpp \
  Palo/Network/DimensionToken.cpp \
  Palo/Network/PaloClient.cpp \
  Palo/Dimensions.cpp \
  Palo/Exception/PaloExceptionFactory.cpp \
  Palo/Exception/LibPaloNGExceptionFactory.cpp \
  Palo/Element.cpp \
  Palo/Cube.cpp \
  Palo/Exception/MaximumServerRetrysReachedException.cpp \
  Palo/Dimension.cpp \
  Palo/ServerPool.cpp \
  Palo/ServerImpl.cpp \
  Palo/Cubes.cpp \
  Palo/types.cpp \
  Palo/Database.cpp \
  Palo/Server.cpp \
  Palo/View.cpp \
  Util/StringUtils.cpp \
  Util/CsvTokenFromStream.cpp \
  Util/CsvLineDecoder.cpp
  
AM_CPPFLAGS = $(BOOST_CPPFLAGS) 
AM_LDFLAGS += $(BOOST_REGEX_LDFLAGS) $(BOOST_SERIALIZATION_LDFLAGS) $(BOOST_THREAD_LDFLAGS) $(BOOST_REGEX_LIBS) $(BOOST_THREAD_LIBS)

if INSTALL_LIBPALO_HEADERS 
palong_includedir=$(includedir)/libpalo_ng
palong_include_HEADERS=include/libpalo_ng/config_ng.h include/libpalo_ng/libpalo_ng_version.h
Network_includedir=$(includedir)/Network
Network_include_HEADERS=include/libpalo_ng/Network/NetInitialisation.h include/libpalo_ng/Network/SocketException.h
Palo_includedir=$(includedir)/libpalo_ng/Palo
Palo_include_HEADERS=include/libpalo_ng/Palo/Cube.h include/libpalo_ng/Palo/Cubes.h include/libpalo_ng/Palo/Database.h include/libpalo_ng/Palo/Dimension.h include/libpalo_ng/Palo/Dimensions.h include/libpalo_ng/Palo/Element.h include/libpalo_ng/Palo/Server.h include/libpalo_ng/Palo/ServerPool.h include/libpalo_ng/Palo/types.h
PaloCache_includedir=$(includedir)/libpalo_ng/Palo/Cache
PaloCache_include_HEADERS=include/libpalo_ng/Palo/Cache/AbstractCache.h include/libpalo_ng/Palo/Cache/Cache.h include/libpalo_ng/Palo/Cache/CubeCache.h include/libpalo_ng/Palo/Cache/CubesCache.h include/libpalo_ng/Palo/Cache/DatabaseCache.h include/libpalo_ng/Palo/Cache/DimensionCache.h include/libpalo_ng/Palo/Cache/DimensionsCache.h include/libpalo_ng/Palo/Cache/ElementCache.h include/libpalo_ng/Palo/Cache/OnlineCubeCache.h include/libpalo_ng/Palo/Cache/ServerCache.h
PaloLogging_includedir=$(includedir)/libpalo_ng/Palo/Logging
PaloLogging_include_HEADERS=include/libpalo_ng/Palo/Logging/Logging.h
PaloException_includedir=$(includedir)/libpalo_ng/Palo/Exception
PaloException_include_HEADERS=include/libpalo_ng/Palo/Exception/CubeNotFoundException.h include/libpalo_ng/Palo/Exception/DatabaseNotFoundException.h include/libpalo_ng/Palo/Exception/DimensionNotFoundException.h include/libpalo_ng/Palo/Exception/ElementNotFoundException.h include/libpalo_ng/Palo/Exception/ExceptionFactory.h include/libpalo_ng/Palo/Exception/PaloExceptionFactory.h include/libpalo_ng/Palo/Exception/PaloException.h include/libpalo_ng/Palo/Exception/PaloServerException.h include/libpalo_ng/Palo/Exception/ResolveException.h
PaloNetwork_includedir=$(includedir)/libpalo_ng/Palo/Network
PaloNetwork_include_HEADERS=include/libpalo_ng/Palo/Network/CubeToken.h include/libpalo_ng/Palo/Network/DatabaseToken.h include/libpalo_ng/Palo/Network/DimensionToken.h include/libpalo_ng/Palo/Network/PaloClient.h include/libpalo_ng/Palo/Network/ServerToken.h include/libpalo_ng/Palo/Network/Token.h include/libpalo_ng/Palo/Network/Tokens.h
Util_includedir=$(includedir)/libpalo_ng/Util
Util_include_HEADERS=include/libpalo_ng/Util/StringUtils.h
endif
