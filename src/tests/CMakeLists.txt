# NOTE: also check src/core/tora_export.h

#FOREACH(item ${TORA_MOC_SOURCES})
#STRING(REGEX MATCH ".*/core/moc_.*c..$" item ${item})
#IF(item)
# LIST(APPEND CORE_OBJECT_SOURCES ${item})
#ENDIF(item)
#EgNDFOREACH(item ${TORA_MOC_SOURCES})

IF(TORA_DEBUG AND TEST_APP01)
# test01
ADD_EXECUTABLE("test01" ${GUI_TYPE}
  tests/test1.cpp
  ${PCH_SOURCE}  
  ${CORE_SOURCES}        #$<TARGET_OBJECTS:CORE_OBJECT_LIB>
  ${PARSING_SOURCES}     #$<TARGET_OBJECTS:PARSING_OBJECT_LIB>
  ${WIDGETS_SOURCES}     #$<TARGET_OBJECTS:WIDGETS_OBJECT_LIB>
  ${LOGGING_SOURCES}
  )
TARGET_LINK_LIBRARIES("test01"
	Qt5::Core
	Qt5::Widgets
	Qt5::Gui
	Qt5::Network
	${CMAKE_DL_LIBS}	
	${TORA_LOKI_LIB}
	${TORA_QSCINTILLA_LIB}
	${QSCINTILLA_LIBRARIES}
)
SET_TARGET_PROPERTIES("test01" PROPERTIES ENABLE_EXPORTS ON)
IF(PCH_DEFINED)
  ADD_PRECOMPILED_HEADER("test01" ${PCH_HEADER} FORCEINCLUDE)
ENDIF(PCH_DEFINED)
ENDIF(TORA_DEBUG AND TEST_APP01)

IF(TORA_DEBUG AND TEST_APP02)  
# test02
ADD_EXECUTABLE("test02" ${GUI_TYPE}
  tests/test2.cpp
  ${PCH_SOURCE}  
  ${CORE_SOURCES}
  ${WIDGETS_SOURCES}
  ${LOGGING_SOURCES}
  )
TARGET_LINK_LIBRARIES("test02"
	Qt5::Core
	Qt5::Widgets
	Qt5::Gui
	Qt5::Network
	${CMAKE_DL_LIBS}
	${TORA_QSCINTILLA_LIB}
	${QSCINTILLA_LIBRARIES}
	${TORA_LOKI_LIB}
)
SET_TARGET_PROPERTIES("test02" PROPERTIES ENABLE_EXPORTS ON)
IF(PCH_DEFINED)
  ADD_PRECOMPILED_HEADER("test02" ${PCH_HEADER} FORCEINCLUDE)
ENDIF(PCH_DEFINED)
ENDIF(TORA_DEBUG AND TEST_APP02)

IF(TORA_DEBUG AND TEST_APP03)
# test02
QT5_WRAP_CPP(TEST3_MOC_SOURCES tests/test3window.h
	connection/tooracleconfiguration.h
	result/toresulttabledata.h
	tools/tobarchart.h
	tools/tobrowserbasewidget.h
	tools/todescribe.h	
	tools/tolinechart.h
	tools/toparamget.h
	tools/toresultbar.h
	tools/toresultcols.h
	result/toresultdatasingle.h
	tools/toresultplan.h
	tools/toresultstats.h
	tools/toresulttableview.h
	tools/toresulttableviewedit.h
	tools/toresultview.h
	tools/toworksheet.h
	)
ADD_EXECUTABLE("test03" ${GUI_TYPE}
  tests/test3.cpp
  tests/test3window.cpp
  connection/tooracleconfiguration.cpp
  result/toresulttabledata.cpp
  tools/tobarchart.cpp
  tools/tobrowserbasewidget.cpp  
  tools/todescribe.cpp
  tools/tolinechart.cpp
  tools/toparamget.cpp
  tools/toresultbar.cpp
  tools/toresultcols.cpp
  result/toresultdatasingle.cpp
  tools/toresultplan.cpp
  tools/toresultstats.cpp
  tools/toresulttableview.cpp
  tools/toresulttableviewedit.cpp  
  tools/toresultview.cpp
  tools/toworksheet.cpp
  ${TEST3_MOC_SOURCES}
  ${PCH_SOURCE}  
  ${CORE_SOURCES}    
  ${PARSING_SOURCES} 
  ${WIDGETS_SOURCES}
  ${EDITOR_SOURCES}
  ${LOGGING_SOURCES}
  )
