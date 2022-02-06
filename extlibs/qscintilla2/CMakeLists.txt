# Warning: all lexers compilation and building is mandatory for successful linking!
# Warning: it is possible that FILE GLOB is evaluated only once - enforce cmake re-run if qsciscintilla version changes

ADD_DEFINITIONS("-DQT")
ADD_DEFINITIONS("-DSCI_LEXER")
ADD_DEFINITIONS("-DQSCINTILLA_MAKE_DLL")
ADD_DEFINITIONS("-DSCINTILLA_QT")

SET(qsciscintilla_VERSION_MAJOR 2)
SET(qsciscintilla_VERSION_MINOR 12)
SET(qsciscintilla_VERSION_PATCH 1)
SET(qsciscintilla_VERSION ${qsciscintilla_VERSION_MAJOR}.${qsciscintilla_VERSION_MINOR}.${qsciscintilla_VERSION_PATCH})

FILE(GLOB TORA_QSCINTILLA2_MOC_CLASSES "scintilla/include/*.h"  "scintilla/lexlib/*.h"   "scintilla/src/*.h"   "src/Qsci/*.h" "src/*.h")
FILE(GLOB TORA_QSCINTILLA2_LIB_SOURCES "scintilla/lexers/*.cpp" "scintilla/lexlib/*.cpp" "scintilla/src/*.cpp" "src/*.cpp")

INCLUDE_DIRECTORIES(${QT_INCLUDES} scintilla/include/ scintilla/lexlib/ scintilla/src/)

IF(USE_QT6)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)

  QT6_WRAP_CPP(TORA_QSCINTILLA2_MOC_SOURCES ${TORA_QSCINTILLA2_MOC_CLASSES})
ELSE()
  QT5_WRAP_CPP(TORA_QSCINTILLA2_MOC_SOURCES ${TORA_QSCINTILLA2_MOC_CLASSES})
ENDIF()

SET(TORA_QSCINTILLA2_LIB "tora_qscintilla2_lib")
ADD_LIBRARY(${TORA_QSCINTILLA2_LIB} STATIC ${TORA_QSCINTILLA2_LIB_SOURCES} ${TORA_QSCINTILLA2_MOC_SOURCES})

SET_TARGET_PROPERTIES(${TORA_QSCINTILLA2_LIB} PROPERTIES VERSION ${qsciscintilla_VERSION} SOVERSION ${qsciscintilla_VERSION_MAJOR})

