#!/bin/sh

# This file will recursively format all files using astyle so that all files have same code style

for file in `find . | grep -E '\.hpp$|\.h$|\.cpp$'`
do
    astyle --indent=spaces --indent-classes --indent-switches --indent-cases --indent-namespaces --indent-labels --indent-preprocessor --pad-header --style=allman $file
done
