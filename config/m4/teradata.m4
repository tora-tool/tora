AC_DEFUN([TORA_CHECK_TERADATA],
[
  AC_MSG_CHECKING([for teradata])

  dnl the default
  have_teradata=no

  AC_ARG_WITH(teradata,
  [  --with-teradata=DIR     enable support for Teradata (default NO)],
  [
    have_teradata=$withval
  ], )

  teradata_user_inc=
  AC_ARG_WITH(teradata-includes,
  [  --with-teradata-includes=DIR
                          set teradata include dir],
  [
    have_teradata=yes
    teradata_user_inc=$withval
  ], )

  teradata_user_lib=
  AC_ARG_WITH(teradata-libraries,
  [  --with-teradata-libraries=DIR
                          set teradata lib dir],
  [
    have_teradata=yes
    teradata_user_lib=$withval
  ], )

  cflags_td_save=$CXXFLAGS
  ldflags_td_save=$LDFLAGS
  libs_td_save=$LIBS

  td_cflags=
  td_ldflags=
  td_lib="-lcliv2 -ltdusr"

  if test "x$teradata_user_lib" != "x"; then
    td_ldflags="-L$teradata_user_lib"
  fi
  if test "x$teradata_user_inc" != "x"; then
    td_cflags="-I$teradata_user_inc"
  fi

  if test $have_teradata = no; then
    AC_MSG_RESULT(no)
  else
    AC_DEFINE(TO_HAVE_TERADATA, 1, [Define if Teradata support is enabled.])

    CXXFLAGS="$CXXFLAGS $td_cflags"
    LDFLAGS="$LDFLAGS $td_ldflags"
    LIBS="$td_lib"
  
    AC_RUN_IFELSE([[
#include <stdlib.h>

#include <coptypes.h>       /* Teradata include files */
#include <coperr.h>
#include <dbcarea.h>
#include <parcel.h>
#include <dbchqep.h>

      int main(int c, char **v) {
        static Int32  result;
        static char   cnta[4];
        struct DBCAREA *dbcp;

        dbcp = (DBCAREA *) malloc(sizeof(struct DBCAREA));
        dbcp->total_len = sizeof(struct DBCAREA);
        DBCHINI(&result,cnta, dbcp);

        return 0;
      }
    ]], [found_td=yes],
    [found_td=no], )
  
    if test $found_td = no; then
      AC_MSG_ERROR([
Couldn't compile and run a Teradata client.])
    fi
  
    # AM_CONDITIONAL in configure.in uses this variable to enable td
    # targets.
    enable_teradata=yes
    AC_MSG_RESULT(yes)

    # don't change flags for all targets, just export TD variables.
    CXXFLAGS=$cflags_td_save
    AC_SUBST(TERADATA_CXXFLAGS, $td_cflags)
  
    LDFLAGS=$ldflags_td_save
    AC_SUBST(TERADATA_LDFLAGS, $td_ldflags)
  
    LIBS=$libs_td_save
    AC_SUBST(TERADATA_LIBS, $td_lib)
  fi
])
