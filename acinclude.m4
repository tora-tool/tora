##   -*- autoconf -*-

m4_include([qtkde.m4])

dnl mrj: set default prefix for tora
AC_DEFUN(mrj_SET_PREFIX,
[
    AC_PREFIX_DEFAULT(/usr/local/tora)
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

  have_oracle=yes
  AC_ARG_WITH(oracle,
  [  --with-oracle=dir       enable support for Oracle (default ORACLE_HOME)],
  [
    if test $withval = no; then
      have_oracle=no
    elif test $withval != yes; then
      ORACLE_HOME=$withval
    fi
  ], )

if test $have_oracle = no || test "x$ORACLE_HOME" = "x"; then
  AC_MSG_WARN(ORACLE_HOME not set.)
  dnl yeah, this is backwards.
  AC_DEFINE(TO_NO_ORACLE, 1, [Define if you do *not* have Oracle.])
else
  AC_MSG_RESULT($ORACLE_HOME)

  AC_MSG_CHECKING([oci works])
  cflags_ora_save=$CFLAGS
  ora_cflags="-I$ORACLE_HOME/rdbms/demo -I$ORACLE_HOME/plsql/public -I$ORACLE_HOME/rdbms/public -I$ORACLE_HOME/network/public"
  CFLAGS="$CFLAGS $ora_cflags"

  ldflags_ora_save=$LDFLAGS
  ora_ldflags="-L$ORACLE_HOME/lib -lclntsh"
  LDFLAGS="$LDFLAGS $ora_ldflags"

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

  if test "x${sqlplus}" = "x"; then
    AC_MSG_WARN([Couldn't find sqlplus. Set the Oracle version manually.])
  else
    # get oracle oci version. know a better way?
    sqlplus_ver=`$sqlplus -? | $AWK '/Release/ {print @S|@3}'`
    if expr $sqlplus_ver \> 9 >/dev/null; then
      otl_ver=9I
    elif expr $sqlplus_ver \< 8.1 >/dev/null; then
      otl_ver=8
    else
      otl_ver=8I
    fi

    ora_cflags="$ora_cflags -DOTL_ORA${otl_ver} -DOTL_ORA_TIMESTAMP -DOTL_ANSI_CPP"
  fi

  # don't change flags for all targets, just export ORA variables.
  CFLAGS=$cflags_ora_save
  AC_SUBST(ORACLE_CXXFLAGS, $ora_cflags)

  LDFLAGS=$ldflags_ora_save
  AC_SUBST(ORACLE_LDFLAGS, $ora_ldflags)

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
AC_SUBST([PCRE_LIBS])
AC_SUBST([PCRE_CFLAGS])
])