TARGET_LINK_LIBRARIES("test03"
	Qt5::Core
	Qt5::Widgets
	Qt5::Gui
	Qt5::Network
	Qt5::PrintSupport
	${CMAKE_DL_LIBS}	
	${TORA_QSCINTILLA_LIB}
	${QSCINTILLA_LIBRARIES}	
	${TORA_LOKI_LIB}
	)
IF(PCH_DEFINED)
  ADD_PRECOMPILED_HEADER("test03" ${PCH_HEADER} FORCEINCLUDE)
ENDIF(PCH_DEFINED)
SET_TARGET_PROPERTIES("test03" PROPERTIES ENABLE_EXPORTS ON)
ENDIF(TORA_DEBUG AND TEST_APP03)

IF(TORA_DEBUG AND TEST_APP04)
# test4
QT5_WRAP_CPP(TEST4_MOC_SOURCES
  tests/test4window.h
  widgets/tosearch.h
  editor/tomemoeditor.h
  editor/toscintilla.h
  editor/tosqltext.h
  editor/tosyntaxanalyzermysql.h
  editor/tosyntaxanalyzernl.h
  editor/tosyntaxanalyzeroracle.h
  editor/tosyntaxanalyzerpostgresql.h
  tools/toresultstats.h
  tools/toresultview.h
  )
QT5_WRAP_UI(TEST4_UI_SOURCES tests/test4ui.ui)
QT5_ADD_RESOURCES(TEST4_RC_SOURCES tests/test4.qrc)
ADD_EXECUTABLE("test04"
  tests/test4.cpp
  tests/test4window.cpp
  widgets/tosearch.cpp
  editor/tomemoeditor.cpp
  editor/toscintilla.cpp
  editor/tosqltext.cpp
  editor/tosyntaxanalyzermysql.cpp
  editor/tosyntaxanalyzernl.cpp
  editor/tosyntaxanalyzeroracle.cpp
  editor/tosyntaxanalyzerpostgresql.cpp
  tools/toresultstats.cpp
  tools/toresultview.cpp
  ${TEST4_MOC_SOURCES}
  ${TEST4_UI_SOURCES}
  ${TEST4_RC_SOURCES}
  ${PCH_SOURCE}  
  ${CORE_SOURCES}    
  ${PARSING_SOURCES} 
  ${WIDGETS_SOURCES} 
  ${LOGGING_SOURCES}
)
TARGET_LINK_LIBRARIES("test04"
	Qt5::Core
	Qt5::Widgets
	Qt5::Gui
	Qt5::Network
	Qt5::PrintSupport
	${CMAKE_DL_LIBS}
	${TORA_QSCINTILLA_LIB}
	${QSCINTILLA_LIBRARIES}	
	${TORA_LOKI_LIB}
)
IF(PCH_DEFINED)
  ADD_PRECOMPILED_HEADER("test04" ${PCH_HEADER} FORCEINCLUDE)
ENDIF(PCH_DEFINED)
SET_TARGET_PROPERTIES("test04" PROPERTIES ENABLE_EXPORTS ON)
ENDIF(TORA_DEBUG AND TEST_APP04)

IF(TORA_DEBUG AND TEST_APP05)
# test5
QT5_WRAP_CPP(TEST5_MOC_SOURCES
  tests/test5window.h
  result/toresulttabledata.h
  tools/tobrowserbasewidget.h
  tools/todescribe.h
  tools/toparamget.h
  tools/toresultview.h
  tools/toresultcols.h
  result/toresultdatasingle.h
  tools/toresulttableview.h
  tools/toresulttableviewedit.h
  # tools
  tools/toplsqleditor.h
  tools/toplsqltext.h
  tools/toworksheet.h
  tools/toresultstats.h
  tools/toresultplan.h
  tools/tolinechart.h
  tools/toresultbar.h
  tools/tobarchart.h
	)
