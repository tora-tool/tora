# Warning: all lexers compilation and building is mandatory for successful linking!

ADD_DEFINITIONS("-DQT")
ADD_DEFINITIONS("-DSCI_LEXER")

FILE(GLOB TORA_QSCINTILLA2_MOC_CLASSES "include/*.h" "lexlib/*.h" "Qt4Qt5/*.h" "Qt4Qt5/Qsci/*.h" "src/*.h")

FILE(GLOB TORA_QSCINTILLA2_LIB_SOURCES "lexers/*.cpp" "lexlib/*.cpp" "Qt4Qt5/*.cpp" "src/*.cpp")

QT4_WRAP_CPP(TORA_QSCINTILLA2_MOC_SOURCES ${TORA_QSCINTILLA2_MOC_CLASSES})

SET(TORA_QSCINTILLA2_LIB "tora_qscintilla2_lib")
ADD_LIBRARY(${TORA_QSCINTILLA2_LIB} STATIC ${TORA_QSCINTILLA2_LIB_SOURCES} ${TORA_QSCINTILLA2_MOC_SOURCES})
