#!/bin/sh

LD_LIBRARY_PATH=/usr/local/kde/lib
export LD_LIBRARY_PATH

if [ "X$1" = "X" ]
then
    echo Missing version
    exit 1
fi

rm -rf release
mkdir -p release
cd release
if [ "X$2" = "X" ]
then
    echo Releasing head
    cvs -z3 -d:pserver:anonymous@cvs.tora.sourceforge.net:/cvsroot/tora co tora
    TYPE=-beta
else
    echo Releasing branch $2
    cvs -z3 -d:pserver:anonymous@cvs.tora.sourceforge.net:/cvsroot/tora co -r$2 tora
fi
if [ "X$3" != "X" ]
then
    TYPE=-$3
fi

if rpm -qa | grep 'glibc-2\.2' > /dev/null
then
    GLIBC=glibc22
    echo Detected glibc-2.2
else
    GLIBC=glibc21
    echo Detected glibc-2.1
fi

rm -rf `find tora -name CVS`
touch tora/* tora/*/*
mv tora tora-$1
tar czf ../tora$TYPE-$1.tar.gz tora-$1
tar cf - tora-$1 | bzip2 > /usr/src/RPM/SOURCES/tora-$1.tar.bz2
rpm -bb tora-$1/rpm/tora.spec
cp /usr/src/RPM/RPMS/i686/tora-$1-1gc.i686.rpm tora$TYPE-$1-1gc-$GLIBC.i686.rpm 

SRCPWD=`pwd`
cd /usr/src/RPM/BUILD/tora-$1
make tora-mono
strip tora-mono
mv tora-mono tora
cd ..
echo Packing tora$TYPE-$1-bin-$GLIBC.tar.gz
tar czf $SRCPWD/../tora$TYPE-$1-bin-$GLIBC.tar.gz tora-$1/tora tora-$1/README tora-$1/LICENSE \
    tora-$1/BUGS tora-$1/NEWS tora-$1/INSTALL tora-$1/TODO tora-$1/templates/sqlfunctions.tpl \
    tora-$1/icons/tora.xpm tora-$1/icons/toramini.xpm tora-$1/help

if [ "X$GLIBC" = "Xglibc21" ]
then
    cd $SRCPWD
    QTDIR=/usr/local/qt3
    export QTDIR
    rpm -bb tora-$1/rpm/tora-static.spec
    cd /usr/src/RPM/BUILD/tora-$1
    strip tora-static
    mv tora-static tora
    cd ..
    echo Packing tora$TYPE-$1-static.tar.gz
    tar czf $SRCPWD/../tora$TYPE-$1-static.tar.gz tora-$1/tora tora-$1/README tora-$1/LICENSE \
	tora-$1/BUGS tora-$1/NEWS tora-$1/INSTALL tora-$1/TODO tora-$1/templates/sqlfunctions.tpl \
	tora-$1/icons/tora.xpm tora-$1/icons/toramini.xpm tora-$1/help
    cp /usr/src/RPM/RPMS/i686/tora-$1-1static.i686.rpm tora$TYPE-$1-1static.i686.rpm 
else
    cd $SRCPWD
    export ORACLE_HOME=$ORACLE_HOME/../9.0.1
    rpm -bb tora-$1/rpm/tora.spec
    cp /usr/src/RPM/RPMS/i686/tora-$1-1gc.i686.rpm tora$TYPE-$1-1gc-oracle9.i686.rpm 

    cd /usr/src/RPM/BUILD/tora-$1
    make tora-mono
    strip tora-mono
    mv tora-mono tora
    cd ..
    tar czf $SRCPWD/../tora$TYPE-$1-bin-oracle9.tar.gz tora-$1/tora tora-$1/README tora-$1/LICENSE \
        tora-$1/BUGS tora-$1/NEWS tora-$1/INSTALL tora-$1/TODO tora-$1/templates/sqlfunctions.tpl \
        tora-$1/icons/tora.xpm tora-$1/icons/toramini.xpm tora-$1/help
fi

cd $SRCPWD/..
rm -rf release
#rm -rf /usr/src/RPM/BUILD/*
