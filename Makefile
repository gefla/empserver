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
# Makefile - Wolfpack, 1996-2000
#            Do a generic build just by typing in "make <arch>"
#
# Note that the NT builds were done using MSVC 5.0 and using "nmake nt"
#  So, to build anything on NT, just put an "nt" in front (for example,
#  "ntclean", "ntinstall", etc.

include build.conf
include Make.sysdefs

TOBUILD = all
GENMASTER = $(TOBUILD) GLOBALCFLAGS="$(GLOBALCFLAGS)" GLOBALLFLAGS="$(GLOBALLFLAGS)"
CLIENTLIBS = -ltermcap


#
# Specific targets - make a new one for each new architecture or
#                    build environment.
#

all:
	@echo 'You must type "make <arch>" to build for a specific system.'
	@echo 'Currently buildable architectures are:'
	@echo '   aix'
	@echo '   alpha'
	@echo '   alpha-pthreads'
	@echo '   apollo'
	@echo '   freebsd'
	@echo '   hp'
	@echo '   hpux'
	@echo '   irix'
	@echo '   irix5'
	@echo '   isi'
	@echo '   linux'
	@echo '   linux-pthreads'
	@echo '   mach2.5'
	@echo '   mipsbsd'
	@echo '   mipsultrix'
	@echo '   mipsultrix.gcc'
	@echo '   next'
	@echo '   nt'
	@echo '   osx'
	@echo '   rtpc'
	@echo '   sequent'
	@echo '   sequent.fpa'
	@echo '   solaris'
	@echo '   sun3'
	@echo '   sun4'
	@echo '   sun4.debug'
	@echo '   sun4.opt'
	@echo '   vaxultrix'

alpha:
	@echo 'building a $(WORLDX) by $(WORLDY) server...'
	(cd src/doconfig; $(MAKE) $(ALPHAMASTER))
	($(MAKE) genlibs $(ALPHAMASTER))
	(cd src/lib/lwp; $(MAKE) $(ALPHAMASTER))
	($(MAKE) binaries $(ALPHAMASTER))
	($(MAKE) install)

alpha-pthreads:
	@echo 'building a $(WORLDX) by $(WORLDY) server...'
	(cd src/doconfig; $(MAKE) $(ALPHAPOSMASTER))
	($(MAKE) genlibs $(ALPHAPOSMASTER))
	(cd src/lib/lwp; $(MAKE) $(ALPHAPOSMASTER))
	($(MAKE) binaries $(ALPHAPOSMASTER))
	($(MAKE) install)

apollo:
	@echo 'building a $(WORLDX) by $(WORLDY) server...'
	(cd src/doconfig; $(MAKE) $(APOLLOMASTER))
	($(MAKE) genlibs $(APOLLOMASTER))
	(cd src/lib/lwp; $(MAKE) $(APOLLOMASTER))
	($(MAKE) binaries $(APOLLOMASTER))
	($(MAKE) install)

freebsd:
	@echo 'building a $(WORLDX) by $(WORLDY) server...'
	(cd src/doconfig; $(MAKE) $(FREEBSDMASTER))
	($(MAKE) genlibs $(FREEBSDMASTER))
	(cd src/lib/lwp; $(MAKE) $(FREEBSDMASTER))
	($(MAKE) binaries $(FREEBSDMASTER))
	($(MAKE) install)

hp:
	@echo 'building a $(WORLDX) by $(WORLDY) server...'
	(cd src/doconfig; $(MAKE) $(HPMASTER))
	($(MAKE) genlibs $(HPMASTER))
	(cd src/lib/lwp; $(MAKE) $(HPMASTER))
	($(MAKE) binaries $(HPMASTER))
	($(MAKE) install)

hpux:
	@echo 'building a $(WORLDX) by $(WORLDY) server...'
	(cd src/doconfig; $(MAKE) $(HPUXMASTER))
	($(MAKE) genlibs $(HPUXMASTER))
	(cd src/lib/lwp; $(MAKE) $(HPUXMASTER))
	($(MAKE) binaries $(HPUXMASTER) CLIENTLIBS="")
	($(MAKE) install)

