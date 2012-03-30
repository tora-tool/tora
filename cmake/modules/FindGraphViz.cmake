# This cmake file comes from MOAB:
# MOAB, a Mesh-Oriented datABase, is a software component for creating,
# storing and accessing finite element mesh data.
# 
# Copyright 2004 Sandia Corporation.  Under the terms of Contract
# DE-AC04-94AL85000 with Sandia Coroporation, the U.S. Government
# retains certain rights in this software.
# 
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.  A copy of the full
# GNU Lesser General Public License can be found at
# http://www.gnu.org/copyleft/lesser.html.
# 
# For more information, contact the authors of this software at
# moab@sandia.gov.
# 
# this will set the following variables:
# graphviz_LIBRARIES
# graphviz_FOUND
# graphviz_INCLUDE_DIRECTORIES

if ( NOT WIN32 )
  
  find_package(PkgConfig)
  pkg_check_modules( graphviz ${REQUIRED} libgvc libcdt libgraph libpathplan )
  if ( graphviz_FOUND )
    set ( graphviz_INCLUDE_DIRECTORIES ${graphviz_INCLUDE_DIRS} )
  endif ( graphviz_FOUND )
    
endif ( NOT WIN32 )
    
find_path( graphviz_INCLUDE_DIRECTORIES
    NAMES gvc.h
    PATHS
    ${graphviz_INCLUDE_DIRS}
    /usr/local/include
    /usr/include
    PATH_SUFFIXES graphviz
    )
    
find_library( graphviz_GVC_LIBRARY
    NAMES gvc
    PATHS
    ${graphviz_LIBRARY_DIRS}
    /usr/local/lib64
    /usr/lib64
    /usr/local/lib
    /usr/lib
    )

find_library( graphviz_CDT_LIBRARY
    NAMES cdt
    PATHS
    ${graphviz_LIBRARY_DIRS}
    /usr/local/lib64
    /usr/lib64
    /usr/local/lib
    /usr/lib
    )

find_library( graphviz_GRAPH_LIBRARY
    NAMES graph
    PATHS
    ${graphviz_LIBRARY_DIRS}
    /usr/local/lib64
    /usr/lib64
    /usr/local/lib
    /usr/lib
    )

find_library( graphviz_PATHPLAN_LIBRARY
    NAMES pathplan
    PATHS
    ${graphviz_LIBRARY_DIRS}
    /usr/local/lib64
    /usr/lib64
    /usr/local/lib
    /usr/lib
    )

if ( graphviz_INCLUDE_DIRECTORIES AND
      graphviz_GVC_LIBRARY AND graphviz_CDT_LIBRARY AND
      graphviz_GRAPH_LIBRARY AND graphviz_PATHPLAN_LIBRARY )
  set ( graphviz_FOUND 1 )
  set ( graphviz_LIBRARIES
      "${graphviz_GVC_LIBRARY};${graphviz_GRAPH_LIBRARY};"
      "${graphviz_CDT_LIBRARY};${graphviz_PATHPLAN_LIBRARY}"
      CACHE FILEPATH "Libraries for graphviz" )
else ( graphviz_INCLUDE_DIRECTORIES AND
        graphviz_GVC_LIBRARY AND graphviz_CDT_LIBRARY AND
        graphviz_GRAPH_LIBRARY AND graphviz_PATHPLAN_LIBRARY )
  set ( graphviz_FOUND 0 )
endif ( graphviz_INCLUDE_DIRECTORIES AND
        graphviz_GVC_LIBRARY AND graphviz_CDT_LIBRARY AND
        graphviz_GRAPH_LIBRARY AND graphviz_PATHPLAN_LIBRARY )


find_program(DOT dot)

if (DOT)

EXECUTE_PROCESS(COMMAND dot -V
  OUTPUT_VARIABLE _dot_query_output
  RESULT_VARIABLE _dot_result
  ERROR_VARIABLE _dot_query_output )

IF(_dot_result MATCHES 0)
  STRING(REGEX REPLACE ".*([0-9]+\\.[0-9]+\\.[0-9]+).*" "\\1" GRAPHVIZVERSION "${_dot_query_output}" )
ELSE(_dot_result MATCHES 0)
  SET(_dot_query_output)
ENDIF(_dot_result MATCHES 0)
IF (GRAPHVIZVERSION)

# we need at least version 2.22.0
IF (NOT GRAPHVIZ_MIN_VERSION)
SET(GRAPHVIZ_MIN_VERSION "2.22.0")
ENDIF (NOT GRAPHVIZ_MIN_VERSION)

#now parse the parts of the user given version string into variables
STRING(REGEX MATCH "^[0-9]+\\.[0-9]+\\.[0-9]+" req_graphviz_major_vers "${GRAPHVIZ_MIN_VERSION}")
IF (NOT req_graphviz_major_vers)
MESSAGE( FATAL_ERROR "Invalid GraphViz version string given: \"${GRAPHVIZ_MIN_VERSION}\", expected e.g. \"2.22.0\"")
ENDIF (NOT req_graphviz_major_vers)

# now parse the parts of the user given version string into variables
STRING(REGEX REPLACE "^([0-9]+)\\.[0-9]+\\.[0-9]+" "\\1" req_graphviz_major_vers "${GRAPHVIZ_MIN_VERSION}")
STRING(REGEX REPLACE "^[0-9]+\\.([0-9])+\\.[0-9]+" "\\1" req_graphviz_minor_vers "${GRAPHVIZ_MIN_VERSION}")
STRING(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+)" "\\1" req_graphviz_patch_vers "${GRAPHVIZ_MIN_VERSION}")

IF (NOT req_graphviz_major_vers EQUAL 2)
MESSAGE( FATAL_ERROR "Invalid GraphViz version string given: \"${GRAPHVIZ_MIN_VERSION}\", major version 4 is required, e.g. \"2.22.0\"")
ENDIF (NOT req_graphviz_major_vers EQUAL 2)

# and now the version string given by qmake
STRING(REGEX REPLACE "^([0-9]+)\\.[0-9]+\\.[0-9]+.*" "\\1" GRAPHVIZ_VERSION_MAJOR "${GRAPHVIZVERSION}")
STRING(REGEX REPLACE "^[0-9]+\\.([0-9])+\\.[0-9]+.*" "\\1" GRAPHVIZ_VERSION_MINOR "${GRAPHVIZVERSION}")
STRING(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" GRAPHVIZ_VERSION_PATCH "${GRAPHVIZVERSION}")

# compute an overall version number which can be compared at once
MATH(EXPR req_vers "${req_graphviz_major_vers}*10000 + ${req_graphviz_minor_vers}*100 + ${req_graphviz_patch_vers}")
MATH(EXPR found_vers "${GRAPHVIZ_VERSION_MAJOR}*10000 + ${GRAPHVIZ_VERSION_MINOR}*100 + ${GRAPHVIZ_VERSION_PATCH}")

IF (found_vers LESS req_vers)
SET(graphviz_FOUND FALSE)
SET(GRAPHVIZ_INSTALLED_VERSION_TOO_OLD TRUE)
ELSE (found_vers LESS req_vers)
SET(graphviz_FOUND TRUE)
ENDIF (found_vers LESS req_vers)
ENDIF (GRAPHVIZVERSION)


endif (DOT)

include(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set GRAPHVIZ_FOUND to TRUE if 
# all listed variables are TRUE
find_package_handle_standard_args(GraphViz DEFAULT_MSG graphviz_LIBRARIES graphviz_INCLUDE_DIRECTORIES)