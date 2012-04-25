##
# This code is based on SO discussion
# http://stackoverflow.com/questions/3780667/use-cmake-to-get-build-time-svn-revision
#
# the FindSubversion.cmake module is part of the standard distribution
include(FindSubversion)
# extract working copy information for SOURCE_DIR into MY_XXX variables
Subversion_WC_INFO(${SOURCE_DIR} MY)
# write a file with the SVNREVISION define
file(WRITE svnrevision.h.txt "#define SVNREVISION \"${MY_WC_REVISION}\"\n")
file(WRITE svnrevision.txt "${MY_WC_REVISION}\n")
# copy the file to the final header only if the revision changes
# reduces needless rebuilds
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different svnrevision.h.txt svnrevision.h)

MESSAGE("Root: ${MY_WC_ROOT}")