ADD_EXECUTABLE("test05"
  tests/test5.cpp
  tests/test5window.cpp
  result/toresulttabledata.cpp
  tools/tobrowserbasewidget.cpp
  tools/todescribe.cpp
  tools/toparamget.cpp
  tools/toresultcols.cpp
  tools/toresultview.cpp
  result/toresultdatasingle.cpp
  tools/toresulttableview.cpp
  tools/toresulttableviewedit.cpp
  tools/toresultplan.cpp
  ${TEST5_MOC_SOURCES}
  ${PCH_SOURCE}  
  ${CORE_SOURCES}   
  ${PARSING_SOURCES}
  ${WIDGETS_SOURCES}
  ${LOGGING_SOURCES}
  ${EDITOR_SOURCES}
  ${ORACLE_SOURCES}
  # worksheet tool deps
  tools/toresultstats.cpp
  tools/tolinechart.cpp
  tools/tolinechart.cpp
  tools/toresultbar.cpp
  tools/tobarchart.cpp
  tools/toworksheet.cpp
  # pl/sql tool deps
  tools/toplsqleditor.cpp
  tools/toplsqltext.cpp
  )
TARGET_LINK_LIBRARIES("test05"
	Qt5::Core
	Qt5::Widgets
	Qt5::Gui
	Qt5::Network
	Qt5::PrintSupport
	${CMAKE_DL_LIBS}
	${TORA_LOKI_LIB}
	${TORA_QSCINTILLA_LIB}
	${QSCINTILLA_LIBRARIES}
)
IF(PCH_DEFINED)
  ADD_PRECOMPILED_HEADER("test05" ${PCH_HEADER} FORCEINCLUDE)
ENDIF(PCH_DEFINED)
SET_TARGET_PROPERTIES("test05" PROPERTIES ENABLE_EXPORTS ON)
ENDIF(TORA_DEBUG AND TEST_APP05)

IF(TORA_DEBUG AND TEST_APP06)
# test6
ADD_EXECUTABLE("test06" ${GUI_TYPE}
  tests/test6.cpp
  ${PCH_SOURCE}
  )
TARGET_LINK_LIBRARIES("test06"
	ermodel
	Qt5::Core
	Qt5::Widgets
	Qt5::Gui
)
IF(PCH_DEFINED)
  ADD_PRECOMPILED_HEADER("test06" ${PCH_HEADER} FORCEINCLUDE)
ENDIF(PCH_DEFINED)
SET_TARGET_PROPERTIES("test06" PROPERTIES ENABLE_EXPORTS ON)
ENDIF(TORA_DEBUG AND TEST_APP06)

IF(TORA_DEBUG AND TEST_APP07)
# test7
ADD_EXECUTABLE("test07"
  tests/test7.cpp  
  ${PCH_SOURCE}
  ${CORE_SOURCES}
  ${WIDGETS_SOURCES}
  ${PARSING_SOURCES}
  ${LOGGING_SOURCES}  
  )
TARGET_LINK_LIBRARIES("test07"
	Qt5::Core
	Qt5::Widgets
	Qt5::Gui
	Qt5::Network
	${CMAKE_DL_LIBS}
	${TORA_LOKI_LIB}	
	${TORA_QSCINTILLA_LIB}
	${QSCINTILLA_LIBRARIES}
)
IF(PCH_DEFINED)
  ADD_PRECOMPILED_HEADER("test07" ${PCH_HEADER} FORCEINCLUDE)
ENDIF(PCH_DEFINED)
SET_TARGET_PROPERTIES("test07" PROPERTIES ENABLE_EXPORTS ON)
ENDIF(TORA_DEBUG AND TEST_APP07)

IF(TORA_DEBUG AND TEST_APP08)
# test8
QT5_WRAP_CPP(TEST8_MOC_SOURCES tests/test8.h)
ADD_EXECUTABLE("test08"
  tests/test8.cpp
  ${TEST8_MOC_SOURCES}
  ${PCH_SOURCE}
  #${CORE_SOURCES}
  #${LOGGING_SOURCES}  
  )
