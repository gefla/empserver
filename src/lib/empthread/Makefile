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

include ../../../build.conf
include ../../make.src
include ../../make.defs

LIB = $(SRCDIR)/lib/libempth.a
NTLIB = $(SRCDIR)\lib\libempth.lib

OBJS = lwp.o pthread.o ntthread.o

NTOBJS = lwp.obj pthread.obj ntthread.obj

all: $(LIB)

nt: $(NTLIB)

$(NTLIB): $(NTOBJS)
	-del /q $@
	lib /OUT:$@ /DEBUGTYPE:CV $(NTOBJS)

$(LIB): $(OBJS)
	rm -f $(LIB)
	ar cq $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

clean:
	-(rm -f $(OBJS))
	-(del /q $(NTOBJS))

include ../../make.rules
-include Makedepend
