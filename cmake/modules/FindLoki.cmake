# - Find loki
# Find LOKI headers and libraries.
#
#  LOKI_INCLUDE_DIRS - where to find tbb uncludes.
#  LOKI_LIBRARIES    - List of libraries when using tbb.
#  LOKI_FOUND        - True if LOKI found.

# Look for the header file.
FIND_PATH(LOKI_INCLUDE_DIR NAMES loki/Singleton.h)

# Look for the library.
FIND_LIBRARY(LOKI_LIBRARY NAMES loki)

# handle the QUIETLY and REQUIRED arguments and set LOKI_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE( FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS( LOKI DEFAULT_MSG LOKI_LIBRARY LOKI_INCLUDE_DIR)

# Copy the results to the output variables.
IF(LOKI_FOUND)
  SET(LOKI_LIBRARIES ${LOKI_LIBRARY})
  SET(LOKI_INCLUDE_DIRS ${LOKI_INCLUDE_DIR})
ELSE(LOKI_FOUND)
  SET(LOKI_LIBRARIES)
  SET(LOKI_INCLUDE_DIRS)
ENDIF(LOKI_FOUND)

MARK_AS_ADVANCED(LOKI_INCLUDE_DIR LOKI_LIBRARY)
