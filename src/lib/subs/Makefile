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

LIB = $(SRCDIR)/lib/libsubs.a
NTLIB = $(SRCDIR)\lib\libsubs.lib

OBJS = aircombat.o askyn.o aswplnsubs.o attsub.o bigmap.o border.o bsanct.o \
	caploss.o chkmoney.o cnumb.o comsub.o control.o detonate.o disloan.o \
	distrea.o fileinit.o fortdef.o getbit.o getele.o land.o landgun.o \
	list.o lndsub.o lostsub.o mission.o move.o mslsub.o mtch.o natarg.o \
	neigh.o nreport.o nstr.o nuke.o nxtitem.o nxtsct.o paths.o plane.o \
	plnsub.o pr.o radmap.o rej.o retreat.o sarg.o satmap.o \
	sect.o ship.o show.o shpsub.o snxtitem.o snxtsct.o supply.o \
	takeover.o trdsub.o trechk.o whatitem.o wu.o

NTOBJS = aircombat.obj askyn.obj aswplnsubs.obj attsub.obj bigmap.obj \
	border.obj bsanct.obj caploss.obj chkmoney.obj cnumb.obj comsub.obj \
	control.obj detonate.obj disloan.obj distrea.obj fileinit.obj fortdef.obj \
	getbit.obj getele.obj land.obj landgun.obj list.obj lndsub.obj \
	lostsub.obj mission.obj move.obj mslsub.obj mtch.obj natarg.obj neigh.obj \
	nreport.obj nstr.obj nuke.obj nxtitem.obj nxtsct.obj paths.obj plane.obj \
	plnsub.obj pr.obj radmap.obj rej.obj retreat.obj sarg.obj satmap.obj \
	sect.obj ship.obj show.obj shpsub.obj snxtitem.obj \
	snxtsct.obj supply.obj takeover.obj trdsub.obj trechk.obj whatitem.obj \
	wu.obj

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
