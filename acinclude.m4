##   -*- mode: autoconf; tab-width: 2; -*-

m4_include([qtkde.m4])

dnl mrj: set default prefix for tora
AC_DEFUN(mrj_SET_PREFIX,
[
    AC_PREFIX_DEFAULT(/usr/local/tora)
])

AC_DEFUN(TORA_CHECK_PLUGIN,
[
  AC_MSG_CHECKING([enable plugin support])
  AC_ARG_ENABLE(plugin,
  [  --enable-plugin         enable support for plugins. (default no)],
  [ enable_plugin=yes ],
  [ enable_plugin=no ])

  if test $enable_plugin = no; then
    AC_DEFINE(TOMONOLITHIC, 1, [True if using monolithic build.])
  fi

  AC_MSG_RESULT($enable_plugin)
])

AC_DEFUN(TORA_LIBTOOL_ALL,
[
  AC_LIBTOOL_DLOPEN
  dnl AC_LIB_LTDL
  dnl AC_SUBST(LTDLINCL)
  dnl AC_SUBST(LIBLTDL)
  dnl AC_DEFINE(HAVE_LTDL, 1, [Always set when using autotools build.])
  dnl check for dlopen, must be called before AC_PROG_LIBTOOL
  AC_PROG_LIBTOOL
  AC_SUBST(LIBTOOL_DEPS)
])

AC_DEFUN(TORA_CHECK_PUTENV,
[
  AC_CACHE_CHECK([for putenv], tora_cv_sys_putenv,
    [AC_TRY_COMPILE(
      [
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
      ],
      [putenv("test=test");],
      tora_cv_sys_putenv=yes,
      tora_cv_sys_putenv=no)
    ])

  if test $tora_cv_sys_putenv = yes; then
    AC_DEFINE(TO_HAS_SETENV, 1, [Define if you have putenv])
  fi
])

#serial 3

dnl From Jim Meyering

