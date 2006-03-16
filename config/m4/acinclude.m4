##   -*- mode: autoconf; tab-width: 2; -*-


dnl mrj: set default prefix for tora
AC_DEFUN([mrj_SET_PREFIX],
[
    AC_PREFIX_DEFAULT(/usr/local/tora)
])

AC_DEFUN([TORA_CHECK_PLUGIN],
[
  AC_MSG_CHECKING([enable plugin support])
  AC_ARG_ENABLE(plugin,
  [  --enable-plugin         enable support for plugins. (default no)],
  [ enable_plugin=yes ],
  [ enable_plugin=no ])

  AC_MSG_RESULT($enable_plugin)
])

AC_DEFUN([TORA_CHECK_MONO],
[
  AC_MSG_CHECKING([if monolithic build])
  AC_ARG_ENABLE(mono,
  [  --enable-mono           build monolithic (default no)],
  [ enable_mono=yes ],
  [ enable_mono=no ])

  if test $enable_mono = no; then
    enable_plugin=no
  fi

  if test $enable_plugin != yes; then
    AC_DEFINE(TOMONOLITHIC, 1, [True if using monolithic build.])
  fi

  AC_MSG_RESULT($enable_mono)
])

dnl check if we need to link against our libtool libraries
AC_DEFUN([TORA_CHECK_LINK_LIBS],
[
  AC_MSG_CHECKING([whether to link plugins into binary])

  if test $enable_plugin = no && test $enable_mono = no; then
    AC_MSG_RESULT([need link])
    TORA_LINK_OTHERS=lib_LTLIBRARIES
    AC_SUBST(TORA_LINK_OTHERS)
  else
    AC_MSG_RESULT(no)
    TORA_LINK_OTHERS=none
    AC_SUBST(TORA_LINK_OTHERS)
  fi
])

AC_DEFUN([TORA_LIBTOOL_ALL],
[
  AC_LIBTOOL_DLOPEN
  dnl AC_LIB_LTDL
  dnl AC_SUBST(LTDLINCL)
  dnl AC_SUBST(LIBLTDL)
  dnl AC_DEFINE(HAVE_LTDL, 1, [Always set when using autotools build.])
  dnl check for dlopen, must be called before AC_PROG_LIBTOOL
  AC_PROG_LIBTOOL
  AC_SUBST([LIBTOOL_DEPS])
])

#serial 3

dnl From Jim Meyering

dnl Define HAVE_STRUCT_UTIMBUF if `struct utimbuf' is declared --
dnl usually in <utime.h>.
dnl Some systems have utime.h but don't declare the struct anywhere.

AC_DEFUN([jm_CHECK_TYPE_STRUCT_UTIMBUF],
[
  AC_CHECK_HEADERS(utime.h)
  AC_REQUIRE([AC_HEADER_TIME])
  AC_CACHE_CHECK([for struct utimbuf], fu_cv_sys_struct_utimbuf,
    [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#ifdef HAVE_UTIME_H
# include <utime.h>
#endif
      ]],
      [[static struct utimbuf x; x.actime = x.modtime;]])],
      fu_cv_sys_struct_utimbuf=yes,
      fu_cv_sys_struct_utimbuf=no)
    ])

  if test $fu_cv_sys_struct_utimbuf = yes; then
    AC_DEFINE_UNQUOTED(HAVE_STRUCT_UTIMBUF, 1,
[Define if struct utimbuf is declared -- usually in <utime.h>.
   Some systems have utime.h but don't declare the struct anywhere. ])
  fi
])

