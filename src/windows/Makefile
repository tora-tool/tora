

# BEGIN_COMMON_COPYRIGHT_HEADER
#
# TOra - An Oracle Toolkit for DBA's and developers
# 
# Shared/mixed copyright is held throughout files in this product
# 
# Portions Copyright (C) 2000-2001 Underscore AB
# Portions Copyright (C) 2003-2005 Quest Software, Inc.
# Portions Copyright (C) 2004-2009 Numerous Other Contributors
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation;  only version 2 of
# the License is valid for this program.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
# 
#      As a special exception, you have permission to link this program
#      with the Oracle Client libraries and distribute executables, as long
#      as you follow the requirements of the GNU GPL in regard to all of the
#      software in the executable aside from Oracle client libraries.
# 
#      Specifically you are not permitted to link this program with the
#      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
#      And you are not permitted to distribute binaries compiled against
#      these libraries. 
# 
#      You may link this product with any GPL'd Qt library.
# 
# All trademarks belong to their respective owners.
#
# END_COMMON_COPYRIGHT_HEADER

# This script only compiles the resources and windows classes in this directory

# C++ Compiler to use
GCC=g++

# Windows resource compiler
RC=windres

# Additional defines
DEFINES=-D_WIN32

# Additional includes needed to compile program
INCLUDES=

# Comment out this line if you want more output from compile
.SILENT:

# Additional flags set when using only the precompiler.
CPPFLAGS_GLOB=

# Additional flags set when compiling.
CFLAGS_GLOB=-g -O3 -W -Wall -c -w -fno-rtti

############################################################################
#
# End of configuration part of Makefile
#
############################################################################

SOURCES=\
	cregistry.cpp

RESOURCES=\
	tora.rc

	
default: all


CPPFLAGS=$(CPPFLAGS_GLOB) $(DEFINES) $(INCLUDES)
CFLAGS=$(CFLAGS_GLOB) $(INCLUDES) $(DEFINES)

OBJECTS=$(filter %.o,$(SOURCES:%.cpp=%.o)) $(filter %.o,$(RESOURCES:%.rc=%.o))

DEPENDS=$(filter %.d,$(SOURCES:%.cpp=.depends/%.d))

vpath %.h $(INCLUDE)

.PHONY: all clean

all: $(OBJECTS) 

%.moc: %.h
	@echo Metacompiling $<
	$(MOC) -o $@ $<

%.o: %.rc
	@echo Compiling resources $<
	$(RC) -o $@ $<

%.o: %.cpp
	@echo Compiling $<
	$(GCC) $(CFLAGS) -o $@ -c $<

include $(DEPENDS)

.depends/%.d: %.cpp
	@echo Making dependencies for $<
	if [ ! -d .depends ] ; then mkdir -p .depends ; fi
	$(GCC) -MM -MG $(CPPFLAGS) $< > $@.tmp && \
		( sed "s/^\(.*\.\)o\([ :]*\)/\1o \
		$(subst /,\\/,$@)\2/g" < $@.tmp > $@ ; rm -f $@.tmp )
clean: 
	@echo Cleaning $(TITLE)
	-rm -rf *.o
