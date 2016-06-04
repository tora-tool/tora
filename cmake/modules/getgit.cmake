#
# Original Author:
# 2009-2010 Ryan Pavlik <rpavlik@iastate.edu> <abiryan@ryand.net>
# http://academic.cleardefinition.com
# Iowa State University HCI Graduate Program/VRAC
#
# Copyright Iowa State University 2009-2010.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
FIND_PACKAGE(Git)

function(git_call _var)
  if(NOT GIT_FOUND)
    find_package(Git QUIET)
  endif()
  if(NOT GIT_FOUND)
    set(${_var} "GIT-NOTFOUND" PARENT_SCOPE)
    return()
  endif()

  # TODO sanitize
  #if((${ARGN}" MATCHES "&&") OR
  #(ARGN MATCHES "||") OR
  #(ARGN MATCHES "\\;"))
  #message("Please report the following error to the project!")
  #message(FATAL_ERROR "Looks like someone's doing something nefarious with git_describe! Passed arguments ${ARGN}")
  #endif()

  #message(STATUS "Arguments to execute_process: ${ARGN}")

  execute_process(COMMAND
    "${GIT_EXECUTABLE}"
    #describe
    ${ARGN}
    WORKING_DIRECTORY
    "${CMAKE_SOURCE_DIR}"
    RESULT_VARIABLE
    res
    OUTPUT_VARIABLE
    out
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(NOT res EQUAL 0)
    set(out "${out}-${res}-NOTFOUND")
  endif()

  set(${_var} "${out}" PARENT_SCOPE)
endfunction()

git_call(GITVERSION describe --long --tags --dirty --always)
#parse the version information into pieces.
string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" GITVERSION_MAJOR "${GITVERSION}")
string(REGEX REPLACE "^v[0-9]+\\.([0-9a-z]+).*" "\\1" GITVERSION_MINOR "${GITVERSION}")
string(REGEX REPLACE "^v[0-9]+\\.[0-9a-z]+-([0-9]+).*" "\\1" GITVERSION_COUNT "${GITVERSION}")
string(REGEX REPLACE "^v[0-9]+\\.[0-9a-z]+-[0-9]+-([0-9a-z]*).*" "\\1" GITVERSION_SHA1 "${GITVERSION}")
string(REGEX REPLACE "^v[0-9]+\\.[0-9a-z]+-[0-9]+-[0-9a-z]+" "" GITVERSION_DIRTY "${GITVERSION}")
string(TIMESTAMP BUILD_DATE "%Y-%m-%d")
set(GITVERSION_SHORT "${GITVERSION_MAJOR}.${GITVERSION_MINOR}")

git_call(GIT_BRANCH rev-parse --abbrev-ref HEAD)

IF (USE_EXPERIMENTAL)
   set(GIT_BUILD_TYPE "Experimental")
ELSE ()
   set(GIT_BUILD_TYPE "Release")
ENDIF()

message("Git version ${GITVERSION}")
message("Git version major ${GITVERSION_MAJOR}")
message("Git version minor ${GITVERSION_MINOR}")
message("Git version count ${GITVERSION_COUNT}")
message("Git version sha1  ${GITVERSION_SHA1}")
message("Git version short ${GITVERSION_SHORT}")
message("Git version dirty ${GITVERSION_DIRTY}")
message("Git branch ${GIT_BRANCH}")
message("Build tag v${GITVERSION_MAJOR}.${GITVERSION_MINOR}-${GITVERSION_COUNT}-${GIT_BUILD_TYPE}-${GITVERSION_SHA1}${GITVERSION_DIRTY}")
message("Build date ${BUILD_DATE}")

if(GITVERSION_DIRTY)
message(WARNING "Git version dirty: ${GITVERSION}")
endif()

# write a file with the GITREVISION define
file(WRITE  gitrevision.h.txt "#pragma once\n")
file(APPEND gitrevision.h.txt "#define GITVERSION       \"${GITVERSION}\"\n")
file(APPEND gitrevision.h.txt "#define GITVERSION_MAJOR \"${GITVERSION_MAJOR}\"\n")
file(APPEND gitrevision.h.txt "#define GITVERSION_MINOR \"${GITVERSION_MINOR}\"\n")
file(APPEND gitrevision.h.txt "#define GIT_BUILD_TYPE   \"${GIT_BUILD_TYPE}\"\n")
file(APPEND gitrevision.h.txt "#define GITVERSION_COUNT \"${GITVERSION_COUNT}\"\n")
file(APPEND gitrevision.h.txt "#define GITVERSION_SHA1  \"${GITVERSION_SHA1}\"\n")
file(APPEND gitrevision.h.txt "#define GITVERSION_SHORT \"${GITVERSION_SHORT}\"\n")
file(APPEND gitrevision.h.txt "#define GITVERSION_DIRTY \"${GITVERSION_DIRTY}\"\n")
file(APPEND gitrevision.h.txt "#define GIT_BRANCH       \"${GIT_BRANCH}\"\n")
file(APPEND gitrevision.h.txt "#define BUILD_TAG        \"v${GITVERSION_MAJOR}.${GITVERSION_MINOR}-${GITVERSION_COUNT}-${GIT_BUILD_TYPE}-${GITVERSION_SHA1}${GITVERSION_DIRTY}\"\n")
file(APPEND gitrevision.h.txt "#define BUILD_DATE       \"${BUILD_DATE}\"\n")
file(WRITE  gitrevision.txt   "${GITVERSION}\n")

# copy the file to the final header only if the revision changes
# reduces needless rebuilds
if(TARGET_NAME)
  # execute during build (make phase)
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different gitrevision.h.txt gitrevision.h)
else()
  # execute during cmake (configure phase)
  file(WRITE  gitrevision.h "#pragma once\n")
  file(APPEND gitrevision.h "#define GITVERSION       \"${GITVERSION}\"\n")
  file(APPEND gitrevision.h "#define GITVERSION_MAJOR \"${GITVERSION_MAJOR}\"\n")
  file(APPEND gitrevision.h "#define GITVERSION_MINOR \"${GITVERSION_MINOR}\"\n")
  file(APPEND gitrevision.h "#define GIT_BUILD_TYPE   \"${GIT_BUILD_TYPE}\"\n")
  file(APPEND gitrevision.h "#define GITVERSION_COUNT \"${GITVERSION_COUNT}\"\n")
  file(APPEND gitrevision.h "#define GITVERSION_SHA1  \"${GITVERSION_SHA1}\"\n")
  file(APPEND gitrevision.h "#define GITVERSION_SHORT \"${GITVERSION_SHORT}\"\n")
  file(APPEND gitrevision.h "#define GITVERSION_DIRTY \"${GITVERSION_DIRTY}\"\n")
  file(APPEND gitrevision.h "#define GIT_BRANCH       \"${GIT_BRANCH}\"\n")
  file(APPEND gitrevision.h "#define BUILD_TAG        \"v${GITVERSION_MAJOR}.${GITVERSION_MINOR}-${GITVERSION_COUNT}-${GIT_BUILD_TYPE}-${GITVERSION_SHA1}${GITVERSION_DIRTY}\"\n")
  file(APPEND gitrevision.h "#define BUILD_DATE       \"${BUILD_DATE}\"\n")
endif()

#message(STATUS "Called with CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR}")

IF(WIN32)
CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/msi/gitrevision.wxi.cmake
	${CMAKE_SOURCE_DIR}/gitrevision.wxi
	@ONLY
)	
CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/msi/gitrevision.bat.cmake
	${CMAKE_SOURCE_DIR}/gitrevision.bat
	@ONLY	
)
ENDIF()

IF (WANT_RPM)
CONFIGURE_FILE(
	${CMAKE_SOURCE_DIR}/rpm/${RPM_SPEC}.cmake
	${CMAKE_SOURCE_DIR}/${RPM_SPEC}
	@ONLY
)
ENDIF()
