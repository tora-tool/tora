# TOra: Configure Oracle libraries
#
# ORACLE_FOUND - system has Oracle OCI
# ORACLE_HAS_XML - Oracle has XDK support(thick client installed)
# ORACLE_INCLUDES - where to find oci.h
# ORACLE_LIBRARIES - the libraries to link against to use Oracle OCI
#
# Components: CLNTSH, CLNTST, OCCI, XML, OCIEI
# CLNTSH is enabled by default
# for example: FIND_PACKAGE(Oracle REQUIRED CLNTSH OCCI)
# 
# copyright (c) 2007 Petr Vanek <petr@scribus.info>
# copyright (c) 2015 Ivan Brezina <ibre5041@ibrezina.net>
# Redistribution and use is allowed according to the terms of the BSD license.
#
# This module will set the following variables in your project:
#
# ``ORACLE_FOUND``
#   Oracle installation detected (InstantClient or "think" client)
# ``ORACLE_INCLUDES``
#   Oracle include directory (where oci.h resides)
# ``ORACLE_LIBRARY_CLNTSH``
#   Library libclntsh.so.*
# ``ORACLE_LIBRARY_CLNTST``
#   Library libclntst??.a (not present in InstantClient, must be geenrated by executing genclntst)
# ``ORACLE_LIBRARY_XML``
#   Oracle XMLTYPE library libxml?.a (usually static, only present in thick client)
# ``ORACLE_LIBRARIES``
#   All Oracle libraries detected libclntsh.so.*, libociei.so.8 libxml.a ....
# ``ORACLE_OCI_VERSION``
#   This is set to 8i, 9i, 10g, 11g, 12c

SET(ORACLE_FOUND "NO")
SET(ORACLE_HAS_XML "NO")
SET(ORACLE_HOME $ENV{ORACLE_HOME})

# "components" handling. It enables to find various independent libraries,
IF( Oracle_FIND_COMPONENTS )
  FOREACH( component ${Oracle_FIND_COMPONENTS} )
    STRING( TOUPPER ${component} _COMPONENT )
    SET( ORACLE_USE_${_COMPONENT} 1 )
  ENDFOREACH( component )

  # legacy behaviour
  IF (NOT ORACLE_USE_CLNTSH)
    SET(ORACLE_USE_CLNTSH 1)
  ENDIF (NOT ORACLE_USE_CLNTSH)

ENDIF( Oracle_FIND_COMPONENTS )


IF (ORACLE_PATH_INCLUDES)
  SET (ORACLE_INCLUDES_LOCATION ${ORACLE_PATH_INCLUDES})
ELSE (ORACLE_PATH_INCLUDES)
    SET (ORACLE_INCLUDES_LOCATION
            ${ORACLE_HOME}/rdbms/public
            ${ORACLE_HOME}/include
            # xdk
            ${ORACLE_HOME}/xdk/include
            # sdk
            ${ORACLE_HOME}/sdk/include
            # xe on windows
            ${ORACLE_HOME}/OCI/include
            # instant client from rpm
            /usr/include/oracle/11.2/client${LIB_SUFFIX}/
            # instantclient on OS X from macports
            /opt/local/lib/oracle/sdk/include/
       )
ENDIF (ORACLE_PATH_INCLUDES)

IF (ORACLE_PATH_LIB)
    SET (ORACLE_LIB_LOCATION ${ORACLE_PATH_LIB})
ELSE (ORACLE_PATH_LIB)
    SET (ORACLE_LIB_LOCATION
            # TODO add 32/64bit version here (lib32/lib64 on some platforms)
            ${ORACLE_HOME}/lib 
            # xe on windows
            ${ORACLE_HOME}/OCI/lib/MSVC
            ${ORACLE_HOME}
            # InstantClient on WinXP
            ${ORACLE_HOME}/sdk/lib/msvc/
            # Instant client from rpm
            /usr/lib${LIB_SUFFIX}/
            # instant client on OS X from macports
            /opt/local/lib/oracle/
        )
ENDIF (ORACLE_PATH_LIB)

FIND_PATH(
    ORACLE_INCLUDES
    oci.h
    ${ORACLE_INCLUDES_LOCATION}
    NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH
)

SET(FORCE_ERROR 0)


