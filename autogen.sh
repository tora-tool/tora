#!/bin/sh

me=$0

err() {
    echo "$me: $1"
    exit 1
}

# these programs don't always print their program names.
run() {
    echo "$me: running $1 ..."
    $*
    if test "$?" != "0"; then
        err "$1 failed. exit."
    fi
}

libtool() {
    opts="--automake --copy"

    if glibtoolize --version >/dev/null 2>&1; then
        run glibtoolize $opts
    elif libtoolize --version >/dev/null 2>&1; then
        run libtoolize $opts
    else
        err "libtoolize not found"
    fi
}

run autoheader
run aclocal-1.6
run autoconf
libtool
run automake-1.6 -a
