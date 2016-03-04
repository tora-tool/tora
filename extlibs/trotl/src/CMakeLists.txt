INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/extlibs/stack
  ${ORACLE_INCLUDES}
  ${LOKI_INCLUDE_DIRS}
)

IF(Boost_FOUND)
  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
ENDIF()

SET(TROTL_SOURCES
  trotl_anydata.cpp
  trotl_collection.cpp
  trotl_conn.cpp
  trotl_convertor.cpp
  trotl_cursor.cpp
  trotl_date.cpp
  trotl_describe.cpp
  trotl_error.cpp  
  trotl_int.cpp
  trotl_lob.cpp
  trotl_misc.cpp
  trotl_spatial.cpp
  trotl_parser.cpp
  trotl_rid.cpp
  trotl_stat.cpp
  trotl_string.cpp
  trotl_var.cpp
)

IF(ORACLE_HAS_XML)
  LIST(APPEND TROTL_SOURCES trotl_xml.cpp)
ENDIF (ORACLE_HAS_XML)

ADD_DEFINITIONS(${TROTL_DLL_DEFINES})

IF(APPLE)
  ADD_LIBRARY(${LIB_NAME} STATIC ${TROTL_SOURCES})
ELSEIF(WIN32)
  ADD_LIBRARY(${LIB_NAME} SHARED ${TROTL_SOURCES})

  # include the stack library in dubug builds only
  IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
    TARGET_LINK_LIBRARIES(${LIB_NAME} ${ORACLE_LIBRARIES} ${STACK_LIB} ${Boost_SYSTEM_LIBRARY})
  ELSE(CMAKE_BUILD_TYPE STREQUAL "Debug")
    TARGET_LINK_LIBRARIES(${LIB_NAME} ${ORACLE_LIBRARIES})
  ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")

  SET(LIBRARY_OUTPUT_PATH   "${CMAKE_BINARY_DIR}/src/")

  install(TARGETS ${LIB_NAME} RUNTIME DESTINATION bin LIBRARY DESTINATION lib${LIB_SUFFIX})
ELSE()
  SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
  SET(CMAKE_INSTALL_RPATH "$ORIGIN/instantclient/:$$ORIGIN/instantclient/")

  ADD_LIBRARY(${LIB_NAME} SHARED ${TROTL_SOURCES})

  # include the stack library in dubug builds only
  IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
    TARGET_LINK_LIBRARIES(${LIB_NAME} ${ORACLE_LIBRARIES} ${STACK_LIB} ${Boost_SYSTEM_LIBRARY})
  ELSE(CMAKE_BUILD_TYPE STREQUAL "Debug")
    TARGET_LINK_LIBRARIES(${LIB_NAME} ${ORACLE_LIBRARIES})
  ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")

  SET(LIBRARY_OUTPUT_PATH   "${CMAKE_BINARY_DIR}/src/")

  install(TARGETS ${LIB_NAME} RUNTIME DESTINATION bin LIBRARY DESTINATION lib${LIB_SUFFIX})

ENDIF()