next:
	@echo 'building a $(WORLDX) by $(WORLDY) server...'
	(cd src/doconfig; $(MAKE) $(NEXTMASTER))
	($(MAKE) genlibs $(NEXTMASTER))
	(cd src/lib/lwp; $(MAKE) $(NEXTMASTER))
	($(MAKE) binaries $(NEXTMASTER))
	($(MAKE) install)

osx:
	@echo 'building a $(WORLDX) by $(WORLDY) server...'
	(cd src/doconfig; $(MAKE) $(OSXMASTER))
	($(MAKE) genlibs $(OSXMASTER))
	($(MAKE) binaries TOBUILD=osx $(OSXMASTER) CLIENTLIBS="")
	($(MAKE) install)

irix:
	@echo 'building a $(WORLDX) by $(WORLDY) server...'
	(cd src/doconfig; $(MAKE) $(IRIXMASTER))
	($(MAKE) genlibs $(IRIXMASTER))
	(cd src/lib/lwp; $(MAKE) $(IRIXMASTER))
	($(MAKE) binaries $(IRIXMASTER))
	($(MAKE) install)

irix5:
	@echo 'building a $(WORLDX) by $(WORLDY) server...'
	(cd src/doconfig; $(MAKE) $(IRIX5MASTER))
	($(MAKE) genlibs $(IRIX5MASTER))
	(cd src/lib/lwp; $(MAKE) $(IRIX5MASTER))
	($(MAKE) binaries $(IRIX5MASTER))
	($(MAKE) install)

isi:
	@echo 'building a $(WORLDX) by $(WORLDY) server...'
	(cd src/doconfig; $(MAKE) $(ISIMASTER))
	($(MAKE) genlibs $(ISIMASTER))
	(cd src/lib/lwp; $(MAKE) $(ISIMASTER))
	($(MAKE) binaries $(ISIMASTER))
	($(MAKE) install)

linux:
	@echo 'building a $(WORLDX) by $(WORLDY) server...'
	@echo 'This build is only tested on x86 machines.  Try using'
	@echo 'linux-pthreads for other architectures.'
	(cd src/doconfig; $(MAKE) $(LINUXMASTER))
	($(MAKE) genlibs $(LINUXMASTER))
	(cd src/lib/lwp; $(MAKE) $(LINUXMASTER))
	($(MAKE) binaries $(LINUXMASTER))
	($(MAKE) install)

linux-pthreads:
	@echo 'building a $(WORLDX) by $(WORLDY) server...'
	(cd src/doconfig; $(MAKE) $(LINUXPTHMASTER))
	($(MAKE) genlibs $(LINUXPTHMASTER))
	(cd src/lib/lwp; $(MAKE) $(LINUXPTHMASTER))
	($(MAKE) binaries $(LINUXPTHMASTER))
	($(MAKE) install)

mipsbsd:
	@echo 'building a $(WORLDX) by $(WORLDY) server...'
	(cd src/doconfig; $(MAKE) $(MIPSBSDMASTER))
	($(MAKE) genlibs $(MIPSBSDMASTER))
	(cd src/lib/lwp; $(MAKE) mipsultrix $(MIPSBSDMASTER))
	($(MAKE) binaries $(MIPSBSDMASTER))
	($(MAKE) install)

mach2.5:
	@echo 'building a $(WORLDX) by $(WORLDY) server...'
	(cd src/doconfig; $(MAKE) $(MACH25MASTER))
	($(MAKE) genlibs $(MACH25MASTER))
	(cd src/lib/lwp; $(MAKE) mipsultrix $(MACH25MASTER))
	($(MAKE) binaries $(MACH25MASTER))
	($(MAKE) install)

mipsultrix:
	@echo 'building a $(WORLDX) by $(WORLDY) server...'
	(cd src/doconfig; $(MAKE) $(MIPSULTMASTER))
	($(MAKE) genlibs $(MIPSULTMASTER))
	(cd src/lib/lwp; $(MAKE) mipsultrix $(MIPSULTMASTER))
	($(MAKE) binaries $(MIPSULTMASTER))
	($(MAKE) install)

