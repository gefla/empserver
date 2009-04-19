AC_DEFUN([MY_CURSES_TERMINFO],
[
	AC_SEARCH_LIBS([setupterm], [termlib termcap curses ncurses],
		[have_terminfo=yes], [have_terminfo=no])
	AC_CHECK_HEADER([curses.h], , [have_terminfo=no])
	AC_CHECK_HEADER([term.h], , [have_terminfo=no])
	if test "$have_terminfo" = yes
	then AC_DEFINE([HAVE_CURSES_TERMINFO], 1,
		[Define if you have the curses interface to terminfo])
	fi
])

AC_DEFUN([MY_WITH_TERMINFO],
[
	AC_ARG_WITH([terminfo],
		AS_HELP_STRING([--with-terminfo],
			[use terminfo for highlighting (default check)]))
	if test "x$with_terminfo" != xno; then
		MY_CURSES_TERMINFO
		if test "$have_terminfo$with_terminfo" = noyes
		then AC_MSG_FAILURE([Can't satisfy --with-terminfo])
		fi
		with_terminfo="$have_terminfo"
	fi
])
