# 
#   Empire - A multi-player, client/server Internet based war game.
#   Copyright (C) 1986-2004, Dave Pare, Jeff Bailey, Thomas Ruschak,
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

OBJS = main.o idle.o lostitem.o marketup.o shutdown.o timestamp.o update.o

NTOBJS = main.obj idle.obj lostitem.obj marketup.obj shutdown.obj \
	timestamp.obj update.obj

LIBDIR = $(SRCDIR)/lib

LIBS = $(LIBDIR)/libplayer.a \
       $(LIBDIR)/libupdate.a \
       $(LIBDIR)/libcommands.a \
       $(LIBDIR)/libsubs.a \
       $(LIBDIR)/libcommon.a \
       $(LIBDIR)/libgen.a \
       $(LIBDIR)/libglobal.a \
       $(LIBDIR)/libas.a \
       $(LIBDIR)/libempth.a \
       $(LIBDIR)/liblwp.a

OSXLIBS = $(LIBDIR)/libplayer.a \
       $(LIBDIR)/libupdate.a \
       $(LIBDIR)/libcommands.a \
       $(LIBDIR)/libsubs.a \
       $(LIBDIR)/libcommon.a \
       $(LIBDIR)/libgen.a \
       $(LIBDIR)/libglobal.a \
       $(LIBDIR)/libas.a \
       $(LIBDIR)/libempth.a

NTLIBS = $(LIBDIR)\libplayer.lib \
       $(LIBDIR)\libupdate.lib \
       $(LIBDIR)\libcommands.lib \
       $(LIBDIR)\libsubs.lib \
       $(LIBDIR)\libcommon.lib \
       $(LIBDIR)\libgen.lib \
       $(LIBDIR)\libglobal.lib \
       $(LIBDIR)\libas.lib \
       $(LIBDIR)\libempth.lib

all: emp_server

osx: empX_server

nt: emp_server.exe

emp_server.exe: $(NTOBJS) $(NTLIBS)
	link /OUT:emp_server.exe \
        /PDB:emp_server.pdb  /NOLOGO \
        /NOD /DEBUG:full /DEBUGTYPE:cv /MACHINE:IX86 /NOLOGO \
        $(NTOBJS) $(NTLIBS) \
        kernel32.lib \
        ws2_32.lib \
        libcmt.lib \
        oldnames.lib \
        advapi32.lib

emp_server: $(OBJS) $(LIBS)
	$(CC) -o emp_server $(OBJS) $(LIBS) $(LFLAGS) $(GLOBALPFLAGS) -lm -lc

empX_server: $(OBJS) $(OSXLIBS)
	$(CC) -o emp_server $(OBJS) $(OSXLIBS) $(LFLAGS) $(GLOBALPFLAGS) -lm -lc

clean:
	-(del /q $(NTOBJS))
	-(del /q emp_server.exe)
	-(del /q emp_server.ilk)
	-(del /q emp_server.pdb)
	-(rm -f $(OBJS))
	-(rm -f emp_server)
	-(rm -f $(OSXOBJS))
	-(rm -f empX_server)

install:
	-(mkdir $(EMPDIR)/bin)
	-(cp emp_server $(EMPDIR)/bin)

OSXinstall:
	-(mkdir $(EMPDIR)/bin)
	-(cp empX_server $(EMPDIR)/bin)

ntinstall:
	-(mkdir $(NTINSTDIR)\bin)
	-(copy emp_server.exe $(NTINSTDIR)\bin)

include ../make.rules
include Makedepend
