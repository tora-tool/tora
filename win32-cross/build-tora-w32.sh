#!/bin/sh

set -x

# tora cache revision
SVN=3317

# relevant directories and file locations
export WIN32_ROOT=/home/zippy/win32-tora
export QTDIR=/usr/i686-pc-mingw32/sys-root/mingw
# QTLIB required to force use library files with a "4" on the end
export QTLIB=$QTDIR/lib
export QTINCLUDE=$QTDIR/include

export QSCINTILLA_DIR=$WIN32_ROOT/QScintilla-gpl-2.3.2

# orginal Oracle Instance client directory
export INSTANT_CLIENT_DIR=$WIN32_ROOT/instantclient_11_1

# these are where the *.a files are (which have been created from the original Oracle DDLs)
export OCILIB=$WIN32_ROOT/instantclient_11_1_lib/lib

# set up the compiler flags
export CFLAGS="-O2 -Wall -mthreads -w -pipe -I$WIN32_ROOT/w32api-3.13-mingw32/include \
               -DOTL_ORA_TIMESTAMP \
               -DOTL_ANSI_CPP \
               -DOTL_EXCEPTION_ENABLE_ERROR_OFFSET \
               -DQT_THREAD_SUPPORT -DTOMONOLITHIC -DWIN32 -D_WIN32"
export CXXFLAGS=$CFLAGS

# Set up the linker flags.  Note, reference the Oracle libraries with absolute path (not with -l)
export LDFLAGS="-s -static -L$WIN32_ROOT/w32api-3.13-mingw32/lib -L$WIN32_ROOT/instantclient_11_1_lib -lwsock32 \
                $OCILIB/liboci.a $OCILIB/libociw32.a $OCILIB/liboraocci11.a -static-libgcc"

# Use a pristine copy of the TOra source from a cached directory (tora.$revision)
rm -rf tora
cp -r tora.$SVN tora

# Kludge - use the Linux chex (WTF! is chex?) (Maybe try wine with w32 chex.exe sometime?)
cp chex tora/utils/chex/
chmod 744 tora/utils/chex/chex
cp tora_toad.h tora/src
mv -f tora/src/otlv4.h tora/src/otl

# Copy Windoze-specific files to the main source directory
# and update the Makefile.am so the get compiled and linked.
cp tora/src/windows/cregistry.* tora/src/
patch -N tora/src/Makefile.am Makefile.am.patch

# Set the ORACLE_HOME and LD_LIBRARY  (must do this when not using --with-instant-client)
export ORACLE_HOME=$OCILIB
export LD_LIBRARY_PATH=$ORACLE_HOME/lib

# Kludge - hack oracle.m4 to remove clntsh token so not included in generated configure script
patch -N tora/config/m4/oracle.m4 oracle.m4.patch

# Build the executable
cd tora
./autogen.sh
./configure --prefix=/usr/local/tora-w32 \
            --exec-prefix=/usr/local/tora-w32 \
            --without-x \
            --host=i686-pc-mingw32 \
            --with-oci-version=11G \
            --with-oracle-includes=$INSTANT_CLIENT_DIR/sdk/include \
            --with-oracle-libraries=$OCILIB \
            --with-qt-dir=$QTLIB \
            --with-qt-includes=$QTINCLUDE \
            --with-qscintilla=$QSCINTILLA_DIR \
            --with-qscintilla-libraries=$QSCINTILLA_DIR/Qt4/release \
            --with-qscintilla-includes=$QSCINTILLA_DIR/include \

if [ $? -ne 0 ]; then
  echo "configure failed [$?]"
fi

time make

