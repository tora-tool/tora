# include the stack library in dubug builds only
IF (TORA_DEBUG)
  SET (STACK_LIB "stack_lib")
  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/extlibs/stack)
  ADD_SUBDIRECTORY(stack)
ENDIF (TORA_DEBUG)

# loki library
IF(WANT_INTERNAL_LOKI)
  SET (TORA_LOKI_LIB tora_loki_lib)
  ADD_SUBDIRECTORY(loki)
ENDIF(WANT_INTERNAL_LOKI)

IF(WANT_INTERNAL_QSCINTILLA)
  SET (TORA_QSCINTILLA_LIB tora_qscintilla2_lib)
  ADD_SUBDIRECTORY(qscintilla2)
ENDIF(WANT_INTERNAL_QSCINTILLA)

ADD_SUBDIRECTORY(libermodel)

# Disabled, partially moved into src/parsing
# ADD_SUBDIRECTORY(parsing) 

# trotl library
IF (ORACLE_FOUND)
  ADD_SUBDIRECTORY(trotl)
ENDIF(ORACLE_FOUND)

#ADD_SUBDIRECTORY(qdecimal)
