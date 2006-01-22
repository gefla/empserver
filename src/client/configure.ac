#                                               -*- Autoconf -*-
# Copyright (C) 2005 Markus Armbruster
# Process this file with autoconf to produce a configure script.

# Autoconf makes checking for and programming around assorted ancient
# crap relatively painless.  But why bother?  Just rely on C89 and
# POSIX, and when something breaks on some oddball machine, see
# whether it's worth fixing.

AC_PREREQ(2.59)
AC_INIT([Empire Client], [4.3.0], [wolfpack@wolfpackempire.com])
AC_CONFIG_SRCDIR([empire.6])
AC_CONFIG_HEADERS([config.h])


### Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL


### Checks for libraries.
LIB_SOCKET_NSL
AC_SEARCH_LIBS([setupterm], [termlib termcap])
# TODO merge this into LIB_SOCKET_NSL?
case "$host_os" in
*mingw32)
    LIBS="-lws2_32 $LIBS" ;;
esac


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
