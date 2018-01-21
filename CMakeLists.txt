#Project Setup
PROJECT(TOra)

##############################################################################################################
########## CMake Setup                                                                              ##########

# Set a default build type if none was specified
IF(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  MESSAGE(STATUS "Setting build type to 'Debug' as none was specified.")
  SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  SET_PROPERTY(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
ENDIF()

# options available for user's input
OPTION(WANT_INTERNAL_QSCINTILLA "Use internal/bundled QScintilla2 source" OFF)
OPTION(WANT_INTERNAL_LOKI "Use internal/bundled Loki source" OFF)
OPTION(ENABLE_ORACLE "Enable/Disable Oracle support at all. Including detection" ON)
OPTION(ENABLE_PGSQL "Enable/Disable PostgreSQL support. Including detection" ON)
OPTION(ENABLE_DB2 "Enable/Disable DB2 support. Including detection" OFF)
OPTION(ENABLE_TERADATA "Enable/Disable Teradata support." OFF)
OPTION(QT5_BUILD "Use Qt5" ON)
OPTION(WANT_RPM "Enable additional RPM related stuff. Additional make package_rpm target" OFF)
OPTION(USE_PCH "Use percompiled headers" OFF)
OPTION(USE_EXPERIMENTAL "Use experimental code" OFF)

OPTION(TEST_APP01 "Simple lexer test" ON)
OPTION(TEST_APP02 "toConnection/toQuery test" ON)
OPTION(TEST_APP03 "simple application - execute toWorksheet in a separate application" ON)
OPTION(TEST_APP04 "simple application - execute toHighlightedText in separate application" ON)
OPTION(TEST_APP05 "simple application - single tool(tabbed)" ON)
OPTION(TEST_APP06 "simple application parser(boost/spirit+.dot)" ON)
OPTION(TEST_APP07 "simple application parser" ON)
OPTION(TEST_APP08 "simple application - wget" ON)
OPTION(TEST_APP09 "simple application - diff" ON)
OPTION(TEST_APP10 "toCodeView" ON)
OPTION(TEST_APP11 "simple parrser" ON)
OPTION(TEST_APP12 "simple parrser" ON)
OPTION(TEST_APP13 "parrser/indenter" ON)

#Set our CMake minimum version
#Require 2.4.2 for Qt finding
#Require 2.4.3 for moc change detection and rpath updates for custom cairo install
CMAKE_MINIMUM_REQUIRED(VERSION 3.1 FATAL_ERROR) # qt5 requirement
#CMAKE_POLICY(SET CMP0046 OLD) # The dependency target "revisiontag" of target "tora" does not exist
#CMAKE_POLICY(SET CMP0042 OLD) # MACOSX_RPATH

#Pretty colors
SET(CMAKE_COLOR_MAKEFILE ON)
#Dont force verbose
SET(CMAKE_VERBOSE_MAKEFILE OFF)
#Include current dir
SET(CMAKE_INCLUDE_CURRENT_DIR TRUE)

#Set the custom CMake module directory where our include/lib finders are
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

SET(RPM_SPEC "tora.generated.spec")

FIND_PACKAGE(Git)
IF(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
  # a custom target that is always built
  ADD_CUSTOM_TARGET(revisiontag ALL)
  # creates gitrevision.h using cmake script
  ADD_CUSTOM_COMMAND(TARGET revisiontag COMMAND ${CMAKE_COMMAND}
  -DUSE_EXPERIMENTAL=${USE_EXPERIMENTAL}
  -DTARGET_NAME=revisiontag
  -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/getgit.cmake
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  )
  add_definitions("-DHAVE_GITREVISION_H=1")
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/getgit.cmake)
  set (VERSION ${GITVERSION_SHORT})
  set (HAVE_GIT_VERSION 1)
ELSE()
  #Set our version values (if not determined from git tag)
  SET (VERSION_MAJOR "3")
  SET (VERSION_MINOR "2")
  #SET (VERSION_PATCH "0")
  #SET (VERSION_SUFFIX "")
  #SET (VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${VERSION_SUFFIX})
  SET (VERSION ${VERSION_MAJOR}.${VERSION_MINOR})
  if(EXISTS {CMAKE_CURRENT_SOURCE_DIR}/gitrevision.h)
    add_definitions("-DHAVE_GITREVISION_H=1")
  else()
    file(WRITE  ${CMAKE_BINARY_DIR}/toversion.h "#pragma once\n")
    file(APPEND ${CMAKE_BINARY_DIR}/toversion.h "#define TOVERSION \"${VERSION}\"\n")
    file(APPEND ${CMAKE_BINARY_DIR}/toversion.h "#define TOBUILDTYPE_RELEASE \"Release\"\n")
  endif()
ENDIF()

# Boost headers are required to compile with -DDEBUG (see tologger.h)
# also for trotl sql parser(boost spirit)
# also for liber dot parser(boost spirit)
# SET(Boost_DEBUG TRUE)
IF(WIN32)
  FIND_PACKAGE(Boost 1.48.0 REQUIRED)
ELSEIF(APPLE)
  FIND_PACKAGE(Boost 1.48.0 REQUIRED)
ELSE()
  FIND_PACKAGE(Boost 1.48.0 REQUIRED system)
ENDIF()

IF (Boost_FOUND)
    MESSAGE(STATUS "Found Boost libraries for debug build: ${Boost_INCLUDE_DIRS}; libs: ${Boost_LIBRARY_DIRS}")
ELSE (Boost_FOUND)
    MESSAGE(FATAL_ERROR "Boost libs not found.")
ENDIF (Boost_FOUND)

IF (APPLE)
  SET(EXE_NAME "TOra")
ELSE()
  SET(EXE_NAME "tora")  
ENDIF(APPLE)

# For Windows only. Plugings depend on the main program binary. Therefore we must also create .exp .lib (for each .exe)
# The plugin's .dll must be linked against one of them (usually against tora.lib). 
# Change this setting if you're compiling some test program(otherwise the test will SEGFAULT)
IF(WIN32)
  SET(TORA_LIB ${EXE_NAME}) 
  #SET(TORA_LIB test5)
ENDIF()

# Ok, it has to be called after project is set. The we are sure we have size of void pointer set correctly...
INCLUDE(LibSuffix)

#Set the permissions to be used when installing plugins
SET(PLUGIN_PERMISSIONS WORLD_EXECUTE GROUP_EXECUTE OWNER_EXECUTE WORLD_READ GROUP_READ OWNER_READ OWNER_WRITE)


##############################################################################################################
########## check for the CPU we build for                                                           ##########
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
IF (CMAKE_COMPILER_IS_GNUCC)
  SET(CMAKE_CXX_FLAGS_DEBUG   "-Wall -ggdb3 -O0 -fno-inline -rdynamic")
  SET(CMAKE_C_FLAGS_DEBUG     "-Wall -ggdb3 -O0 -fno-inline -rdynamic")
#   SET(CMAKE_CXX_FLAGS_RELEASE "-Wall -std=c++11 -ggdb3 -O2")
#   SET(CMAKE_C_FLAGS_RELEASE   "-Wall            -ggdb3 -O2")
#   SET(CMAKE_CXX_FLAGS         "-Wall -std=c++11 -ggdb3")
#   SET(CMAKE_C_FLAGS           "-Wall            -ggdb3")
ENDIF (CMAKE_COMPILER_IS_GNUCC)

## matches Linux Clang and also AppleClang
IF (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  SET(CMAKE_CXX_FLAGS_DEBUG   "-Wall -ggdb3 -O0 -fno-inline")
  SET(CMAKE_C_FLAGS_DEBUG     "-Wall -ggdb3 -O0 -fno-inline")
#   SET(CMAKE_CXX_FLAGS_RELEASE "-Wall -std=c++11 -ggdb3 -O2")
#   SET(CMAKE_C_FLAGS_RELEASE   "-Wall            -ggdb3 -O2")
#   SET(CMAKE_CXX_FLAGS         "-Wall -std=c++11 -ggdb3")
#   SET(CMAKE_C_FLAGS           "-Wall            -ggdb3")
ENDIF()

#Convert our simpler command line option to the CMake style
#None, Debug, Release, .. or custom ones
IF (CMAKE_BUILD_TYPE MATCHES [Dd]ebug)
  message(STATUS "Debug build")
  set(CMAKE_BUILD_TYPE Debug)
  add_definitions(-DDEBUG -DQT_DEBUG)
  set(TORA_DEBUG TRUE)
ELSE()
  message(STATUS "non-debug build")
  set(TORA_DEBUG FALSE)
  add_definitions(-DNDEBUG -DQT_NO_DEBUG)
  add_definitions(-DQT_NO_DEBUG_OUTPUT)
ENDIF()


# Lets disable Wall warnings due the MSVC too much talkative
# output. Wall can slow down compiletion up to hours and
# my boxes can boil eggs too.
# MSVC, MSVC_IDE, MSVC60, MSVC70, MSVC71, MSVC80, CMAKE_COMPILER_2005
IF (MSVC)
  SET (CXX_WARNINGS "")
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
ELSE (MSVC)
  SET (CXX_WARNINGS "-Wall")
ENDIF (MSVC)


IF (WANT_INTERNAL_LOKI)
  SET(TORA_LOKI_LIB "tora_loki_lib")
  INCLUDE_DIRECTORIES(BEFORE "${CMAKE_SOURCE_DIR}/extlibs/loki/include")
  MESSAGE(STATUS "Using internal loki: ${CMAKE_SOURCE_DIR}/extlibs/loki/include")
ELSE (WANT_INTERNAL_LOKI)
  FIND_PACKAGE(Loki REQUIRED)
  INCLUDE_DIRECTORIES(BEFORE "${LOKI_INCLUDE_DIRS}")
  MESSAGE(STATUS "Using external loki: headers=${LOKI_INCLUDE_DIRS} libs=${LOKI_LIBRARIES}")
ENDIF (WANT_INTERNAL_LOKI)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/extlibs/loki-extra/include)

# Required for win when there is used an external qscintilla lib
IF (WIN32 AND NOT DEFINED WANT_INTERNAL_QSCINTILLA)
  SET(CMAKE_CXX_FLAGS_RELEASE "/DQSCINTILLA_DLL ${CMAKE_CXX_FLAGS_RELEASE}")
  SET(CMAKE_C_FLAGS_RELEASE "/DQSCINTILLA_DLL ${CMAKE_C_FLAGS_RELEASE}")
ENDIF (WIN32 AND NOT DEFINED WANT_INTERNAL_QSCINTILLA)

##############################################################################################################
########## Find Dependencies                                                                        ##########

SET(QT_MT_REQUIRED true)
SET(QT_USE_QTNETWORK TRUE)
SET(QT_USE_QTXML TRUE)
SET(QT_USE_QTWEBKIT TRUE)
#SET(CMAKE_AUTOMOC ON)
IF(QT5_BUILD)

  set (QT_FOUND true)

  set( QT5_ROOT_PATH CACHE PATH "Qt5 root directory (i.e. where the 'bin' folder lies)" )
  if ( QT5_ROOT_PATH )
	list( APPEND CMAKE_PREFIX_PATH ${QT5_ROOT_PATH} )
  endif()

  find_package(Qt5Core REQUIRED)
  find_package(Qt5Gui REQUIRED)
  find_package(Qt5Network REQUIRED)
  find_package(Qt5PrintSupport REQUIRED)
  find_package(Qt5Sql REQUIRED)
  find_package(Qt5Widgets REQUIRED)
  find_package(Qt5Xml REQUIRED)
  find_package(Qt5LinguistTools)
  IF(APPLE)
    find_package(Qt5MacExtras REQUIRED)
  ENDIF()
  set(QT_INCLUDES
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
    ${Qt5PrintSupport_INCLUDE_DIRS}
    ${Qt5Sql_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Xml_INCLUDE_DIRS}
  )
  IF(APPLE)    
    LIST(APPEND QT_INCLUDES ${Qt5MacExtras_INCLUDE_DIRS})
  ENDIF()
  include_directories(${QT_INCLUDES})
  # SET(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} ${Qt5Core_EXECUTABLE_COMPILE_FLAGS}")
  # SET(CMAKE_C_FLAGS           "${CMAKE_C_FLAGS}   ${Qt5Core_EXECUTABLE_COMPILE_FLAGS}")
  # STRING(TOUPPER "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" _flags_var_name)
  # SET(${_flags_var_name}      "${${_flags_var_name}} ${Qt5Core_EXECUTABLE_COMPILE_FLAGS}")
  # SET("CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" _flags_var_name)
  # # prevent error "You must build your code with position independent code if Qt was built with..
  if (Qt5_POSITION_INDEPENDENT_CODE)
    SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
  endif()
ELSE()
  SET (QT_MIN_VERSION "4.8.0")
  SET (QT_USE_QTSQL TRUE)
  SET (QT_USE_QTNETWORK TRUE)
  SET (QT_USE_QTXML TRUE)
  FIND_PACKAGE(Qt4 REQUIRED)
  INCLUDE( ${QT_USE_FILE} )
ENDIF()
IF (QT_FOUND)
  MESSAGE("Qt Found OK")
  #Add our Qt definitions
  ADD_DEFINITIONS(
    ${QT_DEFINITIONS}
    -DQT_CORE_LIB
    -DQT_GUI_LIB
    -DQT_THREAD_LIB
  )
ELSE(QT_FOUND)
  MESSAGE(FATAL_ERROR "No Qt found")
ENDIF(QT_FOUND)

IF (WANT_INTERNAL_QSCINTILLA)
  MESSAGE(STATUS "QSci: Building with internal copy of QScintilla.")
  MESSAGE(STATUS "QSci: It's recomended to use external library for it")
  INCLUDE_DIRECTORIES(BEFORE
    ${CMAKE_SOURCE_DIR}/extlibs/qscintilla2/include
    ${CMAKE_SOURCE_DIR}/extlibs/qscintilla2/src
    ${CMAKE_SOURCE_DIR}/extlibs/qscintilla2/Qt4Qt5
    ${CMAKE_SOURCE_DIR}/extlibs/qscintilla2/lexlib
  )
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
  FIND_PACKAGE(Oracle COMPONENTS CLNTSH XML)
  IF(ORACLE_FOUND)
    MESSAGE("Oracle OCI Library Found OK")
  ELSE(ORACLE_FOUND)
    MESSAGE(STATUS "No Oracle OCI found. TOra will be build without Oracle support")
    ADD_DEFINITIONS("-DTO_NO_ORACLE")
  ENDIF(ORACLE_FOUND)
ENDIF (NOT ENABLE_ORACLE)

IF (NOT ENABLE_PGSQL)
  MESSAGE(STATUS "PostgreSQL advanced support is disabled by user choice")
ELSE (NOT ENABLE_PGSQL)
  FIND_PACKAGE(PostgreSQL)
  IF (POSTGRESQL_FOUND)
    ADD_DEFINITIONS(-DLIBPQ_DECL_CANCEL)
    ADD_DEFINITIONS(-DHAVE_POSTGRESQL_LIBPQ_FE_H)
    MESSAGE(STATUS "PostgreSQL environment found: ${POSTGRESQL_INCLUDE_DIR} ${POSTGRESQL_LIBRARIES}")
  ELSE (POSTGRESQL_FOUND)
    MESSAGE(WARNING " No PostgreSQL environment has been found.")
    MESSAGE(WARNING " Specify -DPOSTGRESQL_PATH_INCLUDES=path")
    MESSAGE(WARNING "     and -DPOSTGRESQL_PATH_LIB=path manually")
    #MESSAGE (STATUS "PostgreSQL includes: ${POSTGRESQL_INCLUDE_DIR} vs. ${POSTGRESQL_PATH_INCLUDES}")
    #MESSAGE (STATUS "PostgreSQL libs: ${POSTGRESQL_LIBRARIES} vs. ${POSTGRESQL_PATH_LIB}")
  ENDIF (POSTGRESQL_FOUND)
ENDIF (NOT ENABLE_PGSQL)

IF (NOT ENABLE_DB2)
  MESSAGE(STATUS "DB2 support is disabled by user choice")
ELSE (NOT ENABLE_DB2)
  FIND_PACKAGE(DB2)
  IF (DB2_FOUND)
    ADD_DEFINITIONS(-DHAVE_DB2)
  ELSE (DB2_FOUND)
    MESSAGE(" ")
    MESSAGE(STATUS " No DB2 client libraries have been found.")
    MESSAGE(STATUS " TOra can be build without it - without DB2 support.")
    MESSAGE(STATUS " Specify -DDB2_PATH_INCLUDES=path")
    MESSAGE(STATUS "     and -DDB2_PATH_LIB=path manually")
    MESSAGE(" ")
  ENDIF (DB2_FOUND)
ENDIF (NOT ENABLE_DB2)

IF (NOT ENABLE_TERADATA)
  MESSAGE(STATUS "Teradata support disabled (enable with -DENABLE_TERADATA=on)")
ELSE (NOT ENABLE_TERADATA)
  FIND_PACKAGE(Teradata)
ENDIF (NOT ENABLE_TERADATA)

IF (USE_EXPERIMENTAL)
  FIND_PACKAGE(PopplerQt4)
  #FIND_PACKAGE(GraphViz)
ELSE ()
  ADD_DEFINITIONS("-DTOBUILDTYPE_RELEASE=1")
ENDIF(USE_EXPERIMENTAL)

#MESSAGE(STATUS "graphviz_FOUND: ${graphviz_FOUND}")

################################################################################
##########                    Uninstall Setup                         ##########
################################################################################
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY
  )

ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

SET(TROTL_DLL_DEFINES    -DLOKI_STATIC -DEXPLICIT_EXPORT -DTROTL_MAKE_DLL)
SET(TROTL_CLIENT_DEFINES -DLOKI_STATIC -DEXPLICIT_EXPORT -DTROTL_DLL)
SET(PARSING_DLL_DEFINES    -DLOKI_STATIC -DEXPLICIT_EXPORT -DTSQLPARSER_MAKE_DLL)
SET(PARSING_CLIENT_DEFINES -DLOKI_STATIC -DEXPLICIT_EXPORT -DTSQLPARSER_DLL)

#Add our source subdirs
ADD_SUBDIRECTORY(extlibs)
ADD_SUBDIRECTORY(src)


find_package(Doxygen)
IF(DOXYGEN_FOUND)
  message(STATUS "")
  message(STATUS "Developer documentation (doxygen) can be generated by 'make docs'")
  message(STATUS "")

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/doc/devel/Doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY
  )

  add_custom_target(docs
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
  )
