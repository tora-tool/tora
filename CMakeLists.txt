##############################################################################################################
########## CMake Setup                                                                              ##########

#Set our CMake minimum version
#Require 2.4.2 for Qt finding
#Require 2.4.3 for moc change detection and rpath updates for custom cairo install
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.5 FATAL_ERROR)

#Pretty colors
SET(CMAKE_COLOR_MAKEFILE ON)
#Dont force verbose
SET(CMAKE_VERBOSE_MAKEFILE OFF)
#Include current dir
SET(CMAKE_INCLUDE_CURRENT_DIR TRUE)


INCLUDE(CheckIncludeFile)
INCLUDE(CheckCCompilerFlag)
INCLUDE(CheckCXXCompilerFlag)
INCLUDE(CheckTypeSize)
INCLUDE(TestBigEndian)

#Set the custom CMake module directory where our include/lib finders are
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

#Project Setup
PROJECT(tora)
#Set our version values
#Final version is ${VERSION} = ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${VERSION_SUFFIX}
#where VERSION_SUFFIX is of the form "", "cvs" or "Ncvs" (N being the minor patch level)
SET (VERSION_MAJOR "1")
SET (VERSION_MINOR "4")
SET (VERSION_PATCH "0")
SET (VERSION_SUFFIX "svn")
SET (VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${VERSION_SUFFIX})
MESSAGE(STATUS "TOra ${VERSION} will be built to install into ${CMAKE_INSTALL_PREFIX}")

SET (EXE_NAME "tora")

ADD_DEFINITIONS("-DTOVERSION=\\\"${VERSION}\\\"")

#Set the permissions to be used when installing plugins
SET(PLUGIN_PERMISSIONS WORLD_EXECUTE GROUP_EXECUTE OWNER_EXECUTE WORLD_READ GROUP_READ OWNER_READ OWNER_WRITE)
SET (DEFAULT_PLUGIN_DIR "${CMAKE_INSTALL_PREFIX}/lib/tora")
ADD_DEFINITIONS("-DDEFAULT_PLUGIN_DIR=\\\"${DEFAULT_PLUGIN_DIR}\\\"")



##############################################################################################################
########## check for the CPU we build for                                                           ##########

IF (GCC)

