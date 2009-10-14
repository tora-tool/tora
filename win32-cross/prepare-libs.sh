#!/bin/sh

echo "Create the .def files from oci.lib, ociw32.lib and oraocci10.lib"
set -x
rm oci.def ociw32.def oraocci11.def
wine reimp -d oci.lib
mv OCI.def oci.def
wine reimp -d ociw32.lib
mv OCIW32.def ociw32.def
wine reimp -d oraocci11.lib
set +x

echo "Create the gcc .a libraries from the .def files"
set -x
rm lib/liboci.a lib/libociw32.a lib/liboraocci11.a lib/liboci.a
i686-pc-mingw32-dlltool -k --input-def oci.def --dllname oci.dll --output-lib lib/liboci.a
i686-pc-mingw32-dlltool -k --input-def ociw32.def --dllname ociw32.dll --output-lib lib/libociw32.a
i686-pc-mingw32-dlltool -k --input-def oraocci11.def --dllname oraocci11.dll --output-lib lib/liboraocci11.a
set +x