TARGET_LINK_LIBRARIES("test08"
	Qt5::Core
	Qt5::Widgets
	Qt5::Gui
	Qt5::Network
	${CMAKE_DL_LIBS}
)
IF(PCH_DEFINED)
  ADD_PRECOMPILED_HEADER("test08" ${PCH_HEADER} FORCEINCLUDE)
ENDIF(PCH_DEFINED)
SET_TARGET_PROPERTIES("test08" PROPERTIES ENABLE_EXPORTS ON)
ENDIF(TORA_DEBUG AND TEST_APP08)

IF(TORA_DEBUG AND TEST_APP09)
# test9
QT5_WRAP_CPP(TEST9_MOC_SOURCES
  tests/test9window.h  
  editor/tomemoeditor.h
  editor/toscintilla.h
  editor/tosqltext.h
  editor/todifftext.h  
  editor/tosyntaxanalyzernl.h
  editor/tosyntaxanalyzermysql.h
  editor/tosyntaxanalyzeroracle.h
  editor/tosyntaxanalyzerpostgresql.h
  tools/toresultstats.h
  tools/toresultview.h
  )
QT5_WRAP_UI(TEST9_UI_SOURCES tests/test9ui.ui)
QT5_ADD_RESOURCES(TEST9_RC_SOURCES tests/test9.qrc)
ADD_EXECUTABLE("test09"
  tests/test9.cpp
  tests/test9window.cpp
  editor/tomemoeditor.cpp
  editor/toscintilla.cpp
  editor/tosqltext.cpp
  editor/todifftext.cpp
  editor/tosyntaxanalyzernl.cpp
  editor/tosyntaxanalyzermysql.cpp
  editor/tosyntaxanalyzeroracle.cpp
  editor/tosyntaxanalyzerpostgresql.cpp
  tools/toresultstats.cpp
  tools/toresultview.cpp
  ${TEST9_MOC_SOURCES}
  ${TEST9_UI_SOURCES}
  ${TEST9_RC_SOURCES}
  ${PCH_SOURCE}  
  ${CORE_SOURCES}    
  ${PARSING_SOURCES} 
  ${WIDGETS_SOURCES} 
  ${LOGGING_SOURCES}
)
TARGET_LINK_LIBRARIES("test09"
	Qt5::Core
	Qt5::Widgets
	Qt5::Gui
	Qt5::Network
	Qt5::PrintSupport
	${CMAKE_DL_LIBS}
	${TORA_QSCINTILLA_LIB}
	${QSCINTILLA_LIBRARIES}	
	${TORA_LOKI_LIB}
)
IF(PCH_DEFINED)
  ADD_PRECOMPILED_HEADER("test09" ${PCH_HEADER} FORCEINCLUDE)
ENDIF(PCH_DEFINED)
SET_TARGET_PROPERTIES("test09" PROPERTIES ENABLE_EXPORTS ON)
ENDIF(TORA_DEBUG AND TEST_APP09)

IF(TORA_DEBUG AND TEST_APP10)
# test10
ADD_EXECUTABLE("test10"
  tests/test10.cpp
  ${PCH_SOURCE}
  ${CORE_SOURCES}
  ${PARSING_SOURCES}
  ${WIDGETS_SOURCES}
  ${VIEWS_SOURCES}
  ${RESULT_SOURCES}
  ${EDITOR_SOURCES}
  ${LOGGING_SOURCES}
  ${TOOLS_SOURCES}
  ${ORACLE_SOURCES}
)
TARGET_LINK_LIBRARIES("test10"
	Qt5::Core
	Qt5::Widgets
	Qt5::Gui
	Qt5::Network
	Qt5::PrintSupport
	${CMAKE_DL_LIBS}
	${TORA_QSCINTILLA_LIB}
	${QSCINTILLA_LIBRARIES}	
	${TORA_LOKI_LIB}
	ermodel
)
IF(PCH_DEFINED)
  ADD_PRECOMPILED_HEADER("test10" ${PCH_HEADER} FORCEINCLUDE)
