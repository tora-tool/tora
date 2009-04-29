# TOra: Configure Oracle libraries
#
# ORACLE_FOUND - system has Oracle OCI
# ORACLE_INCLUDES - where to find oci.h
# ORACLE_LIBRARIES - the libraries to link against to use Oracle OCI
#
# copyright (c) 2007 Petr Vanek <petr@scribus.info>
# Redistribution and use is allowed according to the terms of the GPLv2 license.
#

SET(ORACLE_FOUND "NO")


set(ORACLE_HOME $ENV{ORACLE_HOME})


IF (ORACLE_PATH_INCLUDES)
    SET (ORACLE_INCLUDES_LOCATION ${ORACLE_PATH_INCLUDES})
ELSE (ORACLE_PATH_INCLUDES)
    SET (ORACLE_INCLUDES_LOCATION
            ${ORACLE_HOME}/rdbms/public
            ${ORACLE_HOME}/include
            # sdk
            ${ORACLE_HOME}/sdk/include
            # xe on windows
            ${ORACLE_HOME}/OCI/include
       )
ENDIF (ORACLE_PATH_INCLUDES)

IF (ORACLE_PATH_LIB)
    SET (ORACLE_LIB_LOCATION ${ORACLE_PATH_LIB})
ELSE (ORACLE_PATH_LIB)
    SET (ORACLE_LIB_LOCATION
            ${ORACLE_HOME}/lib
            # xe on windows
            ${ORACLE_HOME}/OCI/lib/MSVC
        )
ENDIF (ORACLE_PATH_LIB)

FIND_PATH(
    ORACLE_INCLUDES
    oci.h
    ${ORACLE_INCLUDES_LOCATION}
)

FIND_LIBRARY(
    ORACLE_LIBRARY_OCCI
    NAMES libocci occi oraocci10
    PATHS ${ORACLE_LIB_LOCATION}
)
FIND_LIBRARY(
    ORACLE_LIBRARY_CLNTSH
    NAMES libclntsh clntsh oci
    PATHS ${ORACLE_LIB_LOCATION}
)
#FIND_LIBRARY(
#    ORACLE_LIBRARY_LNNZ
#    NAMES libnnz10 nnz10 libnnz11 nnz11 ociw32
#    PATHS ${ORACLE_LIB_LOCATION}
#)

SET (ORACLE_LIBRARY ${ORACLE_LIBRARY_OCCI} ${ORACLE_LIBRARY_CLNTSH} ${ORACLE_LIBRARY_LNNZ})

IF (ORACLE_LIBRARY AND ORACLE_INCLUDES)
    SET(ORACLE_LIBRARIES ${ORACLE_LIBRARY})
    SET(ORACLE_FOUND "YES")
ENDIF (ORACLE_LIBRARY AND ORACLE_INCLUDES)


# guess OCI version
IF (NOT DEFINED ORACLE_OCI_VERSION AND UNIX)
    FIND_PROGRAM(AWK awk)
    FIND_PROGRAM(SQLPLUS sqlplus PATHS ENV ORACLE_HOME NO_DEFAULT_PATH)
    FIND_PROGRAM(SQLPLUS sqlplus)
    SET (sqlplus_version "${SQLPLUS} -version | ${AWK} '/Release/ {print $3}'")
    EXEC_PROGRAM(${sqlplus_version} OUTPUT_VARIABLE sqlplus_out)
    MESSAGE(STATUS "found sqlplus version: ${sqlplus_out}")

    # WARNING!
    # MATCHES operator is using Cmake regular expression.
    # so the e.g. 9.* does not expand like shell file mask
    # but as "9 and then any sequence of characters"
    IF (${sqlplus_out} MATCHES "8.*")
        SET(ORACLE_OCI_VERSION "8I")
    ELSEIF (${sqlplus_out} MATCHES "9.*")
        SET(ORACLE_OCI_VERSION "9")
# do not change the order of the ora10 checking!
    ELSEIF (${sqlplus_out} MATCHES "10.2.*")
        SET(ORACLE_OCI_VERSION "10G_R2")
    ELSEIF (${sqlplus_out} MATCHES "10.*")
        SET(ORACLE_OCI_VERSION "10G")
    ELSEIF (${sqlplus_out} MATCHES "11.*")
        SET(ORACLE_OCI_VERSION "11G")
    ELSE (${sqlplus_out} MATCHES "8.*")
        SET(ORACLE_OCI_VERSION "10G_R2")
    ENDIF (${sqlplus_out} MATCHES "8.*")

    MESSAGE(STATUS "Guessed ORACLE_OCI_VERSION value: ${ORACLE_OCI_VERSION}")
ENDIF (NOT DEFINED ORACLE_OCI_VERSION AND UNIX)


IF (ORACLE_FOUND)
    IF (NOT ORACLE_FIND_QUIETLY)
         MESSAGE(STATUS "Found Oracle: ${ORACLE_LIBRARY} (ORACLE_HOME='${ORACLE_HOME}')")
    ENDIF (NOT ORACLE_FIND_QUIETLY)
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
    MESSAGE(STATUS "   currently found libs: ${ORACLE_LIBRARY}")
    IF (ORACLE_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find Oracle library")
    ELSE (ORACLE_FIND_REQUIRED)
        # setup the variables for silent continue
        SET (ORACLE_INCLUDES "")
    ENDIF (ORACLE_FIND_REQUIRED)
ENDIF (ORACLE_FOUND)
