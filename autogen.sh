#!/bin/sh

CURRENT_AUTOMAKE_VERSION="1.10"
CURRENT_AUTOCONF_VERSION="2.61"
CURRENT_LIBTOOLIZE_VERSION="1.5.22"

me=$0

err() {
    echo "$me: $1"
    exit 1
}

check_libtoolize_version() {
LIBTOOLIZE=libtoolize
CUR_DIR=`pwd`
TESTDIR="special_dir_$$"
mkdir $TESTDIR && cd $TESTDIR
LIBTOOLIZE_STRING=`$LIBTOOLIZE --version | head -n 1`
case $LIBTOOLIZE_STRING in
  libtoolize*${CURRENT_LIBTOOLIZE_VERSION}* )
    echo "execute: $LIBTOOLIZE_STRING ..."
    break;
    ;;
  libtoolize* ) 
    echo "execute: $LIBTOOLIZE_STRING ..."
    echo "*** TOra suggests ${CURRENT_LIBTOOLIZE_VERSION} !"
    echo "*** http://www.gnu.org/software/libtool/libtool.html"
    ;;
esac
cd $CUR_DIR 
rmdir $TESTDIR
}

check_automake_version() {
AUTOMAKE=automake
CUR_DIR=`pwd`
TESTDIR="special_dir_$$"
mkdir $TESTDIR && cd $TESTDIR
AUTOMAKE_STRING=`$AUTOMAKE --version | head -n 1`
case $AUTOMAKE_STRING in
  automake*${CURRENT_AUTOMAKE_VERSION}* )
    echo "execute: $AUTOMAKE_STRING ..."
    break;
    ;;
  automake*1.6.* | automake*1.7* | automake*1.8* | automake*1.9* ) 
    echo "execute: $AUTOMAKE_STRING ..."
    echo "*** TOra suggests ${CURRENT_AUTOMAKE_VERSION} !"
    echo "*** http://www.gnu.org/software/automake"
    ;;
esac
cd $CUR_DIR 
rmdir $TESTDIR
}

check_autoconf_version() {
AUTOCONF=autoconf
CUR_DIR=`pwd`
TESTDIR="special_dir_$$"
mkdir $TESTDIR && cd $TESTDIR
AUTOCONF_STRING=`$AUTOCONF --version | head -n 1`
case $AUTOCONF_STRING in
  autoconf*${CURRENT_AUTOCONF_VERSION}* )
    echo "execute: $AUTOCONF_STRING ..."
    break;
    ;;
  autoconf*2.* )
    echo "execute: $AUTOCONF_STRING ..."
    echo "*** TOra suggests ${CURRENT_AUTOCONF_VERSION} !"
    echo "*** http://www.gnu.org/software/autoconf "
    ;;
esac
cd $CUR_DIR
rmdir $TESTDIR
}

rm -f config.cache
rm -f config.log
rm -f configure
rm -f aclocal.m4

echo "execute: aclocal ..."
aclocal -I config/m4
if test "$?" != "0"; then
   err "aclocal failed. exit."
fi

echo "execute: autoheader ..."
autoheader --force
if test "$?" != "0"; then
   err "autoheader failed. exit."
fi

check_libtoolize_version
libtoolize --force --copy --automake
if test "$?" != "0"; then
   err "libtoolize failed. exit."
fi

check_automake_version
automake --add-missing --copy --force-missing
if test "$?" != "0"; then
   err "automake failed. exit."
fi

check_autoconf_version
autoconf --force
if test "$?" != "0"; then
   err "autoconf failed. exit."
fi


