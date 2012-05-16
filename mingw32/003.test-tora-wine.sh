#!/bin/sh

#set -x
export ROOTDIR=/tmp/tora-zd
export Path='Z:\\tmp\\tora-zd\\instantclient_11_2;Z:\\tmp\\tora-zd\\bin'
#export WINEDLLOVERRIDES="oci=n"
cd $ROOTDIR/bin
#wine cmd
wine tora.exe
