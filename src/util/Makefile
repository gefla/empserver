# 
#   Empire - A multi-player, client/server Internet based war game.
#   Copyright (C) 1986-2000, Dave Pare, Jeff Bailey, Thomas Ruschak,
#                            Ken Stevens, Steve McClure
# 
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
# 
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
# 
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
# 
#   ---
# 
#   See the "LEGAL", "LICENSE", "CREDITS" and "README" files for all the
#   related information and legal notices. It is expected that any future
#   projects/authors will amend these files as needed.
# 
# Makefile - Wolfpack, 1996

# Note that these could have been listed 1 per line, but I chose to just
# stick them all together this way to shorten the file.

include ../../build.conf
include ../make.src
include ../make.defs

LIBDIR = $(SRCDIR)/lib

LIBS = $(LIBDIR)/libsubs.a \
       $(LIBDIR)/libcommon.a \
       $(LIBDIR)/libgen.a \
       $(LIBDIR)/libglobal.a

NTLIBS = $(LIBDIR)\libsubs.lib \
       $(LIBDIR)\libcommon.lib \
       $(LIBDIR)\libgen.lib \
       $(LIBDIR)\libglobal.lib

OBJS = fairland.o files.o land.o ore.o pconfig.o perfect.o

NTOBJS = fairland.obj files.obj land.obj ore.obj pconfig.obj perfect.obj

nt: ntfairland ntfiles ntpconfig

all: fairland files pconfig

osx: fairland files pconfig

# all: $(OBJS) fairland files land ore pconfig perfect

ntfiles: files.obj
	link /OUT:files.exe \
        /NOD /DEBUG:full /DEBUGTYPE:cv /MACHINE:IX86 /NOLOGO \
        files.obj $(NTLIBS) \
        kernel32.lib \
        wsock32.lib \
        libcmt.lib \
        oldnames.lib

files: files.o
	$(CC) -o files $(IFLAGS) $(CFLAGS) files.o $(LIBS) $(LFLAGS) -lc

land: land.o
	$(CC) -o land $(IFLAGS) $(CFLAGS) land.o $(LIBS) $(LFLAGS) -lc

ore: ore.o
	$(CC) -o ore $(IFLAGS) $(CFLAGS) ore.o $(LIBS) $(LFLAGS) -lc

fairland: fairland.o
	$(CC) -o fairland $(IFLAGS) $(CFLAGS) fairland.o $(LIBS) $(LFLAGS) -lc

ntfairland: fairland.obj
	link /OUT:fairland.exe \
        /NOD /DEBUG:full /DEBUGTYPE:cv /MACHINE:IX86 /NOLOGO \
        fairland.obj $(NTLIBS) \
        kernel32.lib \
        wsock32.lib \
        libcmt.lib \
        oldnames.lib

perfect: perfect.o
	$(CC) -o perfect $(IFLAGS) $(CFLAGS) perfect.o $(LIBS) $(LFLAGS) -lc

pconfig: pconfig.o
	$(CC) -o pconfig $(IFLAGS) $(CFLAGS) pconfig.o $(LIBS) $(LFLAGS) -lc

ntpconfig: pconfig.o
	link /OUT:pconfig.exe \
        /NOD /DEBUG:full /DEBUGTYPE:cv /MACHINE:IX86 /NOLOGO \
        pconfig.obj $(NTLIBS) \
        kernel32.lib \
        wsock32.lib \
        libcmt.lib \
        oldnames.lib

clean:
	-(rm -f files land ore fairland perfect pconfig $(OBJS))
	-(del /q *.exe *.pdb *.ilk $(NTOBJS))

install:
	-(mkdir $(EMPDIR)/bin)
	-(cp files $(EMPDIR)/bin)
#	-(cp land $(EMPDIR)/bin)
#	-(cp ore $(EMPDIR)/bin)
	-(cp fairland $(EMPDIR)/bin)
#	-(cp perfect $(EMPDIR)/bin)
	-(cp pconfig $(EMPDIR)/bin)

ntinstall:
	-(mkdir $(NTINSTDIR)\bin)
	-(copy files.exe $(NTINSTDIR)\bin)
#	-(copy land.exe $(NTINSTDIR)\bin)
#	-(copy ore.exe $(NTINSTDIR)\bin)
	-(copy fairland.exe $(NTINSTDIR)\bin)
#	-(copy perfect.exe $(NTINSTDIR)\bin)
	-(copy pconfig.exe $(NTINSTDIR)\bin)

include ../make.rules
include Makedepend
