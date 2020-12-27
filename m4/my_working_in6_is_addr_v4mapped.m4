AC_DEFUN([MY_WORKING_IN6_IS_ADDR_V4MAPPED], [
    AC_CACHE_CHECK([whether IN6_IS_ADDR_V4MAPPED works],
		   my_cv_func_in6_is_addr_v4mapped, [
	AC_COMPILE_IFELSE(
	    [AC_LANG_SOURCE([[#include <netinet/in.h>
int foo(struct sockaddr_in6 *sap6)
{
    return IN6_IS_ADDR_V4MAPPED(&sap6->sin6_addr);
}]])],
	    [my_cv_func_in6_is_addr_v4mapped=yes],
	    [my_cv_func_in6_is_addr_v4mapped=no])])
    AC_CHECK_MEMBERS([struct sockaddr_in6.sin6_addr.s6_addr],
		     [my_member_sockaddr_in6_sin6_addr_s6_addr=yes],
		     [my_member_sockaddr_in6_sin6_addr_s6_addr=no],
		     [[#include <netinet/in.h>]])
    if test "x$my_cv_func_in6_is_addr_v4mapped$my_member_sockaddr_in6_sin6_addr_s6_addr" = xyesyes; then
	AC_DEFINE([HAVE_WORKING_IN6_IS_ADDR_V4MAPPED], [1],
		  [Define if you have a working IN6_IS_ADDR_V4MAPPED()])
    fi
])
