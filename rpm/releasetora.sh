#!/bin/sh

LD_LIBRARY_PATH=/usr/local/kde/lib
export LD_LIBRARY_PATH

if [ "X$1" = "X" ]
then
    echo Missing version
    exit 1
fi

SRCPWD=`pwd`

rm -rf release
mkdir -p release
cd release
if [ "X$2" = "X" ]
then
    echo Releasing head
    cvs -d`cat ../tora/CVS/Root` co tora
    TYPE=-beta
else
    echo Releasing branch $2
    cvs -d`cat ../tora/CVS/Root` co tora
fi
if [ "X$3" != "X" ]
then
    TYPE=-$3
fi

GLIBC=glibc22

rm -rf `find tora -name CVS`
touch tora/* tora/*/*
mv tora tora-$1
echo Packing source
tar czf ../tora$TYPE-$1.tar.gz tora-$1
ln -sf `pwd`/../tora$TYPE-$1.tar.gz /usr/src/RPM/SOURCES/tora-$1.tar.gz

QTDIR=/usr/local/qt3
export QTDIR

echo Building Mandrake RPM
rpm -bb tora-$1/rpm/tora.spec
cp /usr/src/RPM/RPMS/*/tora-$1-1mdk.*.rpm $SRCPWD/tora$TYPE-$1-1mdk.i686.rpm 
cp /usr/src/RPM/RPMS/*/tora-oracle-$1-1mdk.*.rpm $SRCPWD/tora-oracle$TYPE-$1-1mdk.i686.rpm 

echo Building generic RPM
rpm -bb tora-$1/rpm/tora-static.spec
cp /usr/src/RPM/RPMS/*/tora-$1-1static.*.rpm $SRCPWD/tora$TYPE-$1-1static.i686.rpm 

cd /usr/src/RPM/BUILD/tora-$1
strip tora-static
mv tora-static tora
cd ..
echo Packing tora$TYPE-$1-static.tar.gz

tar czf $SRCPWD/tora$TYPE-$1-underscore.tar.gz tora-$1/tora tora-$1/README tora-$1/LICENSE.txt \
        tora-$1/BUGS tora-$1/NEWS tora-$1/INSTALL tora-$1/TODO tora-$1/templates/sqlfunctions.tpl \
	tora-$1/icons/tora.xpm tora-$1/icons/toramini.xpm tora-$1/help

cd $SRCPWD
rm -rf release