mipsultrix.gcc:
	@echo 'building a $(WORLDX) by $(WORLDY) server...'
	(cd src/doconfig; $(MAKE) $(MIPSULTGMASTER))
	($(MAKE) genlibs $(MIPSULTGMASTER))
	(cd src/lib/lwp; $(MAKE) mipsultrix $(MIPSULTGMASTER))
	($(MAKE) binaries $(MIPSULTGMASTER))
	($(MAKE) install)

nt:
	@echo 'building a $(WORLDX) by $(WORLDY) server...'
	cd src/doconfig
	$(MAKE) nt $(NTMASTER)
	cd ../..
	$(MAKE) ntgenlibs TOBUILD=nt $(NTMASTER)
	cd src/lib/lwp
	$(MAKE) nt $(NTMASTER)
	cd ../../..
	$(MAKE) ntbinaries TOBUILD=nt $(NTMASTER)
	$(MAKE) ntinstall

vaxultrix:
	@echo 'building a $(WORLDX) by $(WORLDY) server...'
	(cd src/doconfig; $(MAKE) $(VAXULTMASTER))
	($(MAKE) genlibs $(VAXULTMASTER))
	(cd src/lib/lwp; $(MAKE) $(VAXULTMASTER))
	($(MAKE) binaries $(VAXULTMASTER))
	($(MAKE) install)

aix:
	@echo 'building a $(WORLDX) by $(WORLDY) server...'
	(cd src/doconfig; $(MAKE) $(AIXMASTER))
	($(MAKE) genlibs $(AIXMASTER))
	(cd src/lib/lwp; $(MAKE) aix $(AIXMASTER))
	($(MAKE) binaries $(AIXMASTER))
	($(MAKE) install)

rtpc:
	@echo 'building a $(WORLDX) by $(WORLDY) server...'
	(cd src/doconfig; $(MAKE) $(RTPCMASTER))
	($(MAKE) genlibs $(RTPCMASTER))
	(cd src/lib/lwp; $(MAKE) aix $(RTPCMASTER))
	($(MAKE) binaries $(RTPCMASTER))
	($(MAKE) install)

sequent:
	@echo 'building a $(WORLDX) by $(WORLDY) server...'
	(cd src/doconfig; $(MAKE) $(SEQMASTER))
	($(MAKE) genlibs $(SEQMASTER))
	(cd src/lib/lwp; $(MAKE) $(SEQMASTER))
	($(MAKE) binaries $(SEQMASTER))
	($(MAKE) install)

sequent.fpa:
	@echo 'building a $(WORLDX) by $(WORLDY) server...'
	(cd src/doconfig; $(MAKE) $(SEQFPAMASTER))
	($(MAKE) genlibs $(SEQFPAMASTER))
	(cd src/lib/lwp; $(MAKE) $(SEQFPAMASTER))
	($(MAKE) binaries $(SEQFPAMASTER))
	($(MAKE) install)

solaris:
	@echo 'building a $(WORLDX) by $(WORLDY) server...'
	(cd src/doconfig; $(MAKE) $(SOLMASTER))
	($(MAKE) genlibs $(SOLMASTER))
	(cd src/lib/lwp; $(MAKE) $(SOLMASTER))
	($(MAKE) binaries $(SOLMASTER))
	($(MAKE) install)

sun3:
	@echo 'building a $(WORLDX) by $(WORLDY) server...'
	(cd src/doconfig; $(MAKE) $(SUN3MASTER))
	($(MAKE) genlibs $(SUN3MASTER))
	(cd src/lib/lwp; $(MAKE) $(SUN3MASTER))
	($(MAKE) binaries $(SUN3MASTER))
	($(MAKE) install)

