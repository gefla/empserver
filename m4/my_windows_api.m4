AC_DEFUN([MY_WINDOWS_API],
[
	AC_MSG_CHECKING([whether compiling for Windows API])
	AC_COMPILE_IFELSE(
		[AC_LANG_SOURCE([[#ifdef _WIN32
#Here's a nickel, kid
#endif]])],
		[Windows_API=no],
		[Windows_API=yes])
	AC_MSG_RESULT([$Windows_API])
	if test $Windows_API = yes; then
		AC_DEFINE([WINDOWS], 1, [Define if compiling for Windows API])
		LIBS_SOCKETS="-lws2_32"
	else
		LIBS_SOCKETS=
	fi
	AC_SUBST([LIBS_SOCKETS])
])
