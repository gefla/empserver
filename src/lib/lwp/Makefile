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
# Makefile - Wolfpack, 1996

# Note that these could have been listed 1 per line, but I chose to just
# stick them all together this way to shorten the file.

include ../../../build.conf
include ../../make.src
include ../../make.defs

LIB = $(SRCDIR)/lib/liblwp.a

OBJS = arch.o lwp.o queue.o sel.o sem.o status.o

MIPSOBJS = mipsarch.o

AIXOBJS = lwpInit.o lwpRestore.o lwpSave.o

GENMASTER = GLOBALCFLAGS="$(GLOBALCFLAGS)" GLOBALLFLAGS="$(GLOBALLFLAGS)"

all: $(LIB)

$(LIB): $(OBJS) $(EXTRAOBJS)
	rm -f $(LIB)
	ar cq $(LIB) $(OBJS) $(EXTRAOBJS)
	$(RANLIB) $(LIB)

nt:

mipsultrix: 
	make EXTRAOBJS="$(MIPSOBJS)" $(GENMASTER)

aix:
	make EXTRAOBJS="$(AIXOBJS)" $(GENMASTER)

clean:
	-(rm -f $(OBJS) $(AIXOBJS) $(MIPSOBJS))

include ../../make.rules
include Makedepend
