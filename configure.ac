# Process this file with autoconf to produce a configure script.

AC_INIT([tora],[2.1.1],[tora-develop@lists.sourceforge.net])
AC_PREREQ([2.59])
AC_CONFIG_AUX_DIR(config)
# Every other copy of the package version number gets its value from here
AM_INIT_AUTOMAKE

# create a config.h file (Automake will add -DHAVE_CONFIG_H)
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADER([config.h])
mrj_SET_PREFIX

AC_SUBST(PACKAGE_VERSION)

ISODATE=`date +%Y-%m-%d`
AC_SUBST(ISODATE)

AC_CANONICAL_HOST

# Checks for programs.
AC_PROG_INSTALL
# AC_PROG_CC
AC_PROG_CXX
AC_CHECK_PROG([MAKEINFO], [makeinfo], [makeinfo], [no])
if test "${MAKEINFO}" = no; then
   AC_MSG_ERROR([makeinfo not found])
fi
# Checks for header files.
# AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h sys/param.h sys/time.h time.h sys/mkdev.h sys/sysmacros.h string.h memory.h fcntl.h dirent.h sys/ndir.h ndir.h alloca.h locale.h utime.h postgresql/libpq-fe.h)
AC_HEADER_MAJOR
AC_FUNC_ALLOCA
AC_FUNC_MALLOC
AC_CHECK_FUNCS(snprintf vsnprintf setenv)
AC_HEADER_STDBOOL
AC_STRUCT_TM
AC_STRUCT_ST_BLOCKS
AC_FUNC_CLOSEDIR_VOID
AC_TYPE_SIZE_T

AC_VALIDIFY_CXXFLAGS
TORA_DEBUG
TORA_CHECK_MONO
TORA_LIBTOOL_ALL
TORA_WINDOWS
AC_PATH_QT
AC_PATH_MOC
AC_PATH_UIC
AC_PATH_RCC
TORA_CHECK_POSTGRES
TORA_CHECK_ORACLE
TORA_CHECK_QSCINTILLA
AX_PATH_LIB_PCRE
AM_PATH_CPPUNIT([1.10.2], found_cppunit=yes)
AM_CONDITIONAL([CPPUNIT], [test "x$found_cppunit" = "xyes"])

# Makefile.am uses these
AM_CONDITIONAL(HAVE_ORACLE, test "$enable_oracle" = yes)

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([utils/Makefile])
AC_CONFIG_FILES([utils/chex/Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([test/Makefile])
AC_CONFIG_FILES([test/configuration/Makefile])
AC_CONFIG_FILES([doc/Makefile])
AC_CONFIG_FILES([doc/help/Makefile])
AC_CONFIG_FILES([rpm/tora.spec])
AC_OUTPUT