ENDIF(DOXYGEN_FOUND)

# packaging related stuff
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Toolkit for Oracle")
SET(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
string(TOLOWER ${CMAKE_PROJECT_NAME} CPACK_PACKAGE_NAME)
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Petr Bena")
SET(CPACK_PACKAGE_VERSION "${VERSION}")
#SET(CPACK_SOURCE_GENERATOR "TGZ;TBZ2")
SET(CPACK_SOURCE_GENERATOR "TGZ")
#SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
SET(CPACK_IGNORE_FILES 
    "\\\\.git"
    "\\\\.swp$"
    "\\\\.swp$"
    "\\\\.#"
    "/#"
    "\\\\.tar.gz$"
    "/CMakeFiles/"
    "CMakeCache.txt"
    "refresh-copyright-and-license.pl")
SET(CPACK_SOURCE_IGNORE_FILES ${CPACK_IGNORE_FILES})
INCLUDE(CPack)
# simulate autotools' "make dist"
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)


IF (WANT_RPM)
  SET(RPM_DEFINES --define "_sourcedir ${CMAKE_SOURCE_DIR}" )

  # get git commit count (see rpm rpm/tora.generated.spec.cmake _revision @REVISION@
  # this is evaluated only once (see getgit.cmake)
  IF(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    execute_process(COMMAND ${GIT_EXECUTABLE} describe --long --tags --dirty --always OUTPUT_VARIABLE GITVERSION)
    string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" GITVERSION_MAJOR "${GITVERSION}")
    string(REGEX REPLACE "^v[0-9]+\\.([0-9a-z]+).*" "\\1" GITVERSION_MINOR "${GITVERSION}")
    string(REGEX REPLACE "^v[0-9]+\\.[0-9a-z]+-([0-9]+).*" "\\1" GITVERSION_COUNT "${GITVERSION}")
    string(REGEX REPLACE "^v[0-9]+\\.[0-9a-z]+-[0-9]+-([0-9a-z]*).*" "\\1" GITVERSION_SHA1 "${GITVERSION}")
    string(REGEX REPLACE "^v[0-9]+\\.[0-9a-z]+-[0-9]+-[0-9a-z]+" "" GITVERSION_DIRTY "${GITVERSION}")
    string(TIMESTAMP BUILD_DATE "%Y-%m-%d")
    set(GITVERSION_SHORT "${GITVERSION_MAJOR}.${GITVERSION_MINOR}")    
    
    CONFIGURE_FILE(
        ${CMAKE_SOURCE_DIR}/rpm/${RPM_SPEC}.cmake
        ${CMAKE_SOURCE_DIR}/${RPM_SPEC}
        @ONLY
    )
  ENDIF()

  ADD_CUSTOM_TARGET(package-rpm
    COMMAND make package_source
    COMMAND rpmbuild -ba ${RPM_DEFINES} "${RPM_SPEC}"
    )
ENDIF (WANT_RPM)

IF (WIN32)
  CONFIGURE_FILE(
      ${CMAKE_SOURCE_DIR}/msi/variables.wxi.cmake
      ${CMAKE_SOURCE_DIR}/msi/variables.wxi
      @ONLY
  )
  # see make_msi64_qt5.bat
  IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT OR CMAKE_INSTALL_PREFIX MATCHES "^C:/Program Files")
    SET(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/TARGET/" CACHE PATH "TOra install prefix" FORCE )
  ENDIF()
  if(QT5_BUILD)
    get_target_property(_moc_location Qt5::moc IMPORTED_LOCATION)
    get_filename_component(QTBINDIR ${_moc_location} DIRECTORY)
    message(STATUS "QTBINDIR \"${QTBINDIR}\"")
    set(WINDEPLOYQT_PROG "${QTBINDIR}/windeployqt.exe")
    #find_program(WINDEPLOYQT_PROG windeployqt.exe PATHS ${QTBINDIR} $ENV{QTDIR}/bin/)
    #if(WINDEPLOYQT_PROG)
    message(STATUS "Found ${WINDEPLOYQT_PROG}")
    #else()
    #message(WARNING "Unable to find windeployqt")
    #endif()
  endif()
ENDIF()

MESSAGE(STATUS "TOra ${VERSION} will be built & installed into ${CMAKE_INSTALL_PREFIX}")
