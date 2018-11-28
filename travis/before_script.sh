#!/bin/bash -x

set -x

g++ --version

tora=/home/travis/build/tora-tool/tora

if [ "$QTTYPE" = "5" ]; then
    mkdir /tmp/qt5
    cd /tmp/qt5
    source /opt/qt510/bin/qt510-env.sh
    cmake $tora \
	  -DQT5_BUILD=true \
	  -DBOOST_ROOT=/usr/include \
	  -DWANT_INTERNAL_QSCINTILLA=true \
	  -DORACLE_PATH_INCLUDES=/usr/include/oracle/12.1/client64/ \
	  -DORACLE_PATH_LIB=/usr/lib/oracle/12.1/client64/lib/ \
	  -DTEST_APP1=true \
	  -DTEST_APP2=true \
	  -DTEST_APP3=true \
	  -DTEST_APP4=true \
	  -DTEST_APP5=true \
	  -DTEST_APP6=true \
	  -DTEST_APP7=true	      
    make || exit 1
fi
