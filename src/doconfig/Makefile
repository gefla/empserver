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
#            build the configuration program, then run it.
#            Note that this is a stand-alone - it doesn't need anything 
#            else to build and run, other than build.conf for definitions.


include ../../build.conf

DPV = -DPV="\"$(MYNAME)\""
DHN = -DHN="\"$(HOSTNAME)\""
DPN = -DPN=$(PORTNUM)
DEM = -DEM="\"$(EMAIL)\""
DEP = -DEP="\"$(EMPDIR)\""
DMC = -DMC=$(MAXNOC)
DWX = -DWX=$(WORLDX)
DWY = -DWY=$(WORLDY)
DBL = -DBL=$(BLITZ)
DET = -DET=$(ETUS)
DEF = -DEF="\"$(ETUFREQ)\""
DIP = -DIP="\"$(IPADDR)\""
DUN = -DUN="\"$(USERNAME)\""

DALL = $(DPV) $(DHN) $(DPN) $(DEM) $(DEP) $(DMC) $(DWX) $(DWY) $(DBL) $(DET) $(DEF) $(DIP) $(DUN)

all: doconfig

doconfig: ../../build.conf doconfig.c
	@echo 'globalcflags = $(GLOBALCFLAGS)'
	@echo 'globallflags = $(GLOBALLFLAGS)'
	$(CC) $(GLOBALCFLAGS) $(DALL) doconfig.c -o doconfig $(GLOBALLFLAGS)
	./doconfig

nt: ntdoconfig

ntdoconfig: ..\..\build.conf doconfig.c
	@echo 'globalcflags = $(GLOBALCFLAGS)'
	@echo 'globallflags = $(GLOBALLFLAGS)'
	$(CC) $(GLOBALCFLAGS) $(DALL) doconfig.c -o doconfig $(GLOBALLFLAGS)
	doconfig.exe

clean:
	-(rm -f doconfig)
	-(del doconfig.exe)
	-(del *.obj)
	-(del *.pdb)

include ../make.rules
include Makedepend
