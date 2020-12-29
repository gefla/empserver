AC_DEFUN([MY_LIB_READLINE], [
    AC_CACHE_CHECK([for a readline compatible library],
		   my_cv_lib_readline, [
	my_lib_readline_save_LIBS="$LIBS"
	for readline_lib in readline edit editline; do
	    for termcap_lib in "" termlib termcap curses ncurses; do
		if test -z "$termcap_lib"; then
		    my_cv_lib_readline="-l$readline_lib"
		else
		    my_cv_lib_readline="-l$readline_lib -l$termcap_lib"
		fi
		LIBS="$my_cv_lib_readline $my_lib_readline_save_LIBS"
		AC_LINK_IFELSE([AC_LANG_CALL([], [add_history])],
			       [break 2], [my_cv_lib_readline=no])
	    done
	done
    ])

    if test "$my_cv_lib_readline" != no; then
	AC_CHECK_HEADER([readline/readline.h], [], [my_cv_lib_readline=no],
			[AC_INCLUDES_DEFAULT])
    fi
    if test "$my_cv_lib_readline" != no; then
	AC_CHECK_HEADER([readline/history.h], [], [my_cv_lib_readline=no],
			[AC_INCLUDES_DEFAULT])
    fi

    if test "$my_cv_lib_readline" = no; then
	LIBS="$my_lib_readline_save_LIBS"
    else
	AC_DEFINE([HAVE_LIBREADLINE], [1],
		  [Define if you have libreadline])
    fi
])

AC_DEFUN([MY_WITH_READLINE], [
    AC_ARG_WITH([readline],
	[AS_HELP_STRING([--with-readline],
	  [support fancy command line editing @<:@default=check@:>@])],
	[],
	[with_readline=check])
    if test "x$with_readline" != xno; then
	MY_LIB_READLINE
	if test "x$my_cv_lib_readline$with_readline" = xnoyes; then
	    AC_MSG_FAILURE([--with-readline was given, but test for readline failed])
	fi
	if test "$my_cv_lib_readline" = no; then
	    with_readline=no
	else
	    with_readline=yes
	fi
    fi])
