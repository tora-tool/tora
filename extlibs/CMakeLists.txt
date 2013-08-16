# include the stack library in dubug builds only
IF (TORA_DEBUG)
  SET (STACK_LIB "stack_lib")
  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/extlibs/stack)
  ADD_SUBDIRECTORY(stack)
ENDIF (TORA_DEBUG)

# loki library
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/extlibs/loki/include)
ADD_SUBDIRECTORY(loki)

# trotl library
ADD_SUBDIRECTORY(trotl)

IF(WANT_INTERNAL_QSCINTILLA)
  SET (TORA_QSCINTILLA_LIB tora_qscintilla2_lib)
  ADD_SUBDIRECTORY(qscintilla2)
ENDIF(WANT_INTERNAL_QSCINTILLA)

ADD_SUBDIRECTORY(libantlr3c-3.3)

# Disabled, partially moved into src/parsing
# ADD_SUBDIRECTORY(parsing) 

IF (graphviz_FOUND)
  ADD_SUBDIRECTORY(libermodel)
ENDIF(graphviz_FOUND)



  
