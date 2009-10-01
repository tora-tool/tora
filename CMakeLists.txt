##############################################################################################################
########## CMake Setup                                                                              ##########

# options available for user's input
OPTION(WANT_DEBUG "Set the debug build and possible additional outputs" OFF)
OPTION(WANT_INTERNAL_QSCINTILLA "Use internal/bundled QScintilla2 source" OFF)
OPTION(ENABLE_ORACLE "Disable Oracle support at all. Including detection" ON)
OPTION(ENABLE_PGSQL "Disable PostgreSQL support. Including detection" ON)
OPTION(WANT_RPM "Enable additional RPM related stuff. Additional make package_rpm target" ON)
OPTION(WANT_BUNDLE "Enable Mac OS X bundle build" OFF)
OPTION(WANT_BUNDLE_STANDALONE "Do not copy required libs and tools into bundle (WANT_BUNDLE)" ON)

#Set our CMake minimum version
#Require 2.4.2 for Qt finding
#Require 2.4.3 for moc change detection and rpath updates for custom cairo install
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.5 FATAL_ERROR)
# FIXME: dont't forget to change CMP0005 policy to NEW after 2.6.0 minimum required!

# handle new cmake versions correctly
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
  cmake_policy(SET CMP0005 OLD) # FIXME change it to new when there will be cmake 2.6.x in most distros
  cmake_policy(SET CMP0011 NEW)
endif(COMMAND cmake_policy)


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
PROJECT(TOra)
#Set our version values
#Final version is ${VERSION} = ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${VERSION_SUFFIX}
#where VERSION_SUFFIX is of the form "", "cvs" or "Ncvs" (N being the minor patch level)
SET (VERSION_MAJOR "2")
SET (VERSION_MINOR "1")
SET (VERSION_PATCH "1")
SET (VERSION_SUFFIX "")
SET (VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${VERSION_SUFFIX})
MESSAGE(STATUS "TOra ${VERSION} will be built to install into ${CMAKE_INSTALL_PREFIX}")

SET (EXE_NAME "tora")
if (WANT_BUNDLE)
    SET(EXE_NAME "TOra")
endif(WANT_BUNDLE)

ADD_DEFINITIONS("-DTOVERSION=\\\"${VERSION}\\\"")
# Allow using CMAKE-related ifdefs in the code. See e.g. toqsqlconnection.cpp.
ADD_DEFINITIONS("-DCMAKE_BUILD")

#Set the permissions to be used when installing plugins
SET(PLUGIN_PERMISSIONS WORLD_EXECUTE GROUP_EXECUTE OWNER_EXECUTE WORLD_READ GROUP_READ OWNER_READ OWNER_WRITE)

if (NOT TORA_PLUGIN_DIR)
    SET (TORA_PLUGIN_DIR "${CMAKE_INSTALL_PREFIX}/share/tora/")
endif (NOT TORA_PLUGIN_DIR)
ADD_DEFINITIONS("-DDEFAULT_PLUGIN_DIR=\\\"${TORA_PLUGIN_DIR}\\\"")

if (NOT TORA_DOC_DIR)
	IF (UNIX)
	    SET (TORA_DOC_DIR "${CMAKE_INSTALL_PREFIX}/share/doc/tora/") # it would be pain for upgrade configs -${VERSION}/")
	ELSE (UNIX)
		SET (TORA_DOC_DIR "${CMAKE_INSTALL_PREFIX}/doc/")
	ENDIF (UNIX)
endif (NOT TORA_DOC_DIR)

#if (NOT TORA_HELP_DIR)
#    SET (TORA_HELP_DIR "${TORA_PLUGIN_DIR}/help/")
#endif (NOT TORA_HELP_DIR)
#ADD_DEFINITIONS("-DTORA_HELP_DIR=\\\"${TORA_HELP_DIR}\\\"")

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
IF(WANT_DEBUG)
  SET(CMAKE_BUILD_TYPE Debug)
  ADD_DEFINITIONS("-DDEBUG")
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
SET (QT_MIN_VERSION "4.3.0")
SET (QT_USE_QTSQL TRUE)
SET (QT_USE_QTNETWORK TRUE)
SET (QT_USE_QTXML TRUE)
FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE( ${QT_USE_FILE} )
IF (QT_FOUND)
    MESSAGE("Qt4 Found OK")
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


IF (NOT ENABLE_ORACLE)
    MESSAGE(STATUS "Oracle support is disabled by user choice")
    ADD_DEFINITIONS("-DTO_NO_ORACLE")
