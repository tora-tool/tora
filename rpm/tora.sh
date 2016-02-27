#!/bin/sh

if [ "X$ORACLE_HOME" = "X" ]
then
    echo "WARNING: Environment variable ORACLE_HOME not set, check your Oracle installation"
else
    LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$ORACLE_HOME/lib
    export LD_LIBRARY_PATH
fi
tora.real
