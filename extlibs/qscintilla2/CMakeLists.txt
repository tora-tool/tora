# Warning: all lexers compilation and building is mandatory for successful linking!
# Warning: it is possible that FILE GLOB is evaluated only once - enforce cmake re-run if qsciscintilla version changes

ADD_DEFINITIONS("-DQT")
ADD_DEFINITIONS("-DSCI_LEXER")
ADD_DEFINITIONS("-DQSCINTILLA_MAKE_DLL")
ADD_DEFINITIONS("-DSCINTILLA_QT")

SET(qsciscintilla_VERSION_MAJOR 2)
SET(qsciscintilla_VERSION_MINOR 8)
SET(qsciscintilla_VERSION_PATCH 0)
SET(qsciscintilla_VERSION ${qsciscintilla_VERSION_MAJOR}.${qsciscintilla_VERSION_MINOR}.${qsciscintilla_VERSION_PATCH})

FILE(GLOB TORA_QSCINTILLA2_MOC_CLASSES "include/*.h" "lexlib/*.h" "Qt4Qt5/*.h" "Qt4Qt5/Qsci/*.h" "src/*.h")

FILE(GLOB TORA_QSCINTILLA2_LIB_SOURCES "lexers/*.cpp" "lexlib/*.cpp" "Qt4Qt5/*.cpp" "src/*.cpp")

QT4_WRAP_CPP(TORA_QSCINTILLA2_MOC_SOURCES ${TORA_QSCINTILLA2_MOC_CLASSES})

SET(TORA_QSCINTILLA2_LIB "tora_qscintilla2_lib")
ADD_LIBRARY(${TORA_QSCINTILLA2_LIB} STATIC ${TORA_QSCINTILLA2_LIB_SOURCES} ${TORA_QSCINTILLA2_MOC_SOURCES})

SET_TARGET_PROPERTIES(${TORA_QSCINTILLA2_LIB} PROPERTIES VERSION ${qsciscintilla_VERSION} SOVERSION ${qsciscintilla_VERSION_MAJOR})

