##   -*- mode: autoconf; tab-width: 2; -*-

dnl mrj: set default prefix for tora
AC_DEFUN([TORA_CHECK_POSTGRES],
[
  AC_CHECK_DECL([PQcancel],
                [ac_pg_cancel=yes],
                [ac_pg_cancel=no],
                [#include <postgresql/libpq-fe.h>])

  if test $ac_pg_cancel = yes; then
    AC_DEFINE([LIBPQ_DECL_CANCEL], 1, [Define if libpg-fe.h declares PQcancel.])
		AC_SUBST(PQ_LIBS, -lpq)
  fi
])
