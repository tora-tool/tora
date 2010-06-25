SET(TROTL_DLL_DEFINES "-DLOKI_STATIC -DEXPLICIT_EXPORT -DTROTL_MAKE_DLL")
SET(TROTL_CLIENT_DEFINES "-DLOKI_STATIC -DEXPLICIT_EXPORT -DTROTL_DLL")

INCLUDE_DIRECTORIES(
  ${ORACLE_INCLUDES}
)

# include the stack library in dubug builds only
IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET (STACK_LIB "stack_lib")
    ADD_SUBDIRECTORY(stack)
    INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/stack)
    LINK_DIRECTORIES(./stack)
ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")

SET(TROTL_SOURCES
  trotl_aq.cpp
  trotl_anydata.cpp
  trotl_conn.cpp
  trotl_convertor.cpp
  trotl_date.cpp
  trotl_exc.cpp
  trotl_extended_parser.cpp
  trotl_int.cpp
  trotl_misc.cpp
  trotl_string.cpp
  trotl_lob.cpp
  trotl_parser.cpp
  trotl_rid.cpp
  trotl_stat.cpp
  trotl_var.cpp
)

IF(ORACLE_HAS_XML)
  LIST(APPEND TROTL_SOURCES trotl_xml.cpp)
ENDIF (ORACLE_HAS_XML)


#IF(WIN32)
#    IF (MSVC)
#        # this is a must to switch off subsystem:console (console window poping up)
#        # WinMain wrapper from QT_QTMAIN_LIBRARY is used and liner must be modified
#        LINK_LIBRARIES(${QT_QTMAIN_LIBRARY})
#        SET (WIN32_SUBSYSTEM WIN32)
#		IF (WANT_DEBUG)
#	        SET (CMAKE_EXE_LINKER_FLAGS "/NODEFAULTLIB:msvcrtd.lib ${CMAKE_EXE_LINKER_FLAGS}")
#		ELSE (WANT_DEBUG)
#            SET (CMAKE_EXE_LINKER_FLAGS "/NODEFAULTLIB:msvcrt.lib ${CMAKE_EXE_LINKER_FLAGS}")
#		ENDIF (WANT_DEBUG)
#    ENDIF (MSVC)
#ENDIF(WIN32)

SET_SOURCE_FILES_PROPERTIES(${TROTL_SOURCES} COMPILE_FLAGS ${TROTL_DLL_DEFINES})
ADD_LIBRARY(${LIB_NAME} SHARED ${TROTL_SOURCES})
TARGET_LINK_LIBRARIES(${LIB_NAME} ${ORACLE_LIBRARIES} ${STACK_LIB})
