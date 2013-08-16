
##project(libantlr3c)
OPTION(ANTLR3_NODEBUGGER "ANTLR debugger not required" ON)

SET (VERSION "3.3")
SET (PACKAGE_BUGREPORT "jimi@temporal-wave.com")
SET (PACKAGE ${CMAKE_PROJECT_NAME})

SET (PACKAGE_VERSION ${VERSION})
SET (PACKAGE_NAME ${CMAKE_PROJECT_NAME})
SET (PACKAGE_TARNAME ${CMAKE_PROJECT_NAME})
SET (PACKAGE_STRING "${CMAKE_PROJECT_NAME} ${VERSION}")

cmake_minimum_required(VERSION 2.6)

INCLUDE (CheckIncludeFiles)
INCLUDE (CheckFunctionExists)

CHECK_INCLUDE_FILES ( "arpa_nameser.h" HAVE_ARPA_NAMESER_H )
CHECK_INCLUDE_FILES ( "ctype.h" HAVE_CTYPE_H )
CHECK_INCLUDE_FILES ( "dlfcn.h" HAVE_DLFCN_H )
CHECK_INCLUDE_FILES ( "inttypes.h" HAVE_INTTYPES_H )
CHECK_INCLUDE_FILES ( "malloc.h" HAVE_MALLOC_H )
CHECK_INCLUDE_FILES ( "sys/malloc.h" HAVE_SYS_MALLOC_H )
CHECK_INCLUDE_FILES ( "memory.h" HAVE_MEMORY_H )
CHECK_INCLUDE_FILES ( "netdb.h" HAVE_NETDB_H )
CHECK_INCLUDE_FILES ( "netinet/in.h" HAVE_NETINET_IN_H )
CHECK_INCLUDE_FILES ( "netinet/tcp.h" HAVE_NETINET_TCP_H )
CHECK_INCLUDE_FILES ( "resolv.h" HAVE_RESOLV_H )
CHECK_INCLUDE_FILES ( "sys/resolv.h" HAVE_RESOLV_H )
CHECK_INCLUDE_FILES ( "socket.h" HAVE_SOCKET_H )
CHECK_INCLUDE_FILES ( "sys/socket.h" HAVE_SYS_SOCKET_H )
CHECK_INCLUDE_FILES ( "stdarg.h" HAVE_STDARG_H )
CHECK_INCLUDE_FILES ( "stdint.h" HAVE_STDINT_H )
CHECK_INCLUDE_FILES ( "stdlib.h" HAVE_STDLIB_H )
CHECK_INCLUDE_FILES ( "string.h" HAVE_STRING_H )
CHECK_INCLUDE_FILES ( "strings.h" HAVE_STRINGS_H )
CHECK_INCLUDE_FILES ( "sys/stat.h" HAVE_SYS_STAT_H )
CHECK_INCLUDE_FILES ( "sys/types.h" HAVE_SYS_TYPES_H )
CHECK_INCLUDE_FILES ( "unistd.h" HAVE_UNISTD_H )

CHECK_FUNCTION_EXISTS ( "accept" HAVE_ACCEPT )
CHECK_FUNCTION_EXISTS ( "memmove" HAVE_MEMMOVE )
CHECK_FUNCTION_EXISTS ( "memset" HAVE_MEMSET ) 
CHECK_FUNCTION_EXISTS ( "strdup" HAVE_STRDUP )

IF(CMAKE_SIZEOF_VOID_P EQUAL "8")
  SET(ANTLR3_USE_64BIT 1)
ENDIF(CMAKE_SIZEOF_VOID_P EQUAL "8")
  
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/antlr3config.h.in.cmake ${CMAKE_CURRENT_BINARY_DIR}/antlr3config.h)

set(ANTLR3_SRCS
  src/antlr3baserecognizer.c
  src/antlr3basetreeadaptor.c
  src/antlr3basetree.c
  src/antlr3bitset.c
  src/antlr3collections.c
  src/antlr3commontoken.c
  src/antlr3commontreeadaptor.c
  src/antlr3commontree.c
  src/antlr3commontreenodestream.c
  src/antlr3convertutf.c
  src/antlr3cyclicdfa.c
  src/antlr3debughandlers.c
  src/antlr3encodings.c
  src/antlr3exception.c
  src/antlr3filestream.c
  src/antlr3inputstream.c
  src/antlr3intstream.c
  src/antlr3lexer.c
  src/antlr3parser.c
  src/antlr3rewritestreams.c
  src/antlr3string.c
  src/antlr3tokenstream.c
  src/antlr3treeparser.c
)

set(ANTLR3_HDRS
  include/antlr3baserecognizer.h
  include/antlr3basetreeadaptor.h
  include/antlr3basetree.h
  include/antlr3bitset.h
  include/antlr3collections.h
  include/antlr3commontoken.h
  include/antlr3commontreeadaptor.h
  include/antlr3commontree.h
  include/antlr3commontreenodestream.h
  include/antlr3convertutf.h
  include/antlr3cyclicdfa.h
  include/antlr3debugeventlistener.h
  include/antlr3defs.h
  include/antlr3encodings.h
  include/antlr3errors.h
  include/antlr3exception.h
  include/antlr3filestream.h
  include/antlr3.h
  include/antlr3input.h
  include/antlr3interfaces.h
  include/antlr3intstream.h
  include/antlr3lexer.h
  include/antlr3memory.h
  include/antlr3parser.h
  include/antlr3parsetree.h
  include/antlr3recognizersharedstate.h
  include/antlr3rewritestreams.h
  include/antlr3string.h
  include/antlr3tokenstream.h
  include/antlr3treeparser.h

  antlr3config.h
  )

include_directories(include ${CMAKE_CURRENT_BINARY_DIR})

##if(WIN32)
##  add_library(antlr3c STATIC ${ANTLR3_SRCS})
##else(WIN32)
  add_library(antlr3c SHARED ${ANTLR3_SRCS})
  set(LIBRARY_OUTPUT_PATH   "${CMAKE_BINARY_DIR}/src/")
##endif(WIN32)