sun4:
	@echo 'building a $(WORLDX) by $(WORLDY) server...'
	(cd src/doconfig; $(MAKE) $(SUN4MASTER))
	($(MAKE) genlibs $(SUN4MASTER))
	(cd src/lib/lwp; $(MAKE) $(SUN4MASTER))
	($(MAKE) binaries $(SUN4MASTER))
	($(MAKE) install)

sun4.opt:
	@echo 'building a $(WORLDX) by $(WORLDY) server...'
	(cd src/doconfig; $(MAKE) $(SUN4OPTMASTER))
	($(MAKE) genlibs $(SUN4OPTMASTER))
	(cd src/lib/lwp; $(MAKE) $(SUN4OPTMASTER))
	($(MAKE) binaries $(SUN4OPTMASTER))
	($(MAKE) install)

sun4.debug:
	@echo 'building a $(WORLDX) by $(WORLDY) server...'
	(cd src/doconfig; $(MAKE) $(SUN4DBGMASTER))
	($(MAKE) genlibs $(SUN4DBGMASTER))
	(cd src/lib/lwp; $(MAKE) $(SUN4DBGMASTER))
	($(MAKE) binaries $(SUN4DBGMASTER))
	($(MAKE) install)

#
# Generic stuff that is called from the architecture specific stuff.
#

binaries:
	@echo 'building the binaries'
	(cd src/client; $(MAKE) $(GENMASTER) CLIENTLIBS=$(CLIENTLIBS))
	(cd src/server; $(MAKE) $(GENMASTER))
	(cd src/util; $(MAKE) $(GENMASTER))

ntbinaries:
	@echo 'building the binaries'
	cd src/client
	$(MAKE) $(GENMASTER) CLIENTLIBS=$(CLIENTLIBS)
	cd ../..
	cd src/server
	$(MAKE) $(GENMASTER)
	cd ../..
	cd src/util
	$(MAKE) $(GENMASTER)
	cd ../..

genlibs:
	@echo 'building generic libraries'
	-mkdir lib
	(cd src/lib/as; $(MAKE)  $(GENMASTER))
	(cd src/lib/commands; $(MAKE)  $(GENMASTER))
	(cd src/lib/common; $(MAKE)  $(GENMASTER))
	(cd src/lib/empthread; $(MAKE)  $(GENMASTER))
	(cd src/lib/gen; $(MAKE)  $(GENMASTER))
	(cd src/lib/global; $(MAKE)  $(GENMASTER))
	(cd src/lib/player; $(MAKE)  $(GENMASTER))
	(cd src/lib/subs; $(MAKE)  $(GENMASTER))
	(cd src/lib/update; $(MAKE)  $(GENMASTER))
	@echo 'done building generic libraries'

ntgenlibs:
	@echo 'building generic libraries'
	-(mkdir lib)
	cd src/lib/as
	$(MAKE) $(GENMASTER)
	cd ../../..
	cd src/lib/commands
	$(MAKE) $(GENMASTER)
	cd ../../..
	cd src/lib/common
	$(MAKE) $(GENMASTER)
	cd ../../..
	cd src/lib/empthread
	$(MAKE) $(GENMASTER)
	cd ../../..
	cd src/lib/gen
	$(MAKE) $(GENMASTER)
	cd ../../..
	cd src/lib/global
	$(MAKE) $(GENMASTER)
	cd ../../..
	cd src/lib/player
	$(MAKE) $(GENMASTER)
	cd ../../..
	cd src/lib/subs
	$(MAKE) $(GENMASTER)
	cd ../../..
	cd src/lib/update
	$(MAKE) $(GENMASTER)
	cd ../../..
	@echo 'done building generic libraries'

