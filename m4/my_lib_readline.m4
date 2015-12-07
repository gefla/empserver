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
