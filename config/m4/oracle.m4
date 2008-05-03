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

  oracle_user_instant=
  AC_ARG_WITH(instant-client,
  [[  --with-instant-client=DIR
                          Path to your instant client. If you've unpacked
                          all the zip files to this folder, then this is
                          all you need to use an instant client.
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

    instant_ora_libs="$oracle_user_instant"
    instant_ora_inc="$oracle_user_instant/sdk/include"

    dnl try to find oracle includes for instant client
    if test "x$oracle_user_lib" = "x"; then
      if test "`ls $instant_ora_libs/libclntsh*so 2>/dev/null || echo x`" != "x"; then
        ora_ldflags=-L$instant_ora_libs
      else
        AC_MSG_ERROR([
The path "$instant_ora_libs"/libclntsh*so does not exist.
Because of the way Oracle versions its libraries, you need to create a
symlink to the library. Try something like this:

cd $instant_ora_libs
ln -s libclntsh.so.11.1 libclntsh.so
])
      fi
    else
      ora_ldflags="-L$oracle_user_lib"
    fi

    if test "x$oracle_user_inc" = "x"; then
      if test -d $instant_ora_inc; then
        ora_cflags="-I$instant_ora_inc"
      else
        AC_MSG_ERROR([
$instant_ora_inc doesn't exist.
Please install the instant client sdk package or use --with-oracle-includes=DIR.])
      fi
    else
      ora_cflags="-I$oracle_user_inc"
    fi
  elif test "x$ORACLE_HOME" != "x"; then
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
    dnl for some stupid reason the instant client bombs.
    ora_home_oops=`echo $ORACLE_HOME | $AWK '/\/@S|@/ {print "oops"}'`
    if test "$ora_home_oops" = "oops"; then
      AC_MSG_WARN([
Your ORACLE_HOME environment variable ends with a
slash (i.e. /). Oracle Instant Client is known to have a problem
with this. If you get the message "otl_initialize failed!" at the
console when running TOra, this is probably why.])
    fi
  fi

  if test $have_oracle = yes; then
    CXXFLAGS="$CXXFLAGS $ora_cflags"
    LDFLAGS="$LDFLAGS $ora_ldflags"
    LIBS="$ora_lib"
  
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
      AC_MSG_ERROR([
Couldn't compile and run a simpile OCI app.
Try setting ORACLE_HOME or check config.log.
Also make sure ORACLE_HOME/lib is in /etc/ld.so.conf or LD_LIBRARY_PATH])
    fi
  
    # AM_CONDITIONAL in configure.in uses this variable to enable oracle
    # targets.
    enable_oracle=yes
    AC_MSG_RESULT(yes)

    TORA_CHECK_OCI_VERSION

    # don't change flags for all targets, just export ORA variables.
    CXXFLAGS=$cflags_ora_save
    AC_SUBST(ORACLE_CXXFLAGS, $ora_cflags)
  
    LDFLAGS=$ldflags_ora_save
    AC_SUBST(ORACLE_LDFLAGS, $ora_ldflags)
  
    LIBS=$libs_ora_save
    AC_SUBST(ORACLE_LIBS, $ora_lib)
  fi
])


dnl depends on variables from oracle check. must be called with
dnl appropriate LIBS LDFLAGS CXXFLAGS
AC_DEFUN([TORA_CHECK_OCI_VERSION],
[
  oracle_user_otl_ver=
  AC_ARG_WITH(oci-version,
  [[  --with-oci-version=[8, 8I, 9I, 10G, 10G_R2, 11G]
                          this is the version of the client, not the database.]],
  [
    oracle_user_otl_ver=$withval
  ], )

  if test $have_oracle = yes; then
    AC_MSG_CHECKING([oci version])

    AC_RUN_IFELSE([[
#include <oci.h>
#include <iostream>
#include <fstream>
using namespace std;

int main(int c, char **v) {
    ofstream version;
    version.open("ociversion", ios::out);
    if(!version.is_open()) {
        cerr << "Cannot open ociversion output file";
        exit(1);
    }

    sword major, minor, update, patch, port;
    OCIClientVersion(&major, &minor, &update, &patch, &port);

    switch(major) {
    default:
        exit(1);                /* unknown */

    case 11:
        version << "11G";
        break;

    case 10:
        if(minor >= 2)
            version << "10G_R2";
        else
            version << "10G";
        break;

    case 9:
        version << "9I";
        break;

    case 8:
        if(minor >= 1)
            version << "8I";
        else
            version << "8";
    }

    version.close();
    return 0;
}
    ]], [found_oracle=yes],
    [found_oracle=no], )

    if test $found_oracle = no; then
      if test "x$oracle_user_otl_ver" != "x"; then
        otl_ver=$oracle_user_otl_ver
      else
        dnl try to find using awk and sqlplus
        sqlplus_try="
        $oracle_user_instant/sqlplus
        $ORACLE_HOME/bin/sqlplus
        $ORACLE_HOME/bin/sqlplusO"

        for try in $sqlplus_try; do
          if test -x "$try"; then
            sqlplus="$try"
            break;
          fi
        done

        if test "x${sqlplus}" = "x"; then
          AC_MSG_ERROR([
Couldn't find sqlplus. Please set the Oracle version manually using --with-oci-version.])
        else
          # get oracle oci version. know a better way?
          sqlplus_ver=`$sqlplus -? | $AWK '/Release/ {print @S|@3}'`
          echo "sqlplus_ver: $sqlplus_ver" >&5

          if expr $sqlplus_ver \> 11 >/dev/null; then
            otl_ver=11G
          elif expr $sqlplus_ver \> 10.2 >/dev/null; then
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
      fi
    else
      if test -r "ociversion"; then
        otl_ver="`cat ociversion`"
        rm -f ociversion
      else
        AC_MSG_ERROR([
Couldn't find ociversion output file. Please mail the tora-dev email list
and let us know about this problem.])
      fi
    fi

    AC_MSG_RESULT($otl_ver)
    ora_cflags="$ora_cflags -DOTL_ORA${otl_ver}"
  fi
])
