#!/bin/sh

# $1 - path to original find_requires
exec $1 | grep -v -E 'lib(clntsh|nnz|occi)' 
