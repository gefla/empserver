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
#   ---
# 
#   configure.ac: Autoconf input file
#  
#   Known contributors to this file:
#      Markus Armbruster, 2005
# 
# Process this file with autoconf to produce a configure script.

# Autoconf makes checking for and programming around assorted ancient
# crap relatively painless.  But why bother?  Just rely on C89 and
# POSIX, and when something breaks on some oddball machine, see
# whether it's worth fixing.

AC_PREREQ(2.59)
AC_INIT([Empire], [4.3.0], [wolfpack@wolfpackempire.com])
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

# not really a check for a program, but close enough
if test -d $srcdir/CVS; then cvs_controlled=yes; else cvs_controlled=no; fi
AC_SUBST(cvs_controlled,$cvs_controlled)


### Checks for libraries
ACX_PTHREAD
LIB_SOCKET_NSL

# TODO turn this into a macro?
save_LIBS=$LIBS
AC_SEARCH_LIBS([setupterm], [termlib termcap])
termlibs=`echo $LIBS | sed s/\`echo $save_LIBS | sed 's/././g'\`'$//'`
AC_SUBST(termlibs)
LIBS=$save_LIBS


### Checks for header files


### Checks for typedefs, structures, and compiler characteristics


### Checks for library functions

MY_FUNC_MAKECONTEXT


### Site configuration

AC_ARG_VAR([EMPIREHOST], [Default host (client only) [127.0.0.1]])
test "$EMPIREHOST" || EMPIREHOST=127.0.0.1

AC_ARG_VAR([EMPIREPORT], [Default port (client and server) [6665]])
test "$EMPIREPORT" || EMPIREPORT=6665

AC_ARG_WITH([pthread],
	    AS_HELP_STRING([--with-pthread],
			   [use POSIX threads]))

case "$ac_cv_func_makecontext$acx_pthread_ok" in
yesyes)
    if test -z "$with_pthread" || test "$with_pthread" = no; then
        empthread=LWP
    else
        empthread=POSIX
    fi
    ;;
yes*)
    empthread=LWP ;;
*yes)
    empthread=POSIX ;;
*)
    AC_MSG_ERROR([No usable thread package found])
esac
AC_SUBST(empthread)
AC_MSG_NOTICE([Using $empthread threads])

op=
if test "$with_pthread" = no; then
    op='=='
elif test "$with_pthread"; then
    op='!='
fi
if test "$op" && test "$empthread" $op POSIX; then
    AC_MSG_WARN([Ignoring --with-pthread=$with_pthread])
fi


### Output

AC_CONFIG_FILES([GNUmakefile])
AC_CONFIG_COMMANDS([stamp-h],
	[if test $cvs_controlled = yes; then
	    mkdir -p `cd $srcdir && $AWK -f src/scripts/cvsfiles.awk \
		| sed -n '/\//s,/@<:@^/@:>@*$,,gp'| uniq`
	elif test -f $srcdir/sources.mk; then
	    mkdir -p `sed s/.*=// <$srcdir/sources.mk \
		| sed -n '/\//s,/@<:@^/@:>@*$,,gp'| uniq`
	fi
	>stamp-h],
	[cvs_controlled=$cvs_controlled; AWK=$AWK])
AC_OUTPUT
