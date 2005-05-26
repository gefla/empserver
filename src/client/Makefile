# 
#   Empire - A multi-player, client/server Internet based war game.
#   Copyright (C) 1986-2005, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
#
# Empire front end Makefile
# (server mode)
#

include ../../build.conf
include ../make.src
include ../make.defs

CFILES = expect.c handle.c host.c \
	ioqueue.c ipglob.c login.c main.c queue.c saveargv.c \
	servcmd.c serverio.c tags.c termio.c termlib.c
OFILES = expect.o handle.o host.o \
	ioqueue.o ipglob.o login.o main.o queue.o saveargv.o \
	servcmd.o serverio.o tags.o termio.o termlib.o
OBJFILES = expect.obj handle.obj host.obj \
	ioqueue.obj ipglob.obj login.obj main.obj queue.obj saveargv.obj \
	servcmd.obj serverio.obj tags.obj termio.obj termlib.obj

CLIENTLIBS= -ltermlib

#
# Change this line to the compiler you are going to use.
#

all: emp_client

osx: emp_client

emp_client: $(OFILES) 
	$(CC) -o emp_client $(OFILES) $(LFLAGS) $(CLIENTLIBS) -lc

nt: emp_client.exe

emp_client.exe: $(OBJFILES) $(CFILES)
	cl $(CFLAGS) /Feemp_client.exe $(OBJFILES) wsock32.lib advapi32.lib

dgux: $(OFILES) 
	$(CC) $(CFLAGS) -dn -o emp_client $(OFILES) -lsocket -lnsl -ltermcap -lc

solaris:
	$(MAKE) emp_client CLIENTLIBS="-lsocket -lnsl -ltermcap -lc"

hpux:
	$(MAKE) emp_client CLIENTLIBS=-ltermcap

clean:
	-(rm -f $(OFILES))
	-(rm -f emp_client)
	-(del /q $(OBJFILES))
	-(del /q emp_client.exe)
	-(del /q emp_client.ilk)
	-(del /q emp_client.pdb)

install:
	-(mkdir $(EMPDIR)/bin)
	cp emp_client $(EMPDIR)/bin

ntinstall:
	-(mkdir $(NTINSTDIR)\bin)
	copy emp_client.exe $(NTINSTDIR)\bin

include ../make.rules
include Makedepend