clean:
	(cd src/lib/as; $(MAKE) clean)
	(cd src/lib/commands; $(MAKE) clean)
	(cd src/lib/common; $(MAKE) clean)
	(cd src/lib/gen; $(MAKE) clean)
	(cd src/lib/global; $(MAKE) clean)
	(cd src/lib/player; $(MAKE) clean)
	(cd src/lib/subs; $(MAKE) clean)
	(cd src/lib/update; $(MAKE) clean)
	(cd src/lib/lwp; $(MAKE) clean)
	(cd src/lib/empthread; $(MAKE) clean)
	(cd src/client; $(MAKE) clean)
	(cd src/server; $(MAKE) clean)
	(cd src/util; $(MAKE) clean)
	(cd src/doconfig; $(MAKE) clean)
	(rm -f lib/*.a)

ntclean:
	cd src/lib/as
	$(MAKE) clean
	cd ../../../
	cd src/lib/commands
	$(MAKE) clean
	cd ../../../
	cd src/lib/common
	$(MAKE) clean
	cd ../../../
	cd src/lib/gen
	$(MAKE) clean
	cd ../../../
	cd src/lib/global
	$(MAKE) clean
	cd ../../../
	cd src/lib/player
	$(MAKE) clean
	cd ../../../
	cd src/lib/subs
	$(MAKE) clean
	cd ../../../
	cd src/lib/update
	$(MAKE) clean
	cd ../../../
	cd src/lib/lwp
	$(MAKE) clean
	cd ../../../
	cd src/lib/empthread
	$(MAKE) clean
	cd ../../../
	cd src/client
	$(MAKE) clean
	cd ../../
	cd src/server
	$(MAKE) clean
	cd ../../
	cd src/util
	$(MAKE) clean
	cd ../../
	cd src/doconfig
	$(MAKE) clean
	cd ../../
	cd lib
	-(del /Q *.lib)

realclean:
	($(MAKE) clean)
	(rm -f bin/*)
	(rm -rf data/*)
	-(rm -rf $(EMPDIR)/data/*)
	-(rm -rf $(EMPDIR)/bin/*)
	rm -rf lib info.nr info.html
	rm -f include/gamesdef.h src/client/ipglob.c src/make.src

ntrealclean:
	$(MAKE) ntclean
	cd bin
	-(del /Q *.*)
	cd ..
	cd data
	-(del /Q *.*)
	cd ..
	cd $(EMPDIR)\data
	-(del /Q *.*)
	-(rmdir tel)
	cd $(EMPDIR)\bin
	-(del /Q *.*)
	cd $(EMPDIR)\data\tel
	-(del /Q *.*)

depend:
	(cd src/lib/as; $(MAKE) depend)
	(cd src/lib/commands; $(MAKE) depend)
	(cd src/lib/common; $(MAKE) depend)
	(cd src/lib/gen; $(MAKE) depend)
	(cd src/lib/global; $(MAKE) depend)
	(cd src/lib/player; $(MAKE) depend)
	(cd src/lib/subs; $(MAKE) depend)
	(cd src/lib/update; $(MAKE) depend)
	(cd src/lib/lwp; $(MAKE) depend)
	(cd src/lib/empthread; $(MAKE) depend)
	(cd src/client; $(MAKE) depend)
	(cd src/server; $(MAKE) depend)
	(cd src/util; $(MAKE) depend)
	(cd src/doconfig; $(MAKE) depend)

# The last three lines here ensure that we don't overwrite an existing
# econfig file, by moving it first then moving it back.  The '-''s
# ignore any errors (like if the files don't exist.)
# So, after an install, you will have an "econfig" file in the
# data directory if you didn't have one before.

install:
	(cd src/client; $(MAKE) install)
	(cd src/server; $(MAKE) install)
	(cd src/util; $(MAKE) install)
	-(mv $(EMPDIR)/data/econfig $(EMPDIR)/data/econfig.bak)
	($(EMPDIR)/bin/pconfig > $(EMPDIR)/data/econfig)
	-(mv $(EMPDIR)/data/econfig.bak $(EMPDIR)/data/econfig)

ntinstall:
	-(mkdir bin)
	cd src\client
	$(MAKE) ntinstall
	cd ..\..
	cd src\server
	$(MAKE) ntinstall
	cd ..\..
	cd src\util
	$(MAKE) ntinstall
	cd ..\..
	cd data
	($(NTINSTDIR)\bin\pconfig.exe > $(NTINSTDIR)\data\econfig.new)
	-(move econfig.new econfig)
	cd ..

