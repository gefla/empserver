AC_DEFUN([MY_FUNC_MAKECONTEXT],
[
	AC_REQUIRE([AC_CANONICAL_HOST])
	AC_CHECK_FUNCS(makecontext)
	case "$host_os" in
	*irix*|*solaris2.?)
		AC_DEFINE([MAKECONTEXT_SP_HIGH], 1,
			[Define if your makecontext() requires ss_sp at the top of the stack])
		;;
	esac
])
