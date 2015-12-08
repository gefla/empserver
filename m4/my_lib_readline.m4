AC_DEFUN([MY_LIB_READLINE], [
    have_readline=no
    for readline_lib in readline edit editline; do
	for termcap_lib in "" termlib termcap curses ncurses; do
	  AC_CHECK_LIB([$readline_lib], [add_history],
	      [have_readline=yes; break 2], [], [$termcap_lib])
	done
    done

    if test "$have_readline" = yes; then
	AC_CHECK_HEADER([readline/readline.h], [], [have_readline=no],
			[AC_INCLUDES_DEFAULT])
	AC_CHECK_HEADER([readline/history.h], [], [have_readline=no],
			[AC_INCLUDES_DEFAULT])
    fi

    if test "$have_readline" = yes; then
	if test "x$termcap_lib" != x; then
	    LIBS="-l$termcap_lib $LIBS"
	fi
	LIBS="-l$readline_lib $LIBS"
	AC_DEFINE([HAVE_LIBREADLINE], [1],
		  [Define if you have libreadline])
    fi
])

AC_DEFUN([MY_WITH_READLINE],
[
  AC_ARG_WITH([readline],
    [AS_HELP_STRING([--with-readline],
      [support fancy command line editing @<:@default=check@:>@])],
    [],
    [with_readline=check])
  if test "x$with_readline" != xno; then
      MY_LIB_READLINE
      if test "x$have_readline$with_readline" = xnoyes; then
	  AC_MSG_FAILURE([--with-readline was given, but test for readline failed])
      fi
      with_readline="$have_readline"
  fi])