ENDIF(PCH_DEFINED)
SET_TARGET_PROPERTIES("test10" PROPERTIES ENABLE_EXPORTS ON)
ENDIF(TORA_DEBUG AND TEST_APP10)

IF(TORA_DEBUG AND TEST_APP11)
# test11
ADD_EXECUTABLE("test11"
  tests/test11.cpp
  ${PCH_SOURCE}
  ${CORE_SOURCES}
  ${WIDGETS_SOURCES}
  ${PARSING_SOURCES}
  ${LOGGING_SOURCES}
  )
TARGET_LINK_LIBRARIES("test11"
	Qt5::Core
	Qt5::Widgets
	Qt5::Gui
	Qt5::Network
	${CMAKE_DL_LIBS}
	${TORA_LOKI_LIB}
	${TORA_QSCINTILLA_LIB}
	${QSCINTILLA_LIBRARIES}
	ermodel
)
IF(PCH_DEFINED)
  ADD_PRECOMPILED_HEADER("test11" ${PCH_HEADER} FORCEINCLUDE)
ENDIF(PCH_DEFINED)
SET_TARGET_PROPERTIES("test11" PROPERTIES ENABLE_EXPORTS ON)
ENDIF(TORA_DEBUG AND TEST_APP11)

IF(TORA_DEBUG AND TEST_APP12)
# test12
ADD_EXECUTABLE("test12"
  tests/test12.cpp
  ${PCH_SOURCE}
  ${CORE_SOURCES}
  ${WIDGETS_SOURCES}
  ${PARSING_SOURCES}
  ${LOGGING_SOURCES}
  )
TARGET_LINK_LIBRARIES("test12"
	Qt5::Core
	Qt5::Widgets
	Qt5::Gui
	Qt5::Network
	${CMAKE_DL_LIBS}
	${TORA_LOKI_LIB}
	${TORA_QSCINTILLA_LIB}
	${QSCINTILLA_LIBRARIES}
)
IF(PCH_DEFINED)
  ADD_PRECOMPILED_HEADER("test12" ${PCH_HEADER} FORCEINCLUDE)
ENDIF(PCH_DEFINED)
SET_TARGET_PROPERTIES("test12" PROPERTIES ENABLE_EXPORTS ON)
ENDIF(TORA_DEBUG AND TEST_APP12)

IF(TORA_DEBUG AND TEST_APP13)
# test13
QT5_ADD_RESOURCES(TEST13_RC_SOURCES tests/test13.qrc)
QT5_WRAP_UI(TEST13_UI_SOURCES tests/test13ui.ui)
QT5_WRAP_CPP(TEST13_MOC_SOURCES
  widgets/tosearch.h
  tests/test13window.h  
  )
ADD_EXECUTABLE("test13"
  tests/test13.cpp
  tests/test13window.cpp
  widgets/tosearch.cpp
  docklets/toastwalk.cpp
  ${TEST13_MOC_SOURCES}
  ${TEST13_UI_SOURCES}
  ${TEST13_RC_SOURCES}
  ${PCH_SOURCE}
  ${CORE_SOURCES}  
  ${WIDGETS_SOURCES}
  ${EDITOR_SOURCES}  
  ${PARSING_SOURCES}
  ${LOGGING_SOURCES}
  )
TARGET_LINK_LIBRARIES("test13"
	Qt5::Core
	Qt5::Widgets
	Qt5::Gui
	Qt5::Network
	${CMAKE_DL_LIBS}
	${TORA_LOKI_LIB}
	${TORA_QSCINTILLA_LIB}
	${QSCINTILLA_LIBRARIES}
	ermodel
)
IF(PCH_DEFINED)
  ADD_PRECOMPILED_HEADER("test13" ${PCH_HEADER} FORCEINCLUDE)
ENDIF(PCH_DEFINED)
SET_TARGET_PROPERTIES("test13" PROPERTIES ENABLE_EXPORTS ON)
ENDIF(TORA_DEBUG AND TEST_APP13)

