#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.5])
AC_INIT([fucheck], [1.0], [someone@someserver.net])
AM_INIT_AUTOMAKE(fucheck, 2.5)
AC_CONFIG_SRCDIR([src/fucheck.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([m], [cos])
AC_CHECK_LIB([gslcblas], [cblas_dgemm])
AC_CHECK_LIB([gsl], [gsl_blas_dgemm], [], [
	      echo "Error! You need to habe GSL installed."
	      exit -1
	      ])

PKG_CHECK_MODULES([LIBXML2], [libxml-2.0])
AC_SUBST(LIBXML2_CFLAGS)
AC_SUBST(LIBXML2_LIBS)

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC

AC_OUTPUT(Makefile src/Makefile man/Makefile)
