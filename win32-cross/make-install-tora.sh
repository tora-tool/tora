#!/bin/sh

# set -x

ORIG_TORA_EXE=tora.exe

cd /usr/local/tora-w32/bin
SHA1SUM=$(sha1sum $ORIG_TORA_EXE)
BYTES=$(du --bytes $ORIG_TORA_EXE)
LS=$(ls -l $ORIG_TORA_EXE)
ORIG=$(echo "ORIGINAL. $SHA1SUM $BYTES $LS")

cd /home/zippy/win32-tora/tora
sudo make install

cd /usr/local/tora-w32/bin
SHA1SUM=$(sha1sum $ORIG_TORA_EXE)
BYTES=$(du --bytes $ORIG_TORA_EXE)
LS=$(ls -l $ORIG_TORA_EXE)
NEW=$(echo "NEW...... $SHA1SUM $BYTES $LS")

echo $ORIG
echo $NEW