dnl Define HAVE_STRUCT_UTIMBUF if `struct utimbuf' is declared --
dnl usually in <utime.h>.
dnl Some systems have utime.h but don't declare the struct anywhere.

AC_DEFUN(jm_CHECK_TYPE_STRUCT_UTIMBUF,
[
  AC_CHECK_HEADERS(utime.h)
  AC_REQUIRE([AC_HEADER_TIME])
  AC_CACHE_CHECK([for struct utimbuf], fu_cv_sys_struct_utimbuf,
    [AC_TRY_COMPILE(
      [
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
      ],
      [static struct utimbuf x; x.actime = x.modtime;],
      fu_cv_sys_struct_utimbuf=yes,
      fu_cv_sys_struct_utimbuf=no)
    ])

  if test $fu_cv_sys_struct_utimbuf = yes; then
    AC_DEFINE_UNQUOTED(HAVE_STRUCT_UTIMBUF, 1,
[Define if struct utimbuf is declared -- usually in <utime.h>.
   Some systems have utime.h but don't declare the struct anywhere. ])
  fi
])


dnl mrj
dnl check if we have oracle then test it. otherwise, RESULT no.
AC_DEFUN(MRJ_CHECK_ORACLE,
[
  AC_MSG_CHECKING([for oracle])

  dnl the default, true if ORACLE_HOME is set
  test "x$ORACLE_HOME" != "x" && have_oracle=yes

  AC_ARG_WITH(oracle,
  [  --with-oracle=DIR       enable support for Oracle (default ORACLE_HOME)],
  [
    if test $withval = no; then
      have_oracle=no
    elif test $withval != yes; then
      ORACLE_HOME=$withval
    fi
  ], )

  oracle_user_inc=
  AC_ARG_WITH(oracle-includes,
  [  --with-oracle-includes=DIR
                          set oracle include dir (default ORACLE_HOME/subdirs)],
  [
    have_oracle=yes
    oracle_user_inc=$withval
  ], )

  oracle_user_lib=
  AC_ARG_WITH(oracle-libraries,
  [  --with-oracle-libraries=DIR
                          set oracle lib dir (default ORACLE_HOME/lib)],
  [
    have_oracle=yes
    oracle_user_lib=$withval
  ], )

  oracle_user_otl_ver=
  AC_ARG_WITH(oci-version,
  [[  --with-oci-version=[8, 8I, 9I, 10G]
                          this is the version of the client, not the database.]],
  [
    oracle_user_otl_ver=$withval
  ], )

  cflags_ora_save=$CFLAGS
  ldflags_ora_save=$LDFLAGS
  libs_ora_save=$LIBS

  ora_cflags=
  ora_libdir=
  ora_ldflags=
  ora_lib=-lclntsh

  if test $have_oracle = no; then
    dnl yeah, this is backwards.
    AC_DEFINE(TO_NO_ORACLE, 1, [Define if you do _not_ have Oracle.])
    AC_MSG_RESULT(no)
  elif test "x$ORACLE_HOME" != "x"; then
    AC_MSG_RESULT($ORACLE_HOME)

    dnl try to find oracle includes
    ora_check_inc="
      $ORACLE_HOME/rdbms/demo
      $ORACLE_HOME/plsql/public
      $ORACLE_HOME/rdbms/public
      $ORACLE_HOME/network/public
      $ORACLE_HOME/sdk/
      $ORACLE_HOME/include/"

    for dir in $ora_check_inc; do
      if test -d $dir; then
        ora_cflags="$ora_cflags -I$dir"
      fi
    done

    ora_check_lib="
      $ORACLE_HOME/lib
      $ORACLE_HOME/lib32
      $ORACLE_HOME/lib64"

    for dir in $ora_check_lib; do
      if test -d $dir; then
        ora_ldflags="$ora_ldflags -L$dir"
      fi
    done

    dnl check real quick that ORACLE_HOME doesn't end with a slash
    dnl for some stupid reason, the 10g instant client bombs.
    ora_home_oops=`echo $ORACLE_HOME | $AWK '/\/@S|@/ {print "oops"}'`
    if test "$ora_home_oops" = "oops"; then
      AC_MSG_WARN([Your ORACLE_HOME environment variable ends with a
slash (i.e. /). Oracle 10g Instant Client is known to have a problem
with this. If you get the message "otl_initialize failed!" at the
console when running TOra, this is probably why.])
    fi
  else
    dnl test if we have includes or libraries
    if test -z "$oracle_user_lib" || test -z "$oracle_user_inc"; then
       AC_MSG_WARN(no)
       have_oracle=no
     else
      ora_ldflags="-L$oracle_user_lib"
      ora_cflags="-I$oracle_user_inc"
    fi
  fi

if test $have_oracle = yes; then
  AC_MSG_CHECKING([oci works])
  CFLAGS="$CFLAGS $ora_cflags"
  LDFLAGS="$LDFLAGS $ora_ldflags"
  LIBS="$ora_lib"

  # i pulled this from one of the examples in the demo dir.
  AC_RUN_IFELSE([[
    #include <oci.h>
    Lda_Def lda;
    ub4     hda [HDA_SIZE/(sizeof(ub4))];

    int main(int c, char **v) {
      return 0;
    }
  ]], [found_oracle=yes],
  [found_oracle=no], )

  if test $found_oracle = no; then
    AC_MSG_ERROR([Couldn't compile and run a simpile OCI app.
    Try setting ORACLE_HOME or check config.log.
    Otherwise, make sure ORACLE_HOME/lib is in /etc/ld.so.conf or LD_LIBRARY_PATH])
  fi

  sqlplus=
  if test -x "$ORACLE_HOME/bin/sqlplus"; then
    sqlplus="$ORACLE_HOME/bin/sqlplus"
  fi
  if test "x${sqlplus}" = "x"; then
    if test -x "$ORACLE_HOME/bin/sqlplusO"; then
      sqlplus="$ORACLE_HOME/bin/sqlplusO"
    fi
  fi

  if test "x$oracle_user_otl_ver" != "x"; then
    otl_ver=$oracle_user_otl_ver
  elif test "x${sqlplus}" = "x"; then
    AC_MSG_ERROR([Couldn't find sqlplus. Set the Oracle version manually.])
  else
    # get oracle oci version. know a better way?
    sqlplus_ver=`$sqlplus -? | $AWK '/Release/ {print @S|@3}'`
    echo "sqlplus_ver: $sqlplus_ver" >&5

    dnl you have to test '10.1.0.3.0' is > 10
    if expr $sqlplus_ver \> 10 >/dev/null; then
      dnl our version of otl doesn't have 10g defined yet
      otl_ver=10G
    elif expr $sqlplus_ver \> 9 >/dev/null; then
      otl_ver=9I
    elif expr $sqlplus_ver \< 8.1 >/dev/null; then
      otl_ver=8
    else
      otl_ver=8I
    fi
  fi

  ora_cflags="$ora_cflags -DOTL_ORA${otl_ver} -DOTL_ORA_TIMESTAMP -DOTL_ANSI_CPP"

  # don't change flags for all targets, just export ORA variables.
  CFLAGS=$cflags_ora_save
  AC_SUBST(ORACLE_CXXFLAGS, $ora_cflags)

  LDFLAGS=$ldflags_ora_save
  AC_SUBST(ORACLE_LDFLAGS, $ora_ldflags)

  LIBS=$libs_ora_save
  AC_SUBST(ORACLE_LIBS, $ora_lib)

  # AM_CONDITIONAL in configure.in uses this variable to enable oracle
  # targets.
  enable_oracle=yes
  AC_MSG_RESULT(yes)
fi
])


dnl http://autoconf-archive.cryp.to/ax_path_lib_pcre.html
AC_DEFUN([AX_PATH_LIB_PCRE],[dnl
AC_MSG_CHECKING([lib pcre])
AC_ARG_WITH(pcre,
[  --with-pcre[[=prefix]]    compile xmlpcre part (via libpcre check)],,
     with_pcre="yes")
if test ".$with_pcre" = ".no" ; then
  AC_MSG_RESULT([disabled])
  m4_ifval($2,$2)
else
  AC_MSG_RESULT([(testing)])
  AC_CHECK_LIB(pcre, pcre_study)
  if test "$ac_cv_lib_pcre_pcre_study" = "yes" ; then
     PCRE_LIBS="-lpcre"
     AC_MSG_CHECKING([lib pcre])
     AC_MSG_RESULT([$PCRE_LIBS])
     m4_ifval($1,$1)
  else
     OLDLDFLAGS="$LDFLAGS" ; LDFLAGS="$LDFLAGS -L$with_pcre/lib"
     OLDCPPFLAGS="$CPPFLAGS" ; CPPFLAGS="$CPPFLAGS -I$with_pcre/include"
     AC_CHECK_LIB(pcre, pcre_compile)
     CPPFLAGS="$OLDCPPFLAGS"
     LDFLAGS="$OLDLDFLAGS"
     if test "$ac_cv_lib_pcre_pcre_compile" = "yes" ; then
        AC_MSG_RESULT(.setting PCRE_LIBS -L$with_pcre/lib -lpcre)
        PCRE_LIBS="-L$with_pcre/lib -lpcre"
        test -d "$with_pcre/include" && PCRE_CFLAGS="-I$with_pcre/include"
        AC_MSG_CHECKING([lib pcre])
        AC_MSG_RESULT([$PCRE_LIBS])
        m4_ifval($1,$1)
     else
        AC_MSG_CHECKING([lib pcre])
        AC_MSG_RESULT([no, (WARNING)])
        m4_ifval($2,$2)
     fi
  fi
fi
AC_SUBST([PCRE_LIBS], $PCRE_LIBS)
AC_SUBST([PCRE_CFLAGS], $PCRE_CFLAGS)
])
