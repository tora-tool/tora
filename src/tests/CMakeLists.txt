

#FOREACH(item ${TORA_MOC_SOURCES})
#STRING(REGEX MATCH ".*/core/moc_.*c..$" item ${item})
#IF(item)
# LIST(APPEND CORE_OBJECT_SOURCES ${item})
#ENDIF(item)
#ENDFOREACH(item ${TORA_MOC_SOURCES})

IF(TORA_DEBUG AND TEST_APP1)
# test1
SET(TEST1_SOURCES

  )
SET(TEST1_MOC_SOURCES
  #../parsing/moc_tsqllexer.cxx
  )
ADD_EXECUTABLE("test1" ${GUI_TYPE}
  tests/test1.cpp
  #${TEST1_MOC_SOURCES}
  #${TEST1_SOURCES}
  #${TORA_RCC_SRCS}
  ${PCH_SOURCE}  
  $<TARGET_OBJECTS:WIDGETS_OBJECT_LIB>
  $<TARGET_OBJECTS:CORE_OBJECT_LIB>
  $<TARGET_OBJECTS:PARSING_OBJECT_LIB>
  )
TARGET_LINK_LIBRARIES("test1"
)
SET_TARGET_PROPERTIES("test1" PROPERTIES ENABLE_EXPORTS ON)
ADD_DEPENDENCIES("test1" revisiontag) # explicitly say that the executable depends on the svnheader
IF(PCH_DEFINED)
  ADD_PRECOMPILED_HEADER("test1" ${PCH_HEADER} FORCEINCLUDE)
ENDIF(PCH_DEFINED)
ENDIF(TORA_DEBUG AND TEST_APP1)

IF(FALSE)

IF(TORA_DEBUG AND TEST_APP2)  
# test2
ADD_EXECUTABLE("test2" ${GUI_TYPE}
  tests/test2.cpp
  ${TORA_UI_SOURCES}  
  ${TORA_I18N_QM}
  ${TORA_MOC_SOURCES}
  ${TORA_SOURCES}
  ${TORA_RCC_SRCS}
  )
TARGET_LINK_LIBRARIES("test2" ${TORA_LIBS})
SET_TARGET_PROPERTIES("test2" PROPERTIES ENABLE_EXPORTS ON)
ADD_DEPENDENCIES("test2" revisiontag) # explicitly say that the executable depends on the svnheader
IF(PCH_DEFINED)
  ADD_PRECOMPILED_HEADER("test2" ${PCH_HEADER} FORCEINCLUDE)
ENDIF(PCH_DEFINED)
ENDIF(TORA_DEBUG AND TEST_APP2)

IF(TORA_DEBUG AND TEST_APP3)
# test3
SET(TEST3_MOC_CLASSES  tests/test3window.h)
QT4_WRAP_CPP(TEST3_MOC_SOURCES ${TEST3_MOC_CLASSES})

ADD_EXECUTABLE("test3" ${GUI_TYPE}
  tests/test3.cpp
  tests/test3window.cpp
  ${TEST3_MOC_SOURCES}
  ${TORA_UI_SOURCES}  
  ${TORA_I18N_QM}
  ${TORA_MOC_SOURCES}
  ${TORA_SOURCES}
  ${TORA_RCC_SRCS}
  )
TARGET_LINK_LIBRARIES("test3" ${TORA_LIBS})
SET_TARGET_PROPERTIES("test3" PROPERTIES ENABLE_EXPORTS ON)
ADD_DEPENDENCIES("test3" revisiontag) # explicitly say that the executable depends on the svnheader
IF(PCH_DEFINED)
  ADD_PRECOMPILED_HEADER("test3" ${PCH_HEADER} FORCEINCLUDE)
ENDIF(PCH_DEFINED)
ENDIF(TORA_DEBUG AND TEST_APP3)

