#!/bin/sh
autoheader && aclocal-1.6 && libtoolize --automake && autoconf && automake-1.6 -a