# guess OCI version
#   we need it before lib checking because of OS X bugs in instant client (see below)
IF (NOT DEFINED ORACLE_OCI_VERSION AND ORACLE_INCLUDES)
  TRY_RUN(OCIVER_TYPE OCIVER_COMPILED
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/cmake/modules/OCIVersion.c
    CMAKE_FLAGS "-DINCLUDE_DIRECTORIES:STRING=${ORACLE_INCLUDES}"
    OUTPUT_VARIABLE OCIVER)

  IF (${OCIVER_TYPE} GREATER "120")
    SET(ORACLE_OCI_VERSION "12C")
  ELSEIF (${OCIVER_TYPE} GREATER "110")
    SET(ORACLE_OCI_VERSION "11G")
  ELSEIF (${OCIVER_TYPE} EQUAL "102")
    SET(ORACLE_OCI_VERSION "10G_R2")
  ELSEIF (${OCIVER_TYPE} EQUAL "101")
    SET(ORACLE_OCI_VERSION "10G")
  ELSEIF (${OCIVER_TYPE} EQUAL "91")
    SET(ORACLE_OCI_VERSION "9")
  ELSEIF (${OCIVER_TYPE} EQUAL "92")
    SET(ORACLE_OCI_VERSION "9")
  ELSEIF (NOT ${OCIVER_TYPE} GREATER "90")
    SET(ORACLE_OCI_VERSION "8I")
  ELSE (${OCIVER_TYPE} GREATER "110")
    SET(ORACLE_OCI_VERSION "10G_R2")
  ENDIF ()

  MESSAGE(STATUS "Guessed ORACLE_OCI_VERSION value: ${ORACLE_OCI_VERSION} for ${OCIVER_TYPE}")

  IF (APPLE)
    IF (${OCIVER_TYPE} LESS 110)
        SET(ORACLE_USE_OCIEI 1) 
        message(STATUS "    APPLE requires libociei for instantclient < 11, enforcing ociei")
    ENDIF ()
  ENDIF (APPLE)

ENDIF (NOT DEFINED ORACLE_OCI_VERSION AND ORACLE_INCLUDES)


IF (ORACLE_USE_CLNTSH)
    # The NO_DEFAULT_PATH is necessary here in the case there is Oracle DB
    # *and* instant client installed. The order is given in ORACLE_LIB_LOCATION.
    # See above.
    FIND_LIBRARY(
        ORACLE_LIBRARY_CLNTSH
        NAMES libclntsh clntsh oci
        PATHS ${ORACLE_LIB_LOCATION}
        NO_DEFAULT_PATH
	#NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH
    )
    IF (NOT ORACLE_LIBRARY_CLNTSH)
        IF (Oracle_FIND_REQUIRED)
            SET(FORCE_ERROR "CLNTSH")
        ENDIF (Oracle_FIND_REQUIRED)
        set(ORACLE_LIBRARY_CLNTSH "")
    ENDIF (NOT ORACLE_LIBRARY_CLNTSH)
ENDIF (ORACLE_USE_CLNTSH)