IF(TORA_DEBUG AND TEST_APP4)
MESSAGE(WARNING "MOC ${TORA_MOC_SOURCES}")
# test4
SET(TEST4_SOURCES
  tests/test4.cpp
  tests/test4window.cpp
  ${CORE_OBJECT_SOURCES}
  widgets/moc_tobackgroundlabel.cxx
  widgets/moc_toconnectionwidget.cxx
  widgets/moc_tohelpsetup.cxx
  widgets/moc_topushbutton.cxx
  widgets/moc_totabwidget.cxx
  widgets/moc_totoolwidget.cxx
  widgets/moc_totreewidget.cxx
  widgets/moc_toworkspace.cxx
  widgets/tobackgroundlabel.cpp
  widgets/toconnectionwidget.cpp
  widgets/tohelpsetup.cpp
  widgets/topushbutton.cpp
  widgets/totabwidget.cpp
  widgets/totoolwidget.cpp
  widgets/totreewidget.cpp
  widgets/toworkspace.cpp
  
  editor/moc_tobaseeditor.cxx
  editor/moc_tomarkededitor.cxx
  editor/moc_toscintilla.cxx
  editor/moc_tosearchreplace.cxx  
  editor/moc_tosqltext.cxx
  editor/moc_tostyle.cxx
  editor/tobaseeditor.cpp
  editor/tomarkededitor.cpp
  editor/toscintilla.cpp
  editor/tosearchreplace.cpp  
  editor/tosqltext.cpp
  editor/tostyle.cpp

  parsing/MySQLGuiLexer.cc
  parsing/PLSQLGuiLexer.cc
  parsing/moc_tolexeroracle.cxx
  parsing/moc_tolexeroracleapis.cxx
  parsing/moc_tosyntaxanalyzer.cxx  
  parsing/moc_tsqllexer.cxx
  parsing/tolexeroracle.cpp
  parsing/tolexeroracleapis.cpp
  parsing/tosyntaxanalyzer.cpp
  parsing/tosyntaxanalyzermysql.cpp
  parsing/tosyntaxanalyzernl.cpp
  parsing/tosyntaxanalyzeroracle.cpp
  parsing/tsqllexermysql.cc
  parsing/tsqllexeroracle2.cc
  parsing/persistenttrie.cpp
  # parsing/PLSQLLexer.cc
  # parsing/PLSQLParser.cc
  # parsing/PLSQLParser_PLSQL_DMLParser.cc
  # parsing/PLSQLParser_PLSQLCommons.cc
  # parsing/PLSQLParser_PLSQLKeys.cc
  # parsing/PLSQLParser_SQLPLUSParser.cc  
  ts_log/moc_toostream.cxx

  connection/tooraclesetting.cpp
  connection/moc_tooraclesetting.cxx

  tools/tohelp.cpp
  tools/moc_tohelp.cxx
  )
SET(TEST4_MOC_CLASSES  tests/test4window.h)

IF (NOT QT5_BUILD)
  QT4_WRAP_UI(TEST4_UI_SOURCES tests/test4ui.ui)
  QT4_WRAP_CPP(TEST4_MOC_SOURCES ${TEST4_MOC_CLASSES})
ENDIF()

ADD_EXECUTABLE("test4" ${GUI_TYPE}
  # ${TORA_UI_SOURCES}
  ${TEST4_UI_SOURCES}
  ${TEST4_MOC_SOURCES}
  ${TEST4_SOURCES}
  ${TORA_RCC_SRCS}
  ${PCH_SOURCE}
  )
TARGET_LINK_LIBRARIES("test4" ${TORA_LIBS})
SET_TARGET_PROPERTIES("test4" PROPERTIES ENABLE_EXPORTS ON)
ADD_DEPENDENCIES("test4" revisiontag) # explicitly say that the executable depends on the svnheader
IF(PCH_DEFINED)
  ADD_PRECOMPILED_HEADER("test4" ${PCH_HEADER} FORCEINCLUDE)
ENDIF(PCH_DEFINED)
ENDIF(TORA_DEBUG AND TEST_APP4)