ELSE (NOT ENABLE_ORACLE)

  FIND_PACKAGE(Oracle)
  IF(ORACLE_FOUND)
      MESSAGE("Oracle OCI Library Found OK")
      # what else is hidden in the ifdef hell?!
      ADD_DEFINITIONS("-DDBLINK")
  ELSE(ORACLE_FOUND)
      MESSAGE(STATUS "No Oracle OCI found. TOra will be build without Oracle support")
      ADD_DEFINITIONS("-DTO_NO_ORACLE")
  ENDIF(ORACLE_FOUND)

ENDIF (NOT ENABLE_ORACLE)


IF (NOT ENABLE_PGSQL)
    MESSAGE(STATUS "PostgreSQL support is disabled by user choice")
ELSE (NOT ENABLE_PGSQL)

  FIND_PACKAGE(PostgreSQL)
  IF (POSTGRESQL_FOUND)
    ADD_DEFINITIONS(-DLIBPQ_DECL_CANCEL)
    ADD_DEFINITIONS(-DHAVE_POSTGRESQL_LIBPQ_FE_H)
    MESSAGE(STATUS "PostgreSQL environment found: ${POSTGRESQL_INCLUDE_DIR} ${POSTGRESQL_LIBRARIES}")
  ELSE (POSTGRESQL_FOUND)
    MESSAGE(" ")
    MESSAGE(WARNING " No PostgreSQL environment has been found.")
    MESSAGE(WARNING " Specify -DPOSTGRESQL_PATH_INCLUDES=path")
    MESSAGE(WARNING " and -DPOSTGRESQL_PATH_LIB manually")
	#MESSAGE (STATUS "PostgreSQL includes: ${POSTGRESQL_INCLUDE_DIR} vs. ${POSTGRESQL_PATH_INCLUDES}")
	#MESSAGE (STATUS "PostgreSQL libs: ${POSTGRESQL_LIBRARIES} vs. ${POSTGRESQL_PATH_LIB}")
    MESSAGE(" ")
  ENDIF (POSTGRESQL_FOUND)

ENDIF (NOT ENABLE_PGSQL)


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
# INCLUDE (ConfigureChecks.cmake)
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

#Add our source subdirs
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(doc)

IF (NOT WANT_BUNDLE)
#Install our READMEs etc.
INSTALL(FILES
    AUTHORS
    BUGS
    ChangeLog
    COPYING
    INSTALL
    NEWS
    README
    README.BINARIES
    README.CMAKE
    README.CODESTYLE
    README.LAYOUT
    README.LICENSE
    README.OSX
    README.RELEASE
    README.SVN
    README.WINDOWS
    TODO
    DESTINATION ${TORA_DOC_DIR}
)
ENDIF (NOT WANT_BUNDLE)


IF (WANT_RPM)
    # packaging related stuff
    #SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Toolkit for Oracle")
    SET(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
    #SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
    #SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
    SET(CPACK_PACKAGE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
    SET(CPACK_SOURCE_GENERATOR "TGZ")
    #SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
    SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
    SET(CPACK_IGNORE_FILES "/CVS/;/\\\\.svn/;\\\\.swp$;\\\\.#;/#;\\\\.tar.gz$;/CMakeFiles/;CMakeCache.txt")
    SET(CPACK_SOURCE_IGNORE_FILES ${CPACK_IGNORE_FILES})
    INCLUDE(CPack)
    
    # Check for SVN
    FIND_PACKAGE(Subversion)
    IF(NOT Subversion_FOUND)
        SET(Subversion_SVN_EXECUTABLE true)
    ENDIF(NOT Subversion_FOUND)
    
    # get svn revision in rpmbuild time (and not in Makefile generation time)
    SET(SVN_REVISION
        "\"$$(" ${Subversion_SVN_EXECUTABLE} info ${CMAKE_SOURCE_DIR} | awk '/Revision:/ {print \$\$2 }' ")\""
        )
    SET(RPM_DEFINES 
        --define "_sourcedir ${CMAKE_SOURCE_DIR}"
        )
    SET(SPEC_IN_SUBST
        "\""
        "s/(%define\\s+_version) .*/\\\\1 ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}/\;"
        "s/(%define\\s+_release) .*/\\\\1 0.${SVN_REVISION}svn/\;"
        "\""
        )
    SET(RPM_SPEC "rpm/tora.spec")
    ADD_CUSTOM_TARGET(package-rpm
            COMMAND sed -r ${SPEC_IN_SUBST} "${RPM_SPEC}.in" >"${RPM_SPEC}"
            COMMAND make package_source
            COMMAND rpmbuild -ba ${RPM_DEFINES} "${RPM_SPEC}"
            )
ENDIF (WANT_RPM)
