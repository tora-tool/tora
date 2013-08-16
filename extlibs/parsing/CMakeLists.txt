set(PARSING_SRCS
	error_handler.c
	error_handler.h
	macros.h
	OraclePLSQLLexer.c
	OraclePLSQLLexer.h
	OraclePLSQLParser.c
	OraclePLSQLParser.h
	OracleSQLLexer.c
	OracleSQLLexer.h
	OracleSQLParser.c
	OracleSQLParser.h
	tplsqlparseoracle.cpp
	tsqllexeroracle.cpp
	tsqlparseoracle.cpp
	tsqllexeroracle2.cpp
	PLSQLGuiLexer.cpp
)

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/extlibs/libantlr3c-3.3/include
  ${CMAKE_BINARY_DIR}/extlibs/libantlr3c-3.3          # generated antlr3config.h
  ${CMAKE_SOURCE_DIR}/extlibs/libantlr3cpp-3.5.1/include
  ${CMAKE_SOURCE_DIR}/src                             # core/tologger.h
)

IF(Boost_FOUND)
  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
ENDIF()

ADD_DEFINITIONS("-DTSQLPARSER_MAKE_DLL")

# Windows DLL cannot refer to symbols defined in the main application or in previously loaded DLLs. 
# and the parsing.lib reffers to tora logging mechanism
# therefore both libs antlr3c and parsing are static on Windows, while beeing shared on Linux
# Note: if they were static on Linux they would need --whole-archive link option.
#if(WIN32)
#  ADD_LIBRARY(parsing STATIC ${PARSING_SRCS})
#else(WIN32)
  ADD_LIBRARY(parsing SHARED ${PARSING_SRCS})
  SET_TARGET_PROPERTIES(parsing PROPERTIES ENABLE_EXPORTS ON)
  TARGET_LINK_LIBRARIES(parsing ${QT_LIBRARIES} antlr3c ${TORA_LIB})
  SET(LIBRARY_OUTPUT_PATH   "${CMAKE_BINARY_DIR}/src/")
#endif(WIN32)
