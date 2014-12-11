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
### \author Dragan Simic <dragan.simic@jedox.com>
###
###
################################################################################

ifneq "$(MAKECMDGOALS)" "clean"
LIBDIR=/usr/lib
OPENSSLINC ?= /usr/include
PALOINC ?= ../libpalo_ng/source/include
endif

CXX     = g++
LIBRARY = libPaloSpreadsheetFuncs

SOURCES  =$(filter-out DocInitializer.h.cpp,$(wildcard *.cpp))
OBJECTS  =$(subst .cpp,.o,$(SOURCES))
DEPFILES =$(subst .cpp,.d,$(SOURCES))

ARCH      :=$(shell _MAKE_ARCH=`uname -m`; if echo $$_MAKE_ARCH | egrep '^i[356]86$$' > /dev/null; then echo i386; else echo $$_MAKE_ARCH; fi)
ARCH_FLAG :=$(if $(filter $(ARCH),i386),-m32,$(if $(filter $(ARCH),x86_64),-m64,))

override CXXFLAGS :=$(ARCH_FLAG) -D__UNIX__ -g -fPIC -std=c++0x $(CXXFLAGS)
override LDFLAGS  :=$(ARCH_FLAG) -g -fPIC -lpalo_ng -licui18n -licudata -licuuc $(LDFLAGS) $(if $(PALOLIB),-L$(PALOLIB),) $(if $(BOOSTLIB),-L$(BOOSTLIB),) $(if $(ICULIB),-L$(ICULIB),) -L/lib -L/usr/lib
override INCLUDES :=$(INCLUDES) -I. -Iinclude -I$(OPENSSLINC) -I$(PALOINC) $(if $(BOOSTINC),-I$(BOOSTINC),) $(if $(ICUINC),-I$(ICUINC),)


vpath %.h ./include $(PALOINC)

all: $(LIBRARY)

install: $(LIBRARY)
	cp $(LIBRARY).a $(LIBDIR)
	cp $(LIBRARY).so $(LIBDIR)

$(LIBRARY): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -shared -o $@.so 
	$(AR) rcs $(LIBRARY).a *.o
	ranlib $(LIBRARY).a

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

%.d: %.cpp
	@set -e; $(RM) $@; \
	$(CXX) -M -MG $(CXXFLAGS) $(INCLUDES) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	$(RM) $@.$$$$

clean:
	$(RM) *.o *.d $(LIBRARY).so $(LIBRARY).a

.PHONY: all clean

ifneq "$(MAKECMDGOALS)" "clean"
-include $(DEPFILES)
endif
