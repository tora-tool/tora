# Process this file with autoconf to produce a configure script.

AC_INIT([tora],[1.3.22],[tora-develop@lists.sourceforge.net])
AC_PREREQ([2.59])
# Every other copy of the package version number gets its value from here
AM_INIT_AUTOMAKE

# create a config.h file (Automake will add -DHAVE_CONFIG_H)
AC_CONFIG_SRCDIR([main.cpp])
AC_CONFIG_HEADER([config.h])
mrj_SET_PREFIX

AC_SUBST(PACKAGE_VERSION)

ISODATE=`date +%Y-%m-%d`
AC_SUBST(ISODATE)

AC_CANONICAL_HOST

# Checks for programs.
AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_CXX

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h sys/param.h sys/time.h time.h sys/mkdev.h sys/sysmacros.h string.h memory.h fcntl.h dirent.h sys/ndir.h ndir.h alloca.h locale.h )

jm_CHECK_TYPE_STRUCT_UTIMBUF
AC_HEADER_MAJOR
AC_FUNC_ALLOCA
AC_FUNC_MALLOC
AC_CHECK_FUNCS(snprintf vsnprintf)
AC_HEADER_STDBOOL
AC_STRUCT_TM
AC_STRUCT_ST_BLOCKS
AC_FUNC_CLOSEDIR_VOID
AC_TYPE_SIZE_T

# for NLS support. Call them in this order!
# WITH_NLS is for the po files
# AM_KDE_WITH_NLS

TORA_CHECK_PLUGIN
TORA_CHECK_MONO
TORA_CHECK_LINK_LIBS
TORA_LIBTOOL_ALL
TORA_HAVE_KDE
MRJ_CHECK_ORACLE
TORA_CHECK_QSCINTILLA
AX_PATH_LIB_PCRE
TORA_CHECK_PUTENV

# Makefile.am uses these
AM_CONDITIONAL(ENABLE_MONO, test "$enable_mono" = yes)
AM_CONDITIONAL(HAVE_ORACLE, test "$enable_oracle" = yes)

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([rpm/tora.spec])
AC_OUTPUT
