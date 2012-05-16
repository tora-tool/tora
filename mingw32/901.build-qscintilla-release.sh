#!/bin/sh

# see http://trac.transmissionbt.com/wiki/BuildingTransmissionQtWindows

set -x

# relevant directories and file locations
export WIN32_ROOT=/home/tora/Windows/Tora32

# Update this as the version of Qscintilla changes
export QSCINTILLA_DIR=$WIN32_ROOT/QScintilla-gpl-2.6.1

cd $QSCINTILLA_DIR/Qt4

#export QMAKESPEC=/usr/x86_64-w64-mingw32/sys-root/mingw/share/qt4/mkspecs/win32-g++-cross-x64
export QMAKESPEC=win32-g++-cross
export PKG_CONFIG_LIBDIR=/usr/i686-w64-mingw32/sys-root/mingw/lib/pkgconfig

# create the Makefile the QT way
#qmake-qt4 QT_LIBINFIX=4 qscintilla.pro
i686-w64-mingw32-qmake-qt4 QT_LIBINFIX=4 qscintilla.pro

perl -pi -e 's/-fno-rtti//' Makefile.Debug Makefile.Release

# shift a header file to lowercase
perl -pi -e 's/BaseTsd.h/basetsd.h/' ../include/Scintilla.h 

mingw32-make clean
mingw32-make

