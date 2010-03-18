dnl The preffered configure.ac layout from autoconfig documention:
dnl Autoconf requirements
dnl Obligatory at the beginning `AC_INIT(PACKAGE, VERSION, BUG-REPORT-ADDRESS)'
dnl information on the package
AC_PREREQ([2.64])
AC_INIT([unotify], [0.1])
AC_LANG(C++)
AC_CONFIG_SRCDIR([unotify.cc])
AC_CONFIG_HEADERS([config.h])

dnl checks for programs
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AC_PROG_CXX

AC_ARG_ENABLE(debug,   [  --enable-debug          compile with debugging support],,enable_debug=no)
if test "x$enable_debug" = "xyes" ; then
	AC_DEFINE(DEBUG, 1, [Define if debugging is enabled.])
fi

CFLAGS="$CFLAGS -D_XOPEN_SOURCE=600"
AC_SUBST(CFLAGS)

dnl checks for libraries
PKG_CHECK_MODULES(LIBNOTIFYMM, libnotifymm-1.0 >= 0.6.1)
AC_SUBST(LIBNOTIFYMM_CFLAGS)
AC_SUBST(LIBNOTIFYMM_LIBS)

PKG_CHECK_MODULES(GTKMM, gtkmm-2.4 >= 1:2.18.2.1)
AC_SUBST(GTKMM_CFLAGS)
AC_SUBST(GTKMM_LIBS)

dnl checks for header files
AC_CHECK_HEADERS([sys/inotify.h])
dnl checks for types
dnl checks for structures
dnl checks for compiler characteristics
dnl checks for library functions
dnl checks for system services

dnl `AC_CONFIG_FILES([FILE...])'
AC_CONFIG_FILES([Makefile])

dnl Obligatory at the end `AC_OUTPUT'
AC_OUTPUT