EXECUTE_PROCESS(
    COMMAND ${CMAKE_C_COMPILER} -dumpmachine
    OUTPUT_VARIABLE MACHINE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
MESSAGE(STATUS "Building for target ${MACHINE}")

STRING(REGEX MATCH "(i?86-*)|(athlon-*)|(pentium-*)" _machine_x86 "${MACHINE}")
IF (_machine_x86)
    MESSAGE(STATUS "Found target X86")
    SET(ARCH_X86 1)
ENDIF (_machine_x86)

STRING(REGEX MATCH "(x86_64-*)|(X86_64-*)|(AMD64-*)|(amd64-*)" _machine_x86_64 "${MACHINE}")
IF (_machine_x86_64)
    MESSAGE(STATUS "Found target X86_64")
    SET(ARCH_X86_64 1)
ENDIF (_machine_x86_64)

STRING(REGEX MATCH "(ppc-*)" _machine_ppc "${MACHINE}")
IF (_machine_ppc)
    MESSAGE(STATUS "Found target PPC")
    SET(ARCH_PPC 1)
ENDIF (_machine_ppc)

ENDIF (GCC)

#Convert our simpler command line option to the CMake style
#None, Debug, Release, .. or custom ones
# SET(WANT_DEBUG 1)
IF(WANT_DEBUG)
  SET(CMAKE_BUILD_TYPE Debug)
ELSE(WANT_DEBUG)
  SET(CMAKE_BUILD_TYPE RELEASE)
ENDIF(WANT_DEBUG)


# Lets disable Wall warnings due the MSVC too much talkative
# output. Wall can slow down compiletion up to hours and
# my boxes can boil eggs too.
# MSVC, MSVC_IDE, MSVC60, MSVC70, MSVC71, MSVC80, CMAKE_COMPILER_2005
IF (MSVC)
    SET (CXX_WARNINGS "")
ELSE (MSVC)
    SET (CXX_WARNINGS "-Wall")
ENDIF (MSVC)


#Based on our build type, setup our build options
IF(APPLE)
  SET( ENV{MACOSX_DEPLOYMENT_TARGET} 10.3 ) 
  SET( ENV{LD_PREBIND} 1 )
  SET( ENV{LD_PREBIND_ALLOW_OVERLAP} 1 )
  SET(CMAKE_CXX_FLAGS_DEBUG "-O2 -g3 -fno-inline-functions ${CXX_WARNINGS}")
  SET(CMAKE_C_FLAGS_DEBUG "-O2 -g3 -fno-inline-functions ${CXX_WARNINGS}")
ELSE(APPLE)
  SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 ${CXX_WARNINGS} -fstrict-aliasing")
  SET(CMAKE_C_FLAGS_DEBUG "-O0 -g3 ${CXX_WARNINGS} -fstrict-aliasing")
ENDIF(APPLE)


SET(CMAKE_CXX_FLAGS_RELEASE "-O2 ${CXX_WARNINGS}")
SET(CMAKE_C_FLAGS_RELEASE "-O2 ${CXX_WARNINGS}")
# Required for win wjen there is used an external qscintilla lib
IF (WIN32 AND NOT DEFINED WANT_INTERNAL_QSCINTILLA)
  SET(CMAKE_CXX_FLAGS_RELEASE "/DQSCINTILLA_DLL ${CMAKE_CXX_FLAGS_RELEASE}")
  SET(CMAKE_C_FLAGS_RELEASE "/DQSCINTILLA_DLL ${CMAKE_C_FLAGS_RELEASE}")
ENDIF (WIN32 AND NOT DEFINED WANT_INTERNAL_QSCINTILLA)


##############################################################################################################
########## Find Dependencies                                                                        ##########

# wo don't want to plugin build
ADD_DEFINITIONS(-DTOMONOLITHIC)


SET (QT_MT_REQUIRED true)
SET (QT_MIN_VERSION "4.2.0")
SET (QT_USE_QTSQL TRUE)
IF (NOT WANT_NOQT3SUPPORT)
  SET (QT_USE_QT3SUPPORT TRUE)
ENDIF (NOT WANT_NOQT3SUPPORT)

FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE( ${QT_USE_FILE} )
IF (QT_FOUND)
    MESSAGE("Qt4 Found OK")
    IF (WANT_NOQT3SUPPORT)
        MESSAGE(STATUS "Building without Qt3 Support")
    ENDIF (WANT_NOQT3SUPPORT)
    #Add our Qt definitions
    ADD_DEFINITIONS(
        ${QT_DEFINITIONS}
        -DQT_CORE_LIB
        -DQT_GUI_LIB
        -DQT_THREAD_LIB
    )
ELSE(QT_FOUND)
    MESSAGE(FATAL_ERROR "No Qt4 found")
ENDIF(QT_FOUND)


IF (WANT_INTERNAL_QSCINTILLA)
    MESSAGE(STATUS "QSci: Building with internal copy of QScintilla.")
    MESSAGE(STATUS "QSci: It's recomended to use external library for it")
ELSE (WANT_INTERNAL_QSCINTILLA)

	FIND_PACKAGE(QScintilla REQUIRED)
	IF (QSCINTILLA_FOUND)
		MESSAGE("QScintilla2 Found OK")
		ADD_DEFINITIONS("-DQT -DSCI_LEXER")
	ELSE (QSCINTILLA_FOUND)
		MESSAGE(STATUS "QSci: No QScintilla2 libraries found")
		MESSAGE(STATUS "QSci: You can use -DWANT_INTERNAL_QSCINTILLA=1 to build included copy of it.")
		MESSAGE(FATAL_ERROR "No QScintilla2 found")
	ENDIF (QSCINTILLA_FOUND)

ENDIF (WANT_INTERNAL_QSCINTILLA)


FIND_PACKAGE(Oracle)
IF(ORACLE_FOUND)
    MESSAGE("Oracle OCI Library Found OK")
ELSE(ORACLE_FOUND)
    MESSAGE(STATUS "No Oracle OCI found. TOra will be build without Oracle support")
    ADD_DEFINITIONS("-DTO_NO_ORACLE")
ENDIF(ORACLE_FOUND)

##############################################################################################################
########## Include Setup                                                                            ##########

TEST_BIG_ENDIAN(WORDS_BIGENDIAN)

CHECK_INCLUDE_FILE("dlfcn.h" HAVE_DLFCN_H)
IF(HAVE_DLFCN_H)
  ADD_DEFINITIONS(-DHAVE_DLFCN_H)
ENDIF(HAVE_DLFCN_H)

CHECK_INCLUDE_FILE("unistd.h" HAVE_UNISTD_H)
IF(HAVE_UNISTD_H)
  ADD_DEFINITIONS(-DHAVE_UNISTD_H)
ENDIF(HAVE_UNISTD_H)

CHECK_INCLUDE_FILE("sys/types.h" HAVE_SYS_TYPES_H)
# IF(HAVE_SYS_TYPES_H)
#   ADD_DEFINITIONS(-DHAVE_SYS_TYPES_H)
# ENDIF(HAVE_SYS_TYPES_H)

CHECK_INCLUDE_FILE("sys/stat.h" HAVE_SYS_STAT_H)
# IF(HAVE_SYS_STAT_H)
#   ADD_DEFINITIONS(-DHAVE_SYS_STAT_H)
# ENDIF(HAVE_SYS_STAT_H)
#>>Test for existing include files


#Create configure files.. config.h and uninstall
#config.h
INCLUDE (ConfigureChecks.cmake)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
ADD_DEFINITIONS("-DHAVE_CONFIG_H")



##############################################################################################################
########## Uninstall Setup                                                                          ##########

CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY
)

ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
  
  
##############################################################################################################
########## Add our subdirs                                                                          ##########

# tora_toad.h build
# ADD_SUBDIRECTORY(utils/chex)
# MESSAGE(STATUS "Creating src/tora_toad.h")
# MESSAGE("./utils/chex/chex src/i18n/tora_toad.qm > src/tora_toad.h")
# EXECUTE_PROCESS(COMMAND ./utils/chex/chex src/i18n/tora_toad.qm > src/tora_toad.h
#                   RESULT_VARIABLE chex_var
#                   OUTPUT_VARIABLE chex_out)
# MESSAGE(STATUS "Result: ${chex_var}")

#Add our source subdirs
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(doc)

#Install our READMEs etc.
INSTALL(FILES
    AUTHORS
    BUGS
    ChangeLog
    COPYING
    INSTALL
    LICENSE.txt
    NEWS
    README
    README.BINARIES
    README.CODESTYLE
    README.LAYOUT
    README.LICENSE
    README.OSX
    README.RELEASE
    README.SVN
    README.WINDOWS
    TODO
    DESTINATION ${DEFAULT_PLUGIN_DIR}
)
