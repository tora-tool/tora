#! /bin/bash

cd /tmp/qt$QTTYPE/src

echo Testing QTTYPE $QTTYPE

echo "==================================================="
echo "Test 1"
echo "==================================================="
# This thing doesn't work on all builds yet
if [ -f test1 ]
  then
   xvfb-run ./test1
fi
echo "==================================================="
echo "Test 4"
echo "==================================================="
#xvfb-run ./test4 || exit 1
exit 0
