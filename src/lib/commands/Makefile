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

LIB = $(SRCDIR)/lib/libcommands.a
NTLIB = $(SRCDIR)\lib\libcommands.lib

OBJS =  acce.o add.o anti.o arm.o army.o assa.o atta.o bdes.o best.o boar.o \
	bomb.o brea.o budg.o buil.o buy.o capi.o carg.o cede.o cens.o chan.o \
	coas.o coll.o comm.o cons.o conv.o coun.o cuto.o decl.o deli.o demo.o \
	desi.o disa.o diss.o dist.o drop.o dump.o echo.o edit.o enab.o enli.o \
	expl.o fina.o flash.o flee.o fly.o foll.o forc.o fort.o fuel.o give.o \
	grin.o hard.o head.o hidd.o improve.o info.o land.o laun.o lboard.o \
	ldump.o ledg.o leve.o load.o look.o lost.o lstat.o lten.o map.o \
	marc.o mark.o mfir.o mine.o miss.o mobq.o mobu.o mora.o move.o mult.o \
	name.o nati.o navi.o ndump.o new.o newe.o news.o nuke.o offe.o offs.o \
	orde.o orig.o para.o path.o payo.o pboa.o pdump.o plan.o play.o \
	powe.o prod.o pstat.o rada.o rang.o rea.o real.o reco.o reje.o rela.o \
	repa.o repo.o rese.o reso.o retr.o rout.o sabo.o sail.o sate.o scra.o \
	scut.o sdump.o sect.o sell.o set.o setres.o setsect.o shar.o shark.o \
	shi.o shoo.o show.o shut.o sinf.o skyw.o sona.o spy.o sstat.o star.o \
	stop.o stre.o strv.o supp.o surv.o swap.o tele.o tend.o terr.o thre.o \
	togg.o torp.o trad.o tran.o trea.o turn.o upda.o upgr.o vers.o wai.o \
	wing.o wipe.o work.o zdon.o

NTOBJS = acce.obj add.obj anti.obj arm.obj army.obj assa.obj atta.obj \
	bdes.obj best.obj boar.obj bomb.obj brea.obj budg.obj buil.obj \
	buy.obj capi.obj carg.obj cede.obj cens.obj chan.obj coas.obj \
	coll.obj comm.obj cons.obj conv.obj coun.obj cuto.obj decl.obj \
	deli.obj demo.obj desi.obj disa.obj diss.obj dist.obj drop.obj \
	dump.obj echo.obj edit.obj enab.obj enli.obj expl.obj fina.obj \
	flash.obj flee.obj fly.obj foll.obj forc.obj fort.obj fuel.obj \
	give.obj grin.obj hard.obj head.obj hidd.obj improve.obj info.obj \
	land.obj laun.obj lboard.obj ldump.obj ledg.obj leve.obj load.obj \
	look.obj lost.obj lstat.obj lten.obj map.obj marc.obj mark.obj \
	mfir.obj mine.obj miss.obj mobq.obj mobu.obj mora.obj move.obj \
	mult.obj name.obj nati.obj navi.obj ndump.obj new.obj newe.obj \
	news.obj nuke.obj offe.obj offs.obj orde.obj orig.obj para.obj \
	path.obj payo.obj pboa.obj pdump.obj plan.obj play.obj powe.obj \
	prod.obj pstat.obj rada.obj rang.obj rea.obj real.obj reco.obj \
	reje.obj rela.obj repa.obj repo.obj rese.obj reso.obj retr.obj \
	rout.obj sabo.obj sail.obj sate.obj scra.obj scut.obj sdump.obj \
	sect.obj sell.obj set.obj setres.obj setsect.obj shar.obj shark.obj \
	shi.obj shoo.obj show.obj shut.obj sinf.obj skyw.obj sona.obj spy.obj \
	sstat.obj star.obj stop.obj stre.obj strv.obj supp.obj surv.obj \
	swap.obj tele.obj tend.obj terr.obj thre.obj togg.obj torp.obj \
	trad.obj tran.obj trea.obj turn.obj upda.obj upgr.obj vers.obj \
	wai.obj wing.obj wipe.obj work.obj zdon.obj

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
include Makedepend