IF (ORACLE_USE_CLNTST)
    # The NO_DEFAULT_PATH is necessary here in the case there is Oracle DB
    # *and* instant client installed. The order is given in ORACLE_LIB_LOCATION.
    # See above.
    # Support preference of static libs by adjusting CMAKE_FIND_LIBRARY_SUFFIXES
    IF(WIN32)
	set(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
    ELSE()
	set(CMAKE_FIND_LIBRARY_SUFFIXES .a )
    ENDIF()

    MESSAGE(STATUS "Looking for libclntst")

    FIND_LIBRARY(
        ORACLE_LIBRARY_CLNTST
        NAMES libclntst libclntst9 libclntst10 libclntst11 libclntst12 clntst clntst9 clntst10 clntst11 clntst12
        PATHS ${ORACLE_LIB_LOCATION}
        NO_DEFAULT_PATH
	#NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH
    )
    IF (NOT ORACLE_LIBRARY_CLNTST)
        IF (Oracle_FIND_REQUIRED)
            SET(FORCE_ERROR "CLNTST")
        ENDIF (Oracle_FIND_REQUIRED)
        set(ORACLE_LIBRARY_CLNTST "")
    ENDIF (NOT ORACLE_LIBRARY_CLNTST)
ENDIF (ORACLE_USE_CLNTST)

#set (ORACLE_LIBRARY_OCCI "")
IF (ORACLE_USE_OCCI)
    FIND_LIBRARY(
        ORACLE_LIBRARY_OCCI
        NAMES libocci occi 
        PATHS ${ORACLE_LIB_LOCATION}
        NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH
    )
    IF (NOT ORACLE_LIBRARY_OCCI)
        IF (Oracle_FIND_REQUIRED)
            SET(FORCE_ERROR "OCCI")
        ENDIF (Oracle_FIND_REQUIRED)
        set (ORACLE_LIBRARY_OCCI "")
    ENDIF (NOT ORACLE_LIBRARY_OCCI)
ENDIF (ORACLE_USE_OCCI)


IF (ORACLE_USE_XML)
    FIND_PATH(
        ORACLE_INCLUDES_XML
        xml.h
        ${ORACLE_INCLUDES_LOCATION}
        NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH
    )
    IF (NOT ORACLE_INCLUDES_XML)
        IF (Oracle_FIND_REQUIRED)
            SET(FORCE_ERROR "XML")
        ENDIF (Oracle_FIND_REQUIRED)
        set (ORACLE_INCLUDES_XML "")
    ENDIF (NOT ORACLE_INCLUDES_XML)

    FIND_LIBRARY(
        ORACLE_LIBRARY_XML
        NAMES libxml xml libxml10 xml10 libxml11 xml11 oraxml10 oraxml11
        PATHS ${ORACLE_LIB_LOCATION}
        NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH
    )
    IF (NOT ORACLE_LIBRARY_XML)
        IF (Oracle_FIND_REQUIRED)
            SET(FORCE_ERROR "XML")
        endif (Oracle_FIND_REQUIRED)
        set (ORACLE_LIBRARY_XML "")
    ENDIF (NOT ORACLE_LIBRARY_XML)
ENDIF (ORACLE_USE_XML)

IF (ORACLE_USE_OCIEI)
    FIND_LIBRARY(ORACLE_LIBRARY_OCIEI
        NAMES libociei ociei
        PATHS ${ORACLE_LIB_LOCATION}
        NO_DEFAULT_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_PATH NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH
    )
    IF (NOT ORACLE_LIBRARY_OCIEI)
        IF (Oracle_FIND_REQUIRED)
            SET(FORCE_ERROR "OCIEI")
        ENDIF (Oracle_FIND_REQUIRED)
        set (ORACLE_LIBRARY_OCIEI "")
    ENDIF (NOT ORACLE_LIBRARY_OCIEI)
ENDIF (ORACLE_USE_OCIEI)

IF (NOT FORCE_ERROR)
    SET (ORACLE_LIBRARIES ${ORACLE_LIBRARY_CLNTSH} ${ORACLE_LIBRARY_OCCI} ${ORACLE_LIBRARY_XML} ${ORACLE_LIBRARY_OCIEI})
ENDIF (NOT FORCE_ERROR)
MESSAGE(STATUS "ORACLE_LIBRARY_CLNTSH ${ORACLE_LIBRARY_CLNTSH}")
MESSAGE(STATUS "ORACLE_LIBRARY_CLNTST ${ORACLE_LIBRARY_CLNTST}")
MESSAGE(STATUS "ORACLE_LIBRARY_OCCI ${ORACLE_LIBRARY_OCCI}")
MESSAGE(STATUS "ORACLE_LIBRARY_XML ${ORACLE_LIBRARY_XML}")
MESSAGE(STATUS "ORACLE_LIBRARY_OCIEI ${ORACLE_LIBRARY_OCIEI}")


IF (ORACLE_INCLUDES AND ORACLE_LIBRARIES)
    SET(ORACLE_FOUND "YES")
ENDIF (ORACLE_INCLUDES AND ORACLE_LIBRARIES)

IF (ORACLE_INCLUDES_XML AND ORACLE_LIBRARY_XML)
    ADD_DEFINITIONS(-DORACLE_HAS_XML)
    SET(ORACLE_HAS_XML "YES")
    LIST(APPEND ORACLE_INCLUDES ${ORACLE_INCLUDES_XML})
ENDIF (ORACLE_INCLUDES_XML AND ORACLE_LIBRARY_XML)
  
MESSAGE(STATUS "Found Oracle: ${ORACLE_LIBRARIES} (ORACLE_HOME='${ORACLE_HOME}')")
MESSAGE(STATUS "    includes: ${ORACLE_INCLUDES}")
MESSAGE(STATUS "Found XML Oracle: ${ORACLE_INCLUDES_XML} ${ORACLE_LIBRARY_XML}")

IF (ORACLE_FOUND)
    # there *must* be OCI version defined for internal libraries
    IF (ORACLE_OCI_VERSION)
        ADD_DEFINITIONS(-DOTL_ORA${ORACLE_OCI_VERSION})
    ELSE (ORACLE_OCI_VERSION)
        MESSAGE(FATAL_ERROR "Set -DORACLE_OCI_VERSION for your oci. [8, 8I, 9I, 10G, 10G_R2]")
    ENDIF (ORACLE_OCI_VERSION)

ELSE (ORACLE_FOUND)

    MESSAGE(STATUS "Oracle not found.")
    MESSAGE(STATUS "Oracle: You can specify includes: -DORACLE_PATH_INCLUDES=/usr/include/oracle/10.2.0.3/client")
    MESSAGE(STATUS "   currently found includes: ${ORACLE_INCLUDES}")
    MESSAGE(STATUS "Oracle: You can specify libs: -DORACLE_PATH_LIB=/usr/lib/oracle/10.2.0.3/client/lib")
    MESSAGE(STATUS "   currently found libs: ${ORACLE_LIBRARIES}")

    IF (FORCE_ERROR)
        MESSAGE(STATUS "Failed module info: ${FORCE_ERROR}")
    ENDIF (FORCE_ERROR)
    IF (Oracle_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find Oracle library")
    ELSE (Oracle_FIND_REQUIRED)
        # setup the variables for silent continue
        SET (ORACLE_INCLUDES "")
    ENDIF (Oracle_FIND_REQUIRED)

ENDIF (ORACLE_FOUND)