IF(TORA_DEBUG AND TEST_APP5)
# test5
SET(TEST5_SOURCES
  ${CORE_OBJECT_SOURCES}

  widgets/tobackgroundlabel.cpp
  widgets/moc_tobackgroundlabel.cxx
  widgets/tobrowserbasewidget.cpp
  widgets/moc_tobrowserbasewidget.cxx
  widgets/todescribe.cpp
  widgets/moc_todescribe.cxx
  widgets/todockbar.cpp
  widgets/moc_todockbar.cxx
  widgets/tohelpsetup.cpp
  widgets/moc_tohelpsetup.cxx
  widgets/toparamget.cpp
  widgets/moc_toparamget.cxx
  widgets/topushbutton.cpp
  widgets/moc_topushbutton.cxx
  widgets/toresultcols.cpp
  widgets/moc_toresultcols.cxx
  widgets/toresultcolscomment.cpp
  widgets/moc_toresultcolscomment.cxx
  widgets/toresultcombo.cpp
  widgets/moc_toresultcombo.cxx
  widgets/toresultdatasingle.cpp
  widgets/moc_toresultdatasingle.cxx
  widgets/toresultitem.cpp
  widgets/moc_toresultitem.cxx
  widgets/toresultlistformat.cpp
  widgets/moc_toresultlistformat.cxx
  widgets/toresultmodel.cpp
  widgets/moc_toresultmodel.cxx
  widgets/toresultmodeledit.cpp
  widgets/moc_toresultmodeledit.cxx
  widgets/toresultschema.cpp
  widgets/moc_toresultschema.cxx
  widgets/toresulttableview.cpp
  widgets/moc_toresulttableview.cxx
  widgets/toresulttableviewedit.cpp
  widgets/moc_toresulttableviewedit.cxx
  widgets/toresultview.cpp
  widgets/moc_toresultview.cxx
  widgets/tosplash.cpp
  widgets/moc_tosplash.cxx
  widgets/totabwidget.cpp
  widgets/moc_totabwidget.cxx
  widgets/totreewidget.cpp
  widgets/moc_totreewidget.cxx
  widgets/toworkingwidget.cpp
  widgets/moc_toworkingwidget.cxx
  widgets/toworkspace.cpp
  widgets/moc_toworkspace.cxx
  widgets/totoolwidget.cpp
  widgets/moc_totoolwidget.cxx
  widgets/toconnectionwidget.cpp
  widgets/moc_toconnectionwidget.cxx
  
  editor/moc_tobaseeditor.cxx
  editor/moc_todebugeditor.cxx
  editor/moc_todebugtext.cxx  
  editor/moc_tohighlightededitor.cxx
  editor/moc_tomarkededitor.cxx
  editor/moc_tomemoeditor.cxx
  editor/moc_tomodeleditor.cxx
  editor/moc_toscintilla.cxx
  editor/moc_tosearchreplace.cxx
  editor/moc_tosqltext.cxx
  editor/moc_tostyle.cxx
  editor/tobaseeditor.cpp
  editor/todebugeditor.cpp
  editor/todebugeditor.cpp
  editor/todebugtext.cpp
  editor/tohighlightededitor.cpp
  editor/tomarkededitor.cpp
  editor/tomemoeditor.cpp
  editor/tomodeleditor.cpp
  editor/toscintilla.cpp
  editor/tosearchreplace.cpp
  editor/tosqltext.cpp
  editor/tostyle.cpp

  parsing/moc_tolexeroracle.cxx
  parsing/moc_tolexeroracleapis.cxx
  parsing/moc_tosyntaxanalyzer.cxx
  parsing/moc_tsqllexer.cxx
  parsing/persistenttrie.cpp
  parsing/tolexeroracle.cpp
  parsing/tolexeroracleapis.cpp
  parsing/tosyntaxanalyzer.cpp
  parsing/tosyntaxanalyzermysql.cpp
  parsing/tosyntaxanalyzernl.cpp
  parsing/tosyntaxanalyzeroracle.cpp
  result/moc_toresulttabledata.cxx
  result/toresulttabledata.cpp
  tests/moc_test5window.cxx

  tools/moc_tohelp.cxx
  tools/moc_toplsqleditor.cxx
  tools/moc_toplsqltext.cxx
  tools/tohelp.cpp
  tools/toplsqleditor.cpp
  tools/toplsqltext.cpp

  connection/tooraclefind.cpp
  connection/tooraclesetting.cpp
  connection/moc_tooraclesetting.cxx
  
  ts_log/moc_toostream.cxx
  )

