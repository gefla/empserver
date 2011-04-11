#
#   Empire - A multi-player, client/server Internet based war game.
#   Copyright (C) 1986-2011, Dave Pare, Jeff Bailey, Thomas Ruschak,
#                 Ken Stevens, Steve McClure, Markus Armbruster
#
#   Empire is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#   ---
#
#   See files README, COPYING and CREDITS in the root of the source
#   tree for related information and legal notices.  It is expected
#   that future projects/authors will amend these files as needed.
#
#   ---
#
#   configure.ac: Autoconf input file
#
#   Known contributors to this file:
#      Markus Armbruster, 2005-2010
#
# Process this file with autoconf to produce a configure script.

# Autoconf makes checking for and programming around assorted ancient
# crap relatively painless.  But why bother?  Just rely on C89 and
# POSIX, and when something breaks on some oddball machine, see
# whether it's worth fixing.

AC_PREREQ(2.59)
AC_INIT([Wolfpack Empire], [4.3.27], [wolfpack@wolfpackempire.com], [empire])
AC_CONFIG_SRCDIR([include/combat.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])


### Checks for programs
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_PROG_CC
AC_SUBST(GCC)
AM_PROG_CC_C_O
# Private automake macro, tsk, tsk, tsk...
_AM_DEPENDENCIES(CC)

AC_ARG_VAR(NROFF, [nroff command])
AC_CHECK_PROG(NROFF, groff, [GROFF_NO_SGR= groff -Tascii -U], nroff)

# not really a check for a program, but close enough
if test -d $srcdir/.git; then revctrl=git
else revctrl=
fi
AC_SUBST(revctrl,$revctrl)

# Not a program, but need to check this early
MY_WINDOWS_API


### Checks for libraries
ACX_PTHREAD
LIBS_util="$LIBS"
LIBS="$LIBS_SOCKETS $LIBS"
LIB_SOCKET_NSL
LIBS_server="$LIBS"


### Checks for header files


### Checks for typedefs, structures, and compiler characteristics

AC_C_CHAR_UNSIGNED


### Checks for library functions

AC_CHECK_FUNCS(getaddrinfo getpass)
MY_FUNC_MAKECONTEXT


### Site configuration

# Can't be bothered to make MAXNOC configurable right now.
AC_DEFINE([MAXNOC], [99],
	  [The maximum number of countries])

AC_ARG_VAR([EMPIREHOST], [Default host (client only) [127.0.0.1]])
test "$EMPIREHOST" || EMPIREHOST=127.0.0.1

AC_ARG_VAR([EMPIREPORT], [Default port (client and server) [6665]])
test "$EMPIREPORT" || EMPIREPORT=6665

AC_ARG_WITH([pthread],
	    AS_HELP_STRING([--with-pthread],
			   [use POSIX threads]))

# Select thread package
empthread=
if test "$with_pthread" != no; then
    if test "$acx_pthread_ok" = yes; then
	empthread=POSIX
    fi
fi
if test -z "$with_pthread" || test "$with_pthread" = no; then
    if test "$ac_cv_func_makecontext" = yes; then
	empthread=LWP
    fi
    if test $Windows_API = yes; then
	empthread=Windows
    fi
fi
if test -z "$empthread"; then
    AC_MSG_ERROR([No usable thread package found])
fi
AC_SUBST(empthread)
case $empthread in
LWP)	AC_DEFINE([EMPTH_LWP], 1, [Define to use LWP threads]) ;;
POSIX)	AC_DEFINE([EMPTH_POSIX], 1, [Define to use POSIX threads]) ;;
Windows) AC_DEFINE([EMPTH_W32], 1, [Define to use Windows threads]) ;;
esac

MY_WITH_TERMINFO
LIBS_client="$LIBS"


### Output

LIBS="$LIBS_util"
AC_SUBST(LIBS_client)
AC_SUBST(LIBS_server)

AC_CONFIG_FILES([GNUmakefile])
AC_CONFIG_COMMANDS([stamp-h],
	[case $revctrl in
	git)
	    mkdir -p `cd $srcdir && git ls-files \
		| sed -n 's,/@<:@^/@:>@*$,,gp' | uniq`
	    ;;
	*)
	    mkdir -p `sed s/.*=// <$srcdir/sources.mk | tr ' ' '\012' \
		| sed -n '/\//s,/@<:@^/@:>@*$,,gp'| uniq`
	esac
	mkdir -p info.html info.nr lib
	touch stamp-h],
	[revctrl=$revctrl])
AC_OUTPUT

AC_MSG_NOTICE([])
AC_MSG_NOTICE([-= Configuration summary =-])
AC_MSG_NOTICE([Thread package: $empthread])
AC_MSG_NOTICE([      terminfo: $with_terminfo])
AC_MSG_NOTICE([    EMPIREHOST: $EMPIREHOST])
AC_MSG_NOTICE([    EMPIREPORT: $EMPIREPORT])
