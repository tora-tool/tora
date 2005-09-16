#!/bin/sh

me=$0

err() {
    echo "$me: $1"
    exit 1
}

# these programs don't always print their program names. does some
# pretty output, finds version numbers.
#
# $1 - prog name
# $2 - arguments
# $3 - preferred version, optional, major
# $4 - preferred version, optional, minor
run() {
    prog=$1
    args=$2
    major=$3
    minor=$4

    if test "${major}x" != "x"; then
        if test "${minor}x" = "x"; then
            minor=0
        fi

        while expr $major \< 10 >/dev/null; do
            # find a version number greater than param
            ${prog}-${major}.${minor} --version >/dev/null 2>&1
            if test "$?" -eq "0"; then
                prog=${prog}-${major}.${minor}
                break
            fi

            minor=`expr $minor + 1`
            if expr $minor \> 15 >/dev/null; then
                minor=0
                major=`expr $major + 1`
            fi
        done
    fi

    echo "$me: running $prog$ver $args..."
    $prog $args
    if test "$?" != "0"; then
        err "$prog failed. exit."
    fi
}


libtool() {
    opts="'--automake --copy'"

    if glibtoolize --version >/dev/null 2>&1; then
        eval run glibtoolize $opts
    elif libtoolize --version >/dev/null 2>&1; then
        eval run libtoolize $opts
    else
        err "libtoolize not found"
    fi
}


run aclocal '' 1 6
run autoheader
run autoconf
libtool
run automake -a 1 6
