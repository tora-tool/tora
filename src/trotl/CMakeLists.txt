##############################################################################################################
########## CMake Setup                                                                              ##########
#
# Mostly copied from Tora
# 
#Set our CMake minimum version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0 FATAL_ERROR)

# handle new cmake versions correctly
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
  cmake_policy(SET CMP0005 NEW)
endif(COMMAND cmake_policy)


#Pretty colors
SET(CMAKE_COLOR_MAKEFILE ON)
#Dont force verbose
SET(CMAKE_VERBOSE_MAKEFILE ON)
##SET(CMAKE_VERBOSE_MAKEFILE OFF)
#Include current dir
SET(CMAKE_INCLUDE_CURRENT_DIR TRUE)


#Set the custom CMake module directory where our include/lib finders are
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

#Project Setup
PROJECT(trotl)
#Set our version values
#Final version is ${VERSION} = ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${VERSION_SUFFIX}
#where VERSION_SUFFIX is of the form "", "cvs" or "Ncvs" (N being the minor patch level)
SET (VERSION_MAJOR "0")
SET (VERSION_MINOR "1")
SET (VERSION_PATCH "0")
SET (VERSION_SUFFIX "")
SET (VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${VERSION_SUFFIX})

SET (LIB_NAME "trotl")

IF (CMAKE_COMPILER_IS_GNUCC)  
    SET(CXX_WARNINGS "-Wall")
    SET(CMAKE_CXX_FLAGS_RELEASE "-O2 ${CXX_WARNINGS} -rdynamic -fvisibility-inlines-hidden")
    SET(CMAKE_C_FLAGS_RELEASE "-O2 ${CXX_WARNINGS} -rdynamic -fvisibility-inlines-hidden")
    SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb3 -fno-inline ${CXX_WARNINGS} -fstrict-aliasing -rdynamic")
    SET(CMAKE_C_FLAGS_DEBUG "-O0 -ggbb3 -fno-inline ${CXX_WARNINGS} -fstrict-aliasing -rdynamic")
ENDIF (CMAKE_COMPILER_IS_GNUCC)  

# Lets disable Wall warnings due the MSVC too much talkative
# output.
# MSVC, MSVC_IDE, MSVC60, MSVC70, MSVC71, MSVC80, CMAKE_COMPILER_2005
IF (MSVC)
    SET (CXX_WARNINGS "/wd4251")
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS )
    SET (CMAKE_CXX_FLAGS_DEBUG ${CXX_WARNINGS})

#	SET (CMAKE_SHARED_LINKER_FLAGS "/INCREMENTAL:NO")
#	SET (CMAKE_SHARED_LINKER_FLAGS_DEBUG "/INCREMENTAL:NO")
#	SET (CMAKE_EXE_LINKER_FLAGS "/INCREMENTAL:NO")
#	SET (CMAKE_EXE_LINKER_FLAGS_DEBUG "/MANIFEST /MANIFESTUAC:\"level='asInvoker' uiAccess='false'\" ")
#	SET (CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG /SUBSYSTEM:CONSOLE ")
#	SET (CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DYNAMICBASE /NXCOMPAT /MACHINE:X86 ")
#	SET (CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /ERRORREPORT:PROMPT ")
#	SET (CMAKE_SHARED_LINKER_FLAGS_DEBUG "/DLL /MANIFEST /MANIFESTFILE:trotl.dll.manifest ")
#	SET (CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /MANIFESTUAC:\"level='asInvoker' uiAccess='false'\" ")
#	SET (CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /DEBUG /PDB:trotl.pdb /DYNAMICBASE:NO ")
#	SET (CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /ERRORREPORT:PROMPT ")
#	SET (CMAKE_MODULE_LINKER_FLAGS "/INCREMENTAL:NO")
#	SET (CMAKE_MODULE_LINKER_FLAGS_DEBUG "/INCREMENTAL:NO")
ENDIF (MSVC)


IF(ORACLE_FOUND)
    MESSAGE("-- Oracle OCI Library Found OK")
ELSE(ORACLE_FOUND)
    MESSAGE(FATAL_ERROR "No Oracle client found")
ENDIF(ORACLE_FOUND)

IF(ORACLE_HAS_XML)
    MESSAGE("-- Oracle XDK Library Found OK")
ELSE(ORACLE_HAS_XML)
    MESSAGE(STATUS "-- No Oracle XDK found: ignoring")
ENDIF(ORACLE_HAS_XML)


# SET (QT_MT_REQUIRED true)
# SET (QT_MIN_VERSION "4.4.0")
# FIND_PACKAGE(Qt4 REQUIRED)
# INCLUDE( ${QT_USE_FILE} )
# IF (QT_FOUND)
#     MESSAGE("-- Qt4 Found OK")
#     #Add our Qt definitions
#     ADD_DEFINITIONS(
#         ${QT_DEFINITIONS}
#         -DQT_CORE_LIB
#         -DQT_THREAD_LIB
#     )
# ELSE(QT_FOUND)
#     MESSAGE(STATUS "-- No Qt4 found")
# ENDIF(QT_FOUND)

ADD_SUBDIRECTORY(src)
