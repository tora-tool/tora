dnl mrj
dnl check if we have oracle then test it. otherwise, RESULT no.
AC_DEFUN([TORA_CHECK_ORACLE],
[
  AC_MSG_CHECKING([for oracle])

  dnl the default
  have_oracle=yes

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
  [[  --with-oci-version=[8, 8I, 9I, 10G, 10G_R2]
                          this is the version of the client, not the database.]],
  [
    have_oracle=yes
    oracle_user_otl_ver=$withval
  ], )

  oracle_user_instant=
  AC_ARG_WITH(instant-client,
  [[  --with-instant-client
                          define if compiling against Oracle Instant Client.
                          Disables testing for ORACLE_HOME and tnsnames.]],
  [
    have_oracle=yes
    oracle_user_instant=$withval
  ], )

  cflags_ora_save=$CXXFLAGS
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
  elif test "x$oracle_user_instant" != "x"; then
    dnl user says we're running on the instant client libraries.
    AC_DEFINE(TO_INSTANT_CLIENT, 1, [Define if compiled against Oracle Instant Client])

    if test "x$oracle_user_lib" = "x" && test "x$oracle_user_inc" = "x"; then
      dnl try to find oracle includes for instant client
      dnl these are from the rpm install. they're all i know of so far.

      for dir in `ls /usr/lib/oracle/`; do
        echo "trying $dir" >&5
        if expr $dir \> 10 >/dev/null; then
          oracle_user_otl_ver=10G
        fi
        ora_ldflags="-L/usr/lib/oracle/$dir/client/lib"

        incdir=/usr/include/oracle/$dir/client
        if ! test -d $incdir; then
          AC_MSG_ERROR([$incdir doesn't exist. Please install the sdk package or use --oracle-includes.])
        fi
        ora_cflags="-I$incdir"
        break
      done
    else
      ora_ldflags="-L$oracle_user_lib"
      ora_cflags="-I$oracle_user_inc"
    fi
  elif test "x$ORACLE_HOME" != "x"; then
    AC_MSG_RESULT($ORACLE_HOME)

    dnl try to find oci.h
    ora_check_inc="
      $oracle_user_inc
      $ORACLE_HOME/rdbms/demo
      $ORACLE_HOME/plsql/public
      $ORACLE_HOME/rdbms/public
      $ORACLE_HOME/network/public
      $ORACLE_HOME/sdk/
      $ORACLE_HOME/include/"

    for dir in $ora_check_inc; do
      echo "trying oci.h $dir/oci.h" >&AS_MESSAGE_LOG_FD()
      if test -r $dir/oci.h; then
        ora_cflags="$ora_cflags -I$dir"
        echo "adding -I$dir" >&AS_MESSAGE_LOG_FD()
      fi
    done

    ora_check_lib="
      $oracle_user_lib
      $ORACLE_HOME/lib
      $ORACLE_HOME/lib32
      $ORACLE_HOME/lib64"

    for dir in $ora_check_lib; do
      if test -d $dir; then
        ora_ldflags="$ora_ldflags -L$dir"
      fi
    done
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

  if test "x$ORACLE_HOME" != "x"; then
    dnl check real quick that ORACLE_HOME doesn't end with a slash
    dnl for some stupid reason, the 10g instant client bombs.
    ora_home_oops=`echo $ORACLE_HOME | $AWK '/\/@S|@/ {print "oops"}'`
    if test "$ora_home_oops" = "oops"; then
      AC_MSG_WARN([Your ORACLE_HOME environment variable ends with a
slash (i.e. /). Oracle 10g Instant Client is known to have a problem
with this. If you get the message "otl_initialize failed!" at the
console when running TOra, this is probably why.])
    fi
  fi

  if test $have_oracle = yes; then
    AC_MSG_CHECKING([oci works])
    CXXFLAGS="$CXXFLAGS $ora_cflags"
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
  
      if expr $sqlplus_ver \> 10.2 >/dev/null; then
        otl_ver=10G_R2
      elif expr $sqlplus_ver \> 10 >/dev/null; then
        otl_ver=10G
      elif expr $sqlplus_ver \> 9 >/dev/null; then
        otl_ver=9I
      elif expr $sqlplus_ver \< 8.1 >/dev/null; then
        otl_ver=8
      else
        otl_ver=8I
      fi
    fi
  
    ora_cflags="$ora_cflags -DOTL_ORA${otl_ver}"
  
    # don't change flags for all targets, just export ORA variables.
    CXXFLAGS=$cflags_ora_save
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
