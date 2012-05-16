#!/bin/sh

set -x

TMPF=/tmp/get-svn-tora.tmp

# Checkout the tora trunk of the latest release
rm -rf tora
svn co https://tora.svn.sourceforge.net/svnroot/tora/trunk/tora tora | tee $TMPF

# Checked out revision
VER=$(tail -1 $TMPF | cut -f4 -d' '| cut -f1 -d'.')
echo $VER > CURRENT_REV

# Move directory to a cache directory
mv tora tora.$VER

cd tora.$VER

# Update the title bar to include the release version
sed "s/\[2\.1\.4\]/[2.1.4.$VER]/" < configure.ac > configure.ac-
mv -f configure.ac- configure.ac

