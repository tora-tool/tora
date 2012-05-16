#!/bin/sh

set -x

# tora cache revision
REVISION=$(cat CURRENT_REV)

# relevant directories and file locations
export WIN32_ROOT=/home/tora/Windows/Tora32
export TORA_INSTALL_DIR=/tmp/tora-zd
export TORA_INSTALL_DIR_BIN=$TORA_INSTALL_DIR/bin
export QTDIR=/usr/i686-w64-mingw32/sys-root/mingw
export MINGW32_ROOT=$QTDIR
# QTLIB required to force use library files with a "4" on the end
export QTLIB=$QTDIR/lib
export QTINCLUDE=$QTDIR/include

# configure needs to know where these are
export LRELEASE=/usr/lib64/qt4/bin/lrelease
export LUPDATE=/usr/lib64/qt4/bin/lupdate

## export QSCINTILLA_DIR=$WIN32_ROOT/QScintilla-gpl-2.3.2
export QSCINTILLA_DIR=$WIN32_ROOT/QScintilla-gpl-2.6.1

# orginal Oracle Instance client directory
export INSTANT_CLIENT_DIR=$WIN32_ROOT/instantclient_11_2

# these are where the *.a files are (which have been created from the original Oracle DDLs)
export OCILIB=$WIN32_ROOT/instantclient_11_2_lib/lib

# set up the compiler flags
export CFLAGS="-O2 -Wall -mthreads -w -pipe \
               -DOTL_ORA_TIMESTAMP \
               -DOTL_ANSI_CPP \
               -DOTL_EXCEPTION_ENABLE_ERROR_OFFSET \
               -DQT_THREAD_SUPPORT -DTOMONOLITHIC -DWIN32 -D_WIN32 \
               -DWIN32 -D_WIN32"

export CXXFLAGS=$CFLAGS

# Set up the linker flags.  Note, reference the Oracle libraries with absolute path (not with -l)
## Voodoo here! -static in F17 causes the link to fail!
#F16# export LDFLAGS="-s -static -L$WIN32_ROOT/instantclient_11_2_lib -lwsock32 \
export LDFLAGS="-s -L$WIN32_ROOT/instantclient_11_2_lib -lwsock32 \
                $OCILIB/liboci.a $OCILIB/libociw32.a $OCILIB/liboraocci11.a -static-libgcc -static-libstdc++"

# Use a pristine copy of the TOra source from a cached directory (tora.$revision)
rm -rf tora
cp -r tora.$REVISION tora

# Kludge - use the Linux chex (WTF! is chex?) (Maybe try wine with w32 chex.exe sometime?)
## cp chex tora/utils/chex/
## chmod 744 tora/utils/chex/chex
## cp tora_toad.h tora/src

# Hack to prevent double-inclusion of stdc starting with F14
patch -N tora/src/Makefile.am Makefile.am.stdc.patch

# Set the ORACLE_HOME and LD_LIBRARY  (must do this when not using --with-instant-client)
# export ORACLE_HOME=$OCILIB
export LD_LIBRARY_PATH=$ORACLE_HOME/lib

# Kludge - hack oracle.m4 to remove clntsh token so not included in generated configure script
patch -N tora/config/m4/oracle.m4 oracle.m4.patch

# Build the executable
cd tora
./autogen.sh

./configure --prefix=$TORA_INSTALL_DIR \
            --without-x \
            --host=i686-w64-mingw32 \
            --with-oci-version=11G \
            --with-oracle-includes=$WIN32_ROOT/instantclient_11_2_lib/include \
            --with-oracle-libraries=$WIN32_ROOT/instantclient_11_2_lib/lib \
            --with-qt-dir=$QTLIB \
            --with-qt-includes=$QTINCLUDE \
            --with-qscintilla=$QSCINTILLA_DIR \
            --with-qscintilla-libraries=$QSCINTILLA_DIR/Qt4/release \
            --with-qscintilla-includes=$QSCINTILLA_DIR/include \

if [ $? -ne 0 ]; then
  echo "configure failed [$?]"
  exit
fi

## Ugly hack: first make aborts because of chex. The second make carries on without error. 
make
make
if [ $? -ne 0 ]; then
  echo "make failed [$?]"
  exit
fi

rm -rf $TORA_INSTALL_DIR
make install

## Copy Oracle Instant Client to Tora root directory
cp -r $WIN32_ROOT/instantclient_11_2 $TORA_INSTALL_DIR

## Copy required mingw32 DDLs
cp $MINGW32_ROOT/bin/libgcc_s_sjlj-1.dll $TORA_INSTALL_DIR_BIN
cp $MINGW32_ROOT/bin/QtCore4.dll $TORA_INSTALL_DIR_BIN
cp $MINGW32_ROOT/bin/QtGui4.dll $TORA_INSTALL_DIR_BIN
cp $MINGW32_ROOT/bin/QtNetwork4.dll $TORA_INSTALL_DIR_BIN
cp $MINGW32_ROOT/bin/QtSql4.dll $TORA_INSTALL_DIR_BIN
cp $MINGW32_ROOT/bin/QtXml4.dll $TORA_INSTALL_DIR_BIN
cp $QSCINTILLA_DIR/Qt4/release/qscintilla2.dll $TORA_INSTALL_DIR_BIN
cp $MINGW32_ROOT/bin/libstdc++-6.dll $TORA_INSTALL_DIR_BIN
cp $MINGW32_ROOT/bin/zlib1.dll $TORA_INSTALL_DIR_BIN
cp $MINGW32_ROOT/bin/libpng15-15.dll $TORA_INSTALL_DIR_BIN
## Copy extras mingw32
cp $WIN32_ROOT/tora.lnk $TORA_INSTALL_DIR_BIN
cp $WIN32_ROOT/tora.bat $TORA_INSTALL_DIR_BIN
cp $WIN32_ROOT/tora.ico $TORA_INSTALL_DIR_BIN
cp $WIN32_ROOT/README.txt $TORA_INSTALL_DIR

cd /tmp
rm -f /tmp tora-zd.$REVISION.zip $WIN32_ROOT/tora-zd.$REVISION.zip
zip -b /tmp tora-zd.$REVISION.zip $(find tora-zd -print)
cd $WIN32_ROOT
mv /tmp/tora-zd.$REVISION.zip $WIN32_ROOT

