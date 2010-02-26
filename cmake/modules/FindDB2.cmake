# TOra: Configure DB2 libraries
#
# DB2_FOUND - system has a DB2 client
# DB2_INCLUDES - where to find header files
# DB2_LIBRARIES - the libraries to link against to use Oracle OCI
#
# copyright (c) 2009 Petr Vanek <petr@scribus.info>
# Redistribution and use is allowed according to the terms of the BSD license.
#

SET(DB2_FOUND "NO")


IF (DB2_PATH_INCLUDES)
    SET (DB2_INCLUDES_LOCATION ${DB2_PATH_INCLUDES})
ELSE (DB2_PATH_INCLUDES)
    SET (DB2_INCLUDES_LOCATION
            $ENV{HOME}/sqllib/include/
       )
ENDIF (DB2_PATH_INCLUDES)


# DB2 has really strange lib32/lib64 directories
IF (LIB_SUFFIX STREQUAL "64")
    SET(DB2_LIB_SUFFIX 64)
ELSE ()
    SET(DB2_LIB_SUFFIX 32)
ENDIF ()

IF (DB2_PATH_LIB)
    SET (DB2_LIB_LOCATION ${DB2_PATH_LIB})
ELSE (DB2_PATH_LIB)
    SET (DB2_LIB_LOCATION
            $ENV{HOME}/sqllib/lib${DB2_LIB_SUFFIX}
        )
ENDIF (DB2_PATH_LIB)

FIND_PATH(
    DB2_INCLUDES
    sqlcli1.h
    ${DB2_INCLUDES_LOCATION}
)

FIND_LIBRARY(
    DB2_LIBRARY_LIBDB2
    NAMES libdb2 db2
    PATHS ${DB2_LIB_LOCATION}
)

SET (DB2_LIBRARIES ${DB2_LIBRARY_LIBDB2})


IF (DB2_LIBRARIES AND DB2_INCLUDES)
    SET(DB2_FOUND "YES")
ENDIF (DB2_LIBRARIES AND DB2_INCLUDES)

IF (DB2_FOUND)
    MESSAGE(STATUS "Found DB2: ${DB2_LIBRARIES}")
ELSE (DB2_FOUND)
    MESSAGE(STATUS "DB2 not found.")
    MESSAGE(STATUS "DB2: You can specify includes: -DDB2_PATH_INCLUDES=/opt/db2/V9.5/includes")
    MESSAGE(STATUS "     currently found includes: ${DB2_INCLUDES}")
    MESSAGE(STATUS "DB2: You can specify libs: -DDB2_PATH_LIB=/opt/db2/V9.5/lib64")
    MESSAGE(STATUS "     currently found libs: ${DB2_LIBRARIES}")
    IF (DB2_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find DB2 library")
    ENDIF (DB2_FIND_REQUIRED)
ENDIF (DB2_FOUND)

