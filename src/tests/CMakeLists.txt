

#FOREACH(item ${TORA_MOC_SOURCES})
#STRING(REGEX MATCH ".*/core/moc_.*c..$" item ${item})
#IF(item)
# LIST(APPEND CORE_OBJECT_SOURCES ${item})
#ENDIF(item)
#EgNDFOREACH(item ${TORA_MOC_SOURCES})

IF(TORA_DEBUG AND TEST_APP1)
# test1
ADD_EXECUTABLE("test1" ${GUI_TYPE}
  tests/test1.cpp
  ${PCH_SOURCE}  
  ${CORE_SOURCES}        #$<TARGET_OBJECTS:CORE_OBJECT_LIB>
  ${PARSING_SOURCES}     #$<TARGET_OBJECTS:PARSING_OBJECT_LIB>
  ${WIDGETS_SOURCES}     #$<TARGET_OBJECTS:WIDGETS_OBJECT_LIB>
  ${LOGGING_SOURCES}
  )
TARGET_LINK_LIBRARIES("test1"
	Qt5::Core
	Qt5::Widgets
	Qt5::Gui
	Qt5::Network
	${TORA_LOKI_LIB}
	${TORA_QSCINTILLA_LIB}
	${QSCINTILLA_LIBRARIES} 
)
SET_TARGET_PROPERTIES("test1" PROPERTIES ENABLE_EXPORTS ON)
IF(PCH_DEFINED)
  ADD_PRECOMPILED_HEADER("test1" ${PCH_HEADER} FORCEINCLUDE)
ENDIF(PCH_DEFINED)
ENDIF(TORA_DEBUG AND TEST_APP1)

IF(TORA_DEBUG AND TEST_APP2)  
# test2
ADD_EXECUTABLE("test2" ${GUI_TYPE}
  tests/test2.cpp
  ${PCH_SOURCE}  
  ${CORE_SOURCES}
  ${WIDGETS_SOURCES}
  ${LOGGING_SOURCES}
  )
TARGET_LINK_LIBRARIES("test2"
	Qt5::Core
	Qt5::Widgets
	Qt5::Gui
	Qt5::Network
	${TORA_QSCINTILLA_LIB}
	${QSCINTILLA_LIBRARIES}
	${TORA_LOKI_LIB}
)
SET_TARGET_PROPERTIES("test2" PROPERTIES ENABLE_EXPORTS ON)
IF(PCH_DEFINED)
  ADD_PRECOMPILED_HEADER("test2" ${PCH_HEADER} FORCEINCLUDE)
ENDIF(PCH_DEFINED)
ENDIF(TORA_DEBUG AND TEST_APP2)

IF(TORA_DEBUG AND TEST_APP3)
# test3
QT5_WRAP_CPP(TEST3_MOC_SOURCES tests/test3window.h tools/toworksheet.h editor/tosyntaxanalyzer.h editor/toscintilla.h tools/toresultview.h tools/toresulttableview.h)
ADD_EXECUTABLE("test3" ${GUI_TYPE}
  tests/test3.cpp
  tests/test3window.cpp
  editor/toscintilla.cpp
  editor/tosyntaxanalyzer.cpp
  editor/tosyntaxanalyzernl.cpp
  editor/tosyntaxanalyzeroracle.cpp
  tools/toworksheet.cpp
  tools/toresultview.cpp
  tools/toresulttableview.cpp
  ${TEST3_MOC_SOURCES}
  ${PCH_SOURCE}  
  ${CORE_SOURCES}    
  ${PARSING_SOURCES} 
  ${WIDGETS_SOURCES} 
  ${LOGGING_SOURCES}
  )
TARGET_LINK_LIBRARIES("test3"
	Qt5::Core
	Qt5::Widgets
	Qt5::Gui
	Qt5::Network
	Qt5::PrintSupport
	${TORA_QSCINTILLA_LIB}
	${QSCINTILLA_LIBRARIES}	
	${TORA_LOKI_LIB}
	)
IF(PCH_DEFINED)
  ADD_PRECOMPILED_HEADER("test3" ${PCH_HEADER} FORCEINCLUDE)
ENDIF(PCH_DEFINED)
SET_TARGET_PROPERTIES("test3" PROPERTIES ENABLE_EXPORTS ON)
ENDIF(TORA_DEBUG AND TEST_APP3)