IF(TORA_DEBUG AND TEST_APP14)
# test14
QT5_WRAP_UI(TEST14_UI_SOURCES tests/test14ui.ui)
QT5_WRAP_CPP(TEST14_MOC_SOURCES
  widgets/tosearch.h
  tests/test14window.h  
  )
ADD_EXECUTABLE("test14"
  tests/test14.cpp
  tests/test14window.cpp
  ${TEST14_MOC_SOURCES}
  ${TEST14_UI_SOURCES}
  ${TEST14_RC_SOURCES}
  ${PCH_SOURCE}
  ${CORE_SOURCES}  
  ${WIDGETS_SOURCES}
  ${EDITOR_SOURCES}  
  ${PARSING_SOURCES}
  ${LOGGING_SOURCES}
  )
TARGET_LINK_LIBRARIES("test14"
	Qt5::Core
	Qt5::Widgets
	Qt5::Gui
	Qt5::Network
	${CMAKE_DL_LIBS}
	${TORA_LOKI_LIB}
	${TORA_QSCINTILLA_LIB}
	${QSCINTILLA_LIBRARIES}
	ermodel
)
IF(PCH_DEFINED)
  ADD_PRECOMPILED_HEADER("test14" ${PCH_HEADER} FORCEINCLUDE)
ENDIF(PCH_DEFINED)
SET_TARGET_PROPERTIES("test14" PROPERTIES ENABLE_EXPORTS ON)
ENDIF(TORA_DEBUG AND TEST_APP14)

IF(TORA_DEBUG AND TEST_APP15)
# test15
ADD_EXECUTABLE("test15" ${GUI_TYPE}
  tests/test15.cpp
  ${PCH_SOURCE}  
  ${CORE_SOURCES}        #$<TARGET_OBJECTS:CORE_OBJECT_LIB>
  ${PARSING_SOURCES}     #$<TARGET_OBJECTS:PARSING_OBJECT_LIB>
  ${WIDGETS_SOURCES}     #$<TARGET_OBJECTS:WIDGETS_OBJECT_LIB>
  ${LOGGING_SOURCES}
  )
TARGET_LINK_LIBRARIES("test15"
	Qt5::Core
	Qt5::Widgets
	Qt5::Gui
	Qt5::Network
	${CMAKE_DL_LIBS}	
	${TORA_LOKI_LIB}
	${TORA_QSCINTILLA_LIB}
	${QSCINTILLA_LIBRARIES}
)
SET_TARGET_PROPERTIES("test15" PROPERTIES ENABLE_EXPORTS ON)
IF(PCH_DEFINED)
  ADD_PRECOMPILED_HEADER("test15" ${PCH_HEADER} FORCEINCLUDE)
ENDIF(PCH_DEFINED)
ENDIF(TORA_DEBUG AND TEST_APP15)

IF(TORA_DEBUG AND TEST_APP16)
# test16
ADD_EXECUTABLE("test16" ${GUI_TYPE}
  tests/test16.cpp
  ${PCH_SOURCE}  
  ${CORE_SOURCES}        #$<TARGET_OBJECTS:CORE_OBJECT_LIB>
  ${PARSING_SOURCES}     #$<TARGET_OBJECTS:PARSING_OBJECT_LIB>
  ${WIDGETS_SOURCES}     #$<TARGET_OBJECTS:WIDGETS_OBJECT_LIB>
  ${LOGGING_SOURCES}
  )
TARGET_LINK_LIBRARIES("test16"
	Qt5::Core
	Qt5::Widgets
	Qt5::Gui
	Qt5::Network
	${CMAKE_DL_LIBS}	
	${TORA_LOKI_LIB}
	${TORA_QSCINTILLA_LIB}
	${QSCINTILLA_LIBRARIES}
)
SET_TARGET_PROPERTIES("test16" PROPERTIES ENABLE_EXPORTS ON)
IF(PCH_DEFINED)
  ADD_PRECOMPILED_HEADER("test16" ${PCH_HEADER} FORCEINCLUDE)
ENDIF(PCH_DEFINED)
ENDIF(TORA_DEBUG AND TEST_APP16)
