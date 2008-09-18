#
#   Empire - A multi-player, client/server Internet based war game.
#   Copyright (C) 1986-2008, Dave Pare, Jeff Bailey, Thomas Ruschak,
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
#   See files README, COPYING and CREDITS in the root of the source
#   tree for related information and legal notices.  It is expected
#   that future projects/authors will amend these files as needed.
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
AC_INIT([Empire Client], [4.3.17], [wolfpack@wolfpackempire.com])
AC_CONFIG_SRCDIR([empire.6])
AC_CONFIG_HEADERS([config.h])


### Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL


### Checks for libraries.
AC_SEARCH_LIBS([setupterm], [termlib termcap curses ncurses])
LIB_SOCKET_NSL
# TODO merge this into LIB_SOCKET_NSL?
if test "$ac_cv_search_gethostbyname" = no; then
    dnl Can't search ws2_32 for gethostbyname using AC_SEARCH_LIBS, because
    dnl it requires #include <winsock2.h> to work.
    AC_MSG_CHECKING([for gethostbyname in -lws2_32])
    save_LIBS=$LIBS
    LIBS="-lws2_32 $LIBS"
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <winsock2.h>]],
				    [[gethostbyname ("");]])],
		   [ws2_res=yes], [LIBS=save_LIBS ws2_res=no])
    AC_MSG_RESULT([$ws2_res])
fi

### Checks for header files.


### Checks for typedefs, structures, and compiler characteristics.


### Checks for library functions.

AC_CHECK_FUNCS(getaddrinfo)


### Site configuration

AC_ARG_VAR([EMPIREHOST], [Default host [blitz.wolfpackempire.com]])
test "$EMPIREHOST" || EMPIREHOST=blitz.wolfpackempire.com

AC_ARG_VAR([EMPIREPORT], [Default port [6789]])
test "$EMPIREPORT" || EMPIREPORT=6789


### Output

AC_CONFIG_FILES([Makefile ipglob.c])
AC_OUTPUT