IF(TORA_DEBUG AND TEST_APP4)
# test4
QT5_WRAP_CPP(TEST4_MOC_SOURCES tests/test4window.h editor/tosqltext.h editor/toscintilla.h editor/tosyntaxanalyzer.h)
QT5_WRAP_UI(TEST4_UI_SOURCES tests/test4ui.ui)
ADD_EXECUTABLE("test4"
  tests/test4.cpp
  tests/test4window.cpp
  editor/tosqltext.cpp
  editor/toscintilla.cpp
  editor/tosyntaxanalyzer.cpp
  editor/tosyntaxanalyzernl.cpp
  editor/tosyntaxanalyzeroracle.cpp
  ${TEST4_MOC_SOURCES}
  ${TEST4_UI_SOURCES}
  ${PCH_SOURCE}  
  ${CORE_SOURCES}    
  ${PARSING_SOURCES} 
  ${WIDGETS_SOURCES} 
  ${LOGGING_SOURCES}
)
TARGET_LINK_LIBRARIES("test4"
	Qt5::Core
	Qt5::Widgets
	Qt5::Gui
	Qt5::Network
	Qt5::PrintSupport
	${TORA_QSCINTILLA_LIB}
	${QSCINTILLA_LIBRARIES}	
	${TORA_LOKI_LIB}
)
IF(PCH_DEFINED)
  ADD_PRECOMPILED_HEADER("test4" ${PCH_HEADER} FORCEINCLUDE)
ENDIF(PCH_DEFINED)
SET_TARGET_PROPERTIES("test4" PROPERTIES ENABLE_EXPORTS ON)
ENDIF(TORA_DEBUG AND TEST_APP4)

IF(TORA_DEBUG AND TEST_APP5)
# test5
QT5_WRAP_CPP(TEST5_MOC_SOURCES
	tests/test5window.h
	tools/toplsqltext.h
	result/toresulttabledata.h
	tools/tobrowserbasewidget.h
	tools/todescribe.h
	tools/toparamget.h
	tools/toplsqleditor.h
	tools/toresultcols.h
	tools/toresultdatasingle.h
	tools/toresulttableview.h
	tools/toresulttableviewedit.h
	)
ADD_EXECUTABLE("test5"
  tests/test5.cpp
  tests/test5window.cpp
  result/toresulttabledata.cpp
  tools/tobrowserbasewidget.cpp
  tools/todescribe.cpp
  tools/toparamget.cpp
  tools/toplsqleditor.cpp
  tools/toplsqltext.cpp
  tools/toresultcols.cpp
  tools/toresultdatasingle.cpp
  tools/toresulttableview.cpp
  tools/toresulttableviewedit.cpp
  ${TEST5_MOC_SOURCES}
  ${PCH_SOURCE}  
  ${CORE_SOURCES}   
  ${PARSING_SOURCES}
  ${WIDGETS_SOURCES}
  ${LOGGING_SOURCES}
  ${EDITOR_SOURCES}
  )
TARGET_LINK_LIBRARIES("test5"
	Qt5::Core
	Qt5::Widgets
	Qt5::Gui
	Qt5::Network
	Qt5::PrintSupport
	${TORA_LOKI_LIB}
	${TORA_QSCINTILLA_LIB}
	${QSCINTILLA_LIBRARIES}
)
IF(PCH_DEFINED)
  ADD_PRECOMPILED_HEADER("test5" ${PCH_HEADER} FORCEINCLUDE)
ENDIF(PCH_DEFINED)
SET_TARGET_PROPERTIES("test5" PROPERTIES ENABLE_EXPORTS ON)
ENDIF(TORA_DEBUG AND TEST_APP5)

IF(TORA_DEBUG AND TEST_APP6)
# test6
ADD_EXECUTABLE("test6" ${GUI_TYPE}
  tests/test6.cpp
  ${PCH_SOURCE}
  )
TARGET_LINK_LIBRARIES("test6"
	ermodel
	Qt5::Core
	Qt5::Widgets
	Qt5::Gui
)
IF(PCH_DEFINED)
  ADD_PRECOMPILED_HEADER("test6" ${PCH_HEADER} FORCEINCLUDE)
ENDIF(PCH_DEFINED)
SET_TARGET_PROPERTIES("test6" PROPERTIES ENABLE_EXPORTS ON)
ENDIF(TORA_DEBUG AND TEST_APP6)

IF(TORA_DEBUG AND TEST_APP7)
# test7
ADD_EXECUTABLE("test7"
  tests/test7.cpp  
  ${PCH_SOURCE}
  ${CORE_SOURCES}
  ${WIDGETS_SOURCES}
  ${PARSING_SOURCES}
  ${LOGGING_SOURCES}  
  )
TARGET_LINK_LIBRARIES("test7"
	Qt5::Core
	Qt5::Widgets
	Qt5::Gui
	Qt5::Network
	${TORA_LOKI_LIB}	
	${TORA_QSCINTILLA_LIB}
	${QSCINTILLA_LIBRARIES}
)
IF(PCH_DEFINED)
  ADD_PRECOMPILED_HEADER("test7" ${PCH_HEADER} FORCEINCLUDE)
ENDIF(PCH_DEFINED)
SET_TARGET_PROPERTIES("test7" PROPERTIES ENABLE_EXPORTS ON)
ENDIF(TORA_DEBUG AND TEST_APP7)

