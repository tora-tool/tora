##   -*- mode: autoconf; tab-width: 2; -*-


dnl mrj: set default prefix for tora
AC_DEFUN([mrj_SET_PREFIX],
[
    AC_PREFIX_DEFAULT(/usr/local/tora)
])

AC_DEFUN([TORA_CHECK_MONO],
[
  enable_mono=yes
  AC_DEFINE(TOMONOLITHIC, 1, [True if using monolithic build.])
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

AC_DEFUN([TORA_WINDOWS],
[
  WIN_CXXFLAGS=
  WIN_LDFLAGS=
  case $host_os in
  cygwin* | mingw*)
      WIN_CXXFLAGS="-DQT_DLL -DQ_OS_WIN32 -mwindows"
      WIN_LDFLAGS="-mwindows"
      ;;
  esac

    AC_SUBST(WIN_CXXFLAGS)
    AC_SUBST(WIN_LDFLAGS)
])

AC_DEFUN([TORA_DEBUG],
[
  dnl if we're using gcc, disable function inlining
  AX_CXXFLAGS_GCC_OPTION([-fno-inline])

  AC_ARG_ENABLE(
    debug,
    [  --enable-debug          enable debugging options],
    tora_debug=$enableval,
    tora_debug=no)

  if test $tora_debug != no; then
    tora_debug=yes
    if test "x$ac_cv_prog_cxx_g" = "xyes"; then
      CXXFLAGS="$CXXFLAGS -g -Wall"
      LDFLAGS="$LDFLAGS -g"

      AC_MSG_WARN([Debug enabled. You *must* compile qscintilla with
debug support until this build supports using the builded source. ldd
the finished executable to be sure your binary only depends on the
debug qt libraries.])
    fi
  else
    dnl disable debug testing in qt
    CXXFLAGS="$CXXFLAGS -O2"
    LDFLAGS="$LDFLAGS -DQT_NO_DEBUG"
  fi
])
