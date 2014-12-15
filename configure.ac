#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([auto_utilization], [1.0.1], [kun.he@cs2c.com.cn])
AC_CONFIG_SRCDIR([src/memory_limit/memory_limit.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([pthread], [pthread_create])
AC_PROG_RANLIB

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h strings.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([gettimeofday memset strstr])

AC_CONFIG_FILES([Makefile
src/cpu_limit/Makefile
src/memory_limit/Makefile])

AM_INIT_AUTOMAKE

AC_OUTPUT