SET(TEST5_MOC_CLASSES  tests/test5window.h)
QT4_WRAP_CPP(TEST5_MOC_SOURCES ${TEST5_MOC_CLASSES})

ADD_EXECUTABLE("test5" ${GUI_TYPE}
  tests/test5.cpp
  tests/test5window.cpp
  parsing/PLSQLGuiLexer.cc
  parsing/tsqllexeroracle2.cc  
  parsing/MySQLGuiLexer.cc
  parsing/tsqllexermysql.cc
  # parsing/PLSQLLexer.cc
  # parsing/PLSQLParser.cc
  # parsing/PLSQLParser_PLSQL_DMLParser.cc
  # parsing/PLSQLParser_PLSQLCommons.cc
  # parsing/PLSQLParser_PLSQLKeys.cc
  # parsing/PLSQLParser_SQLPLUSParser.cc
  ${TORA_UI_SOURCES}  
  ${TEST5_MOC_SOURCES}
  ${TEST5_SOURCES}
  ${TORA_RCC_SRCS}
  ${PCH_SOURCE}
  )
TARGET_LINK_LIBRARIES("test5" ${TORA_LIBS})
SET_TARGET_PROPERTIES("test5" PROPERTIES ENABLE_EXPORTS ON)
ADD_DEPENDENCIES("test5" revisiontag) # explicitly say that the executable depends on the svnheader
IF(PCH_DEFINED)
  ADD_PRECOMPILED_HEADER("test5" ${PCH_HEADER} FORCEINCLUDE)
ENDIF(PCH_DEFINED)
ENDIF(TORA_DEBUG AND TEST_APP5)

IF(TORA_DEBUG AND TEST_APP6)
# test6
ADD_EXECUTABLE("test6" ${GUI_TYPE}
  tests/test6.cpp
  ${TEST6_SOURCES}
  ${PCH_SOURCE}
  )
TARGET_LINK_LIBRARIES("test6" ermodel)
SET_TARGET_PROPERTIES("test6" PROPERTIES ENABLE_EXPORTS ON)
ADD_DEPENDENCIES("test6" revisiontag) # explicitly say that the executable depends on the svnheader
IF(PCH_DEFINED)
  ADD_PRECOMPILED_HEADER("test6" ${PCH_HEADER} FORCEINCLUDE)
ENDIF(PCH_DEFINED)
ENDIF(TORA_DEBUG AND TEST_APP6)

IF(TORA_DEBUG AND TEST_APP7)
# test7
ADD_EXECUTABLE("test7" ${GUI_TYPE}
  tests/test7.cpp
  parsing/tsqlparse.cpp
  parsing/OracleDML.cc
  parsing/OracleDML.hpp
  parsing/OracleDML_OracleDMLCommons.cc
  parsing/OracleDML_OracleDMLCommons.hpp
  parsing/OracleDML_OracleDMLKeys.cc
  parsing/OracleDML_OracleDMLKeys.hpp
  parsing/OracleDMLLexer.cc
  parsing/OracleDMLLexer.hpp
  parsing/OracleDMLTraits.hpp
  parsing/tsqlparseoracle2.cc
  parsing/OracleSQLLexer.cc
  parsing/OracleSQLParser.cc
  parsing/tsqlparseoracle.cc
  ${PCH_SOURCE}
  )
TARGET_LINK_LIBRARIES("test7" ${TORA_LIBS})
SET_TARGET_PROPERTIES("test7" PROPERTIES ENABLE_EXPORTS ON)
ADD_DEPENDENCIES("test7" revisiontag)
IF(PCH_DEFINED)
  ADD_PRECOMPILED_HEADER("test7" ${PCH_HEADER} FORCEINCLUDE)
ENDIF(PCH_DEFINED)
ENDIF(TORA_DEBUG AND TEST_APP7)

ENDIF()

