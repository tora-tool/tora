# Precompiled headers
IF(USE_PCH AND WIN32 AND MSVC)
  INCLUDE(PrecompiledHeader)
  SET(PCH_DEFINED TRUE)
  SET(PCH_HEADER "precompiled_msvc.h")
  SET(PCH_SOURCE "precompiled/precompiled_msvc.cpp")
ELSEIF (USE_PCH AND CMAKE_COMPILER_IS_GNUCXX AND ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  INCLUDE(PrecompiledHeader)
  SET(PCH_DEFINED TRUE)
  SET(PCH_HEADER "precompiled/precompiled_linux.h")
ENDIF()

################################################################################
#                              Subdirectories                                  #
################################################################################

# ADD_SUBDIRECTORY(i18n)

################################################################################
#                                 DEFINEs                                      #
################################################################################
IF(USE_EXPERIMENTAL)
  ADD_DEFINITIONS("-DTORA_EXPERIMENTAL")	
ENDIF(USE_EXPERIMENTAL)

################################################################################
#                                 Headers                                      #
################################################################################
INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/precompiled
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/extlibs/trotl/src
  ${CMAKE_SOURCE_DIR}/extlibs/libermodel
  ${CMAKE_SOURCE_DIR}/extlibs/libantlr3cpp-3.5.3/include
  ${QT_INCLUDES}  
  )

IF (Boost_FOUND)
  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
ENDIF()

IF (ORACLE_INCLUDES)
  INCLUDE_DIRECTORIES( ${ORACLE_INCLUDES} )
ENDIF (ORACLE_INCLUDES)

IF (POSTGRESQL_INCLUDE_DIR)
  INCLUDE_DIRECTORIES( ${POSTGRESQL_INCLUDE_DIR} )
ENDIF (POSTGRESQL_INCLUDE_DIR)

IF (DB2_INCLUDES)
  INCLUDE_DIRECTORIES( ${DB2_INCLUDES} )
ENDIF (DB2_INCLUDES)

IF (TERADATA_INCLUDES)
  INCLUDE_DIRECTORIES( ${TERADATA_INCLUDES} )
ENDIF (TERADATA_INCLUDES)

IF (NOT WANT_INTERNAL_QSCINTILLA)
  INCLUDE_DIRECTORIES(${QSCINTILLA_INCLUDE_DIR})
ENDIF (NOT WANT_INTERNAL_QSCINTILLA)

IF (USE_EXPERIMENTAL)
  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/extlibs/libantlr3c-3.5.1/include)
  INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/extlibs/libantlr3c-3.5.1)

  IF (POPPLER_QT4_FOUND)
    INCLUDE_DIRECTORIES(${POPPLER_QT4_INCLUDE_DIR})
  ENDIF (POPPLER_QT4_FOUND)
ENDIF(USE_EXPERIMENTAL)

################################################################################
#                              Resource files                                  #
################################################################################
# only toad bindings are here. All others qm files will blow the binary too much
SET (TORA_I18N_TS
  #i18n/tora_fr.ts
  #i18n/tora_it.ts
  #i18n/tora_se.ts
  #i18n/tora_es.ts
  )
SET (TORA_I18N_TS_TOAD i18n/tora_toad.ts)

SET(TORA_RCC_FILES tora.qrc icons.qrc)

IF(TORA_DEBUG)  
  LIST (APPEND TORA_RCC_FILES tests/test4.qrc)
ENDIF()

################################################################################
#                              User interface files                            #
################################################################################
SET(TORA_UI_SRC
  widgets/connectioncolorsdialogui.ui
  widgets/toaboutui.ui
  widgets/todatabasesettingui.ui
  widgets/todescribeui.ui
  widgets/toglobalsettingui.ui
  widgets/tohelpaddfileui.ui
  widgets/tohelpsetupui.ui
  widgets/tohelpui.ui
  widgets/tolinechartsetupui.ui	
  widgets/tomessageui.ui
  widgets/tonewconnectionui.ui
  widgets/topreferencesui.ui
  widgets/toresultcontentfilterui.ui
  widgets/toresultlistformatui.ui
  widgets/tosyntaxsetupui.ui
  widgets/toworksheetsetupui.ui
  editor/tosearchreplaceui.ui
  widgets/toconnectionimportui.ui
  widgets/totoolsettingui.ui

  #obsolete tools/tobrowserconstraintui.ui
  tools/tobrowserfilterui.ui
  #obsolete tools/tobrowserindexui.ui
  #obsolete tools/tobrowsertableui.ui

  tools/tostoragedatafileui.ui
  tools/tostoragedefinitionui.ui
  tools/tostoragedialogui.ui
  tools/tostorageprefsui.ui
  tools/tostoragetablespaceui.ui
  tools/todroptablespaceui.ui

  tools/tosecurityquotaui.ui
  tools/tosecurityroleui.ui
  tools/tosecurityuserui.ui

  tools/toscriptui.ui
  tools/toscriptschemawidgetui.ui

  tools/torollbackdialogui.ui
  tools/torollbacksettingui.ui

  tools/tosgatracesettingui.ui

  #tools/totuningoverviewui.ui
  #tools/totuningsettingui.ui

  tools/tooutputsettingui.ui

  shortcuteditor/shortcuteditordialog.ui
  
  connection/toqmysqlsettingui.ui  
  connection/toqpsqlsettingui.ui 
  
  result/toresulttabledataui.ui

  #     todebugchangeui.ui
  #     todebugwatch.ui

  #     toeditextensionsetupui.ui

  #     tochartalarmui.ui
  #     tochartsetupui.ui


  # #    toschedulerui.ui

  templates/totemplateaddfileui.ui
  templates/totemplateeditui.ui
  templates/totemplatesetupui.ui
  )

IF (ORACLE_FOUND)
  LIST (APPEND TORA_UI_SRC connection/tooraclesettingui.ui)
ENDIF (ORACLE_FOUND)

IF (POPPLER_QT4_FOUND)
  #    LIST(APPEND TORA_UI_SRC pdfview/pdfviewwidget.ui)
ENDIF (POPPLER_QT4_FOUND)

################################################################################
#                           QObject sub-classes(moc)                           #
################################################################################
# MOC classses
SET(TORA_MOC_CLASSES
  # Tora core sorces
  #obsolete core/todatatype.h
  #obsolete core/torunnable.h
  #obsolete core/totaskrunner.h
  #obsolete core/tothread.h
  #obsolete core/tovisualize.h
  #core/toconfiguration.h
  #core/toconfigurationpriv.h
  core/tobackground.h
  core/tocache.h
  core/tochangeconnection.h
  core/tocodemodel.h
  core/toconfenum.h
  core/toconfiguration.h
  core/toconnection.h
  core/toconnectionmodel.h
  core/toconnectionregistry.h
  core/todatabasesetting.h
  widgets/todescribe.h
  widgets/todockbar.h
  core/todocklet.h
  core/toeditablemenu.h
  core/toeditmenu.h
  core/toeditorsetting.h
  core/toeventquery.h
  core/toeventqueryworker.h
  core/toglobalevent.h
  widgets/toglobalsetting.h
  core/toglobalconfiguration.h
  tools/tohelp.h
  widgets/tohelpsetup.h
  core/tolistviewformatter.h
  main/tomain.h
  core/tomainwindow.h
  editor/tomemoeditor.h
  core/tomessage.h
  editor/tomodeleditor.h
  widgets/tonewconnection.h  
  widgets/toparamget.h
  widgets/topreferences.h
  core/toquery.h
  core/toqueryimpl.h
  core/toresult.h
  widgets/toabout.h
  widgets/tobackgroundlabel.h
  widgets/tobarchart.h
  widgets/tobrowserbasewidget.h # this one is used by widgets/todescribe.h (therefore is not in tools)
  widgets/toconnectionimport.h
  widgets/tolinechart.h
  widgets/topiechart.h
  widgets/topushbutton.h     
  widgets/torefreshcombo.h
  widgets/toresultbar.h
  widgets/toresultcode.h
  widgets/toresultcols.h
  widgets/toresultcolscomment.h
  widgets/toresultcombo.h
  widgets/toresultdatasingle.h
  widgets/toresultdepend.h
  widgets/toresultdrawing.h
  widgets/toresultextent.h
  widgets/toresultfield.h
  widgets/toresultgrants.h
  widgets/toresultitem.h
  widgets/toresultline.h
  widgets/toresultlistformat.h
  widgets/toresultlock.h
  widgets/toresultlong.h
  widgets/toresultmodel.h
  widgets/toresultmodeledit.h
  widgets/toresultparam.h
  widgets/toresultplan.h
  widgets/toresultschema.h
  widgets/toresultstats.h
  widgets/toresulttableview.h  
  widgets/toresulttableviewedit.h  
  widgets/toresultview.h
  widgets/tosplash.h
  widgets/tosyntaxsetup.h
  widgets/totableselect.h  
  widgets/totabwidget.h
  widgets/totoolwidget.h
  core/totextview.h
  widgets/totreewidget.h
  widgets/toworkingwidget.h
  widgets/toworkspace.h

  core/tohelpcontext.h
  core/tosettingtab.h
  widgets/toconnectionwidget.h
  
  core/totool.h
  core/utils.h
  parsing/tsqllexer.h
  parsing/tosyntaxanalyzer.h
  parsing/tolexeroracleapis.h 
  
  editor/tobaseeditor.h
  editor/tocomplpopup.h
  editor/tomarkededitor.h
  editor/toscintilla.h
  editor/tohighlightededitor.h
  editor/tosqltext.h
  editor/toworksheettext.h
  editor/todebugtext.h
  editor/todebugeditor.h
  editor/tosearchreplace.h
  editor/tostyle.h
  parsing/tolexeroracle.h

  result/toproviderobserver.h
  result/totablemodel.h
  result/totableview.h
  result/totreemodel.h
  result/totreeview.h
  result/toresulttabledata.h

  ts_log/toostream.h

  connection/toqmysqlsetting.h
  connection/toqpsqlsetting.h

  shortcuteditor/shortcuteditordialog.h
  shortcuteditor/shortcutmodel.h

  docklets/toviewconnections.h
  docklets/toviewdirectory.h

  tools/tooutput.h
  
  tools/toworksheet.h
  tools/toworksheetstatistic.h
  tools/toworksheeteditor.h
  
  tools/tosgatrace.h
  tools/tosgastatement.h

  #obsolete tools/tobrowserconstraint.h
  tools/tobrowser.h
  #obsolete tools/tobrowserindex.h
  #obsolete tools/tobrowsertable.h
  tools/tobrowseraccesswidget.h
  tools/tobrowsertablewidget.h
  tools/tobrowserviewwidget.h
  tools/tobrowserindexwidget.h
  tools/tobrowserschemawidget.h
  tools/tobrowsersequencewidget.h
  tools/tobrowsersynonymwidget.h
  tools/tobrowsercodewidget.h
  tools/tobrowsertriggerwidget.h
  tools/tobrowserdblinkswidget.h
  tools/tobrowserdirectorieswidget.h

  tools/tostorage.h
  tools/toresultstorage.h
  tools/tofilesize.h
  tools/toanalyze.h

  tools/toawr.h

  tools/tosession.h  
  tools/towaitevents.h

  tools/toinvalid.h
  tools/tobackup.h
  tools/tobackuptool.h
  tools/tounittest.h

  tools/tosecurity.h
  tools/tosecuritytreemodel.h
  tools/tosecuritytreeitem.h

  tools/tocurrent.h 

  tools/toscriptschemawidget.h
  tools/toscripttreeitem.h
  tools/toscripttreemodel.h

  tools/toprofiler.h

  tools/totemporary.h

  tools/torollback.h

  tools/toplsqltext.h
  tools/toplsqleditor.h

  templates/tosqledit.h  
  templates/totemplate.h

  #     toproject.h
  
  #TODO tools/toalert.h
  #TODO tools/totuning.h
  #     todebug.h
  #     todebugtext.h
  #     toeditextensions.h
  # #   toreport.h
  #     toresultlabel.h
  # #   toresultresources.h
  # #   toscheduler.h
  # # tostoragedefinition.h
  )

IF (ORACLE_FOUND)
  LIST(APPEND TORA_MOC_CLASSES connection/tooraclesetting.h)
ENDIF (ORACLE_FOUND)

IF (POPPLER_QT4_FOUND)
  #    LIST(APPEND TORA_MOC_CLASSES pdfview/documentwidget.h pdfview/pdfviewwidget.h)
ENDIF (POPPLER_QT4_FOUND)

IF (USE_EXPERIMENTAL)
  LIST(APPEND TORA_MOC_CLASSES tools/toscript.h)
  LIST(APPEND TORA_MOC_CLASSES docklets/tocodeoutline.h)
  #IF(graphviz_FOUND)
  LIST(APPEND TORA_MOC_CLASSES docklets/toquerymodel.h)
  LIST(APPEND TORA_MOC_CLASSES tools/toer.h)
  LIST(APPEND TORA_MOC_CLASSES tools/tosandboxtool.h)
  #ENDIF(graphviz_FOUND)
ENDIF (USE_EXPERIMENTAL)

IF(TORA_DEBUG)
  LIST(APPEND TORA_MOC_CLASSES docklets/tologging.h)
ENDIF()

################################################################################
#                              Source files .cpp .rc                           #
################################################################################
# sources
SET(TORA_SOURCES
  #obsolete core/todatatype.cpp
  #obsolete core/totaskrunner.cpp
  #obsolete core/tovisualize.cpp
  core/tobackground.cpp
  core/tocache.cpp
  core/tochangeconnection.cpp 
  core/tocodemodel.cpp
  core/toconfenum.cpp
  widgets/toabout.cpp
  widgets/tobackgroundlabel.cpp     
  widgets/tobarchart.cpp
  widgets/tobrowserbasewidget.cpp # this one is used by widgets/todescribe.h (therefore is not in tools)
  #core/toconfiguration.cpp
  core/toconfiguration.cpp
  #core/toconfigurationpriv.cpp
  core/toconnection.cpp
  core/toconnectionmodel.cpp
  core/toconnectionoptions.cpp
  core/toconnectionprovider.cpp
  core/toconnectionregistry.cpp
  core/toconnectionsub.cpp
  core/toconnectionsubloan.cpp
  core/todatabasesetting.cpp
  widgets/todescribe.cpp
  core/todocklet.cpp
  core/toeditablemenu.cpp
  core/toeditmenu.cpp
  core/toeditorsetting.cpp
  core/toeditwidget.cpp
  core/toeventquery.cpp
  core/toeventqueryworker.cpp
  core/toextract.cpp  
  core/toglobalevent.cpp
  core/toglobalconfiguration.cpp
  core/tohelpcontext.cpp
  core/tohtml.cpp
  core/tolistviewformatter.cpp
  core/tolistviewformattercsv.cpp
  core/tolistviewformatterhtml.cpp
  core/tolistviewformattersql.cpp
  core/tolistviewformattertabdel.cpp
  core/tolistviewformattertext.cpp
  core/tomainwindow.cpp
  core/tomessage.cpp
  core/toquery.cpp
  core/toqvalue.cpp
  core/toresult.cpp
  core/tosettingtab.cpp
  core/tosql.cpp
  core/totextview.cpp
  core/totool.cpp
  main/tomain.cpp
  widgets/toconnectionimport.cpp
  widgets/toconnectionwidget.cpp
  widgets/todockbar.cpp
  widgets/toglobalsetting.cpp
  widgets/tohelpsetup.cpp
  widgets/tolinechart.cpp
  widgets/tonewconnection.cpp
  widgets/toparamget.cpp
  widgets/topiechart.cpp
  widgets/topreferences.cpp	
  widgets/topushbutton.cpp
  widgets/torefreshcombo.cpp
  widgets/toresultbar.cpp
  widgets/toresultcode.cpp
  widgets/toresultcols.cpp
  widgets/toresultcolscomment.cpp
  widgets/toresultcombo.cpp
  widgets/toresultdatasingle.cpp
  widgets/toresultdepend.cpp
  widgets/toresultdrawing.cpp
  widgets/toresultextent.cpp
  widgets/toresultfield.cpp
  widgets/toresultgrants.cpp
  widgets/toresultitem.cpp
  widgets/toresultline.cpp
  widgets/toresultlistformat.cpp
  widgets/toresultlock.cpp
  widgets/toresultlong.cpp
  widgets/toresultmodel.cpp
  widgets/toresultmodeledit.cpp
  widgets/toresultparam.cpp
  widgets/toresultplan.cpp
  widgets/toresultschema.cpp
  widgets/toresultstats.cpp
  widgets/toresulttableview.cpp
  widgets/toresulttableviewedit.cpp
  widgets/toresultview.cpp
  widgets/tosplash.cpp
  widgets/tosyntaxsetup.cpp
  widgets/totableselect.cpp  
  widgets/totabwidget.cpp
  widgets/totoolwidget.cpp
  widgets/totreewidget.cpp
  widgets/toworkingwidget.cpp
  widgets/toworkspace.cpp
  
  core/utils.cpp
  core/utils_part.cpp

  editor/tomemoeditor.cpp
  editor/tomodeleditor.cpp  
  editor/tobaseeditor.cpp
  editor/tocomplpopup.cpp
  editor/tomarkededitor.cpp
  editor/toscintilla.cpp
  editor/tohighlightededitor.cpp
  editor/tosqltext.cpp
  editor/toworksheettext.cpp
  editor/todebugtext.cpp
  editor/todebugeditor.cpp
  editor/tosearchreplace.cpp
  editor/tostyle.cpp
  parsing/tolexeroracle.cpp

  result/toproviderobserver.cpp
  result/totablemodel.cpp
  result/totableview.cpp
  result/totreemodel.cpp
  result/totreeview.cpp
  result/toresulttabledata.cpp

  migratetool/sqldeveloper.cpp
  migratetool/squirrelsql.cpp
  migratetool/tora3.cpp

  connection/tooraclesql.cpp
  connection/tooracleextract.cpp
  connection/toteradatafind.cpp
  connection/toqsqlfind.cpp
  connection/toqsqlconnection.cpp
  connection/toqsqlprovider.cpp
  connection/toqmysqlprovider.cpp
  connection/toqmysqlsetting.cpp
  connection/toqmysqltraits.cpp
  connection/toqpsqltraits.cpp
  connection/toqmysqlconnection.cpp
  connection/toqmysqlquery.cpp
  connection/toqpsqlprovider.cpp
  connection/toqpsqlconnection.cpp
  connection/toqpsqlquery.cpp
  connection/toqpsqlsetting.cpp
  connection/toqodbcprovider.cpp
  connection/toqsqlquery.cpp

  parsing/tsqlparse.cpp
  parsing/tosyntaxanalyzer.cpp
  parsing/tosyntaxanalyzernl.cpp
  parsing/tosyntaxanalyzermysql.cpp
  parsing/tosyntaxanalyzeroracle.cpp
  parsing/PLSQLGuiLexer.cc
  parsing/tsqllexeroracle2.cc
  parsing/MySQLGuiLexer.cc
  parsing/tsqllexermysql.cc
  parsing/MySQLLexer.cc
  parsing/tolexeroracleapis.cpp
  parsing/persistenttrie.cpp
  parsing/OracleDML.cc
  parsing/OracleDML.hpp
  parsing/OracleDML_OracleDMLCommons.cc
  parsing/OracleDML_OracleDMLCommons.hpp
  parsing/OracleDML_OracleDMLKeys.cc
  parsing/OracleDML_OracleDMLKeys.hpp
  parsing/OracleDMLLexer.cc
  parsing/OracleDMLLexer.hpp
  parsing/OracleDMLTraits.hpp

  docklets/toviewconnections.cpp
  docklets/toviewdirectory.cpp

  shortcuteditor/shortcutmodel.cpp
  shortcuteditor/shortcuteditordialog.cpp

  tools/tohelp.cpp  
  tools/tooutput.cpp

  tools/toworksheet.cpp
  tools/toworksheeteditor.cpp
  tools/toworksheetstatistic.cpp

  tools/tosgatrace.cpp
  tools/tosgastatement.cpp

  #obsolete tools/tobrowserconstraint.cpp
  tools/tobrowser.cpp
  #obsolete tools/tobrowserindex.cpp
  #obsolete tools/tobrowsertable.cpp
  tools/tobrowseraccesswidget.cpp
  tools/tobrowsertablewidget.cpp
  tools/tobrowserviewwidget.cpp
  tools/tobrowserindexwidget.cpp
  tools/tobrowserschemawidget.cpp
  tools/tobrowsersequencewidget.cpp
  tools/tobrowsersynonymwidget.cpp
  tools/tobrowsercodewidget.cpp
  tools/tobrowsertriggerwidget.cpp
  tools/tobrowserdblinkswidget.cpp
  tools/tobrowserdirectorieswidget.cpp

  tools/tostorage.cpp
  tools/tostoragedefinition.cpp
  tools/tofilesize.cpp
  tools/toresultstorage.cpp
  tools/toanalyze.cpp    
  tools/toawr.cpp

  tools/tosession.cpp
  tools/towaitevents.cpp
  tools/tocurrent.cpp
  tools/toinvalid.cpp 
  tools/tobackup.cpp
  tools/tobackuptool.cpp
  tools/tounittest.cpp
  tools/tosecurity.cpp
  tools/tosecuritytreemodel.cpp
  tools/tosecuritytreeitem.cpp

  tools/toscriptschemawidget.cpp
  tools/toscripttreeitem.cpp
  tools/toscripttreemodel.cpp

  tools/toprofiler.cpp

  tools/totemporary.cpp

  tools/torollback.cpp

  tools/toplsqleditor.cpp
  tools/toplsqltext.cpp

  templates/tosqledit.cpp
  templates/totemplate.cpp
  templates/totemplateprovider.cpp
  #     toproject.cpp

  #TODO tools/toalert.cpp
  #TODO tools/totuning.cpp

  #     todebug.cpp
  #     todebugtext.cpp
  #     toeditextensions.cpp
  #     toextratranslations.cpp
  #     tochartmanager.cpp
  #     tolegendchart.cpp

  #     toreport.cpp
  #     toresultlabel.cpp

  #     toscheduler.cpp

  #     tooracletable.cpp
  )

IF (PCH_DEFINED)
  LIST(APPEND TORA_SOURCES ${PCH_SOURCE})
ENDIF (PCH_DEFINED)

IF(ORACLE_FOUND)
  LIST(APPEND TORA_SOURCES connection/tooraclesetting.cpp connection/tooraclefind.cpp)
ENDIF(ORACLE_FOUND)

IF (USE_EXPERIMENTAL)
  LIST(APPEND TORA_SOURCES tools/toscript.cpp)
  LIST(APPEND TORA_SOURCES
    docklets/tocodeoutline.cpp
    parsing/OraclePLSQLLexer.cc
    parsing/OraclePLSQLParser.cc
    parsing/OracleSQLLexer.cc
    parsing/OracleSQLParser.cc
    parsing/error_handler.cc
    parsing/tplsqlparseoracle.cc
    parsing/tsqllexeroracle.cc
    parsing/tsqlparseoracle.cc
    parsing/tsqlparseoracle2.cc
    )
  LIST (APPEND TORA_SOURCES
    docklets/toquerymodel.cpp
    docklets/toastwalk.cpp
    )
  LIST (APPEND TORA_SOURCES
    tools/toer.cpp
    widgets/toresultdrawing.cpp
    )
  LIST (APPEND TORA_SOURCES tools/tosandboxtool.cpp)
ENDIF (USE_EXPERIMENTAL)

IF(POPPLER_QT4_FOUND)
  #    LIST(APPEND TORA_SOURCES pdfview/pdfviewwidget.cpp pdfview/documentwidget.cpp)
ENDIF (POPPLER_QT4_FOUND)

IF(TORA_DEBUG)
  LIST(APPEND TORA_SOURCES docklets/tologging.cpp)  
ENDIF()

################################################################################
#             compiled in icons for windows(MSVC and Mingw way)                #
################################################################################
IF (WIN32 AND MSVC)
  LIST(APPEND TORA_SOURCES tora.rc)
ENDIF (WIN32 AND MSVC)
IF (WIN32 AND MINGW)
  # resource compilation for mingw
  ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tora_rc.o
    COMMAND ${CMAKE_RC_COMPILER} -I${CMAKE_CURRENT_SOURCE_DIR}
    -i${CMAKE_CURRENT_SOURCE_DIR}/tora.rc
    -o ${CMAKE_CURRENT_BINARY_DIR}/tora_rc.o)
  LIST(APPEND TORA_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/tora_rc.o)
ENDIF (WIN32 AND MINGW)

if(QT5_BUILD)
#QT5_ADD_TRANSLATION(TORA_I18N_QM ${TORA_I18N_TS} ${TORA_I18N_TS_TOAD})
QT5_WRAP_UI(TORA_UI_SOURCES ${TORA_UI_SRC} )
QT5_WRAP_CPP(TORA_MOC_SOURCES ${TORA_MOC_CLASSES})
QT5_ADD_RESOURCES(TORA_RCC_SRCS ${TORA_RCC_FILES})
else()
QT4_ADD_TRANSLATION(TORA_I18N_QM ${TORA_I18N_TS} ${TORA_I18N_TS_TOAD})
QT4_WRAP_UI(TORA_UI_SOURCES ${TORA_UI_SRC} )
QT4_WRAP_CPP(TORA_MOC_SOURCES ${TORA_MOC_CLASSES})
QT4_ADD_RESOURCES(TORA_RCC_SRCS ${TORA_RCC_FILES})
endif()

################################################################################
#                       Libraries and library imports                          #
################################################################################
ADD_DEFINITIONS("-DTSQLPARSER_DLL")

IF (WANT_INTERNAL_QSCINTILLA)
  SET (TORA_QSCINTILLA_LIB tora_qscintilla2_lib)
ENDIF (WANT_INTERNAL_QSCINTILLA)

IF (WANT_INTERNAL_LOKI)
  SET (TORA_LOKI_LIB tora_loki_lib)
ELSE (WANT_INTERNAL_LOKI)
  SET (TORA_LOKI_LIB ${LOKI_LIBRARIES})
ENDIF (WANT_INTERNAL_LOKI)

# Handle win/mac special stuff - it's left empty for linux/unix
SET (GUI_TYPE)
IF (APPLE)
  SET(GUI_TYPE MACOSX_BUNDLE)
  SET(APPLE_BUNDLE_SOURCES icons/tora.icns)
ENDIF (APPLE)

IF(WIN32)
  IF (MSVC AND NOT TORA_DEBUG)
    # this is a must to switch off subsystem:console (console window poping up)
    # WinMain wrapper from QT_QTMAIN_LIBRARY is used and liner must be modified
    LINK_LIBRARIES(${QT_QTMAIN_LIBRARY})
    SET (GUI_TYPE WIN32)
  ENDIF ()
ENDIF(WIN32)

#Set up libraries to link with all found packages
SET(TORA_LIBS
  ${QT_LIBRARIES}               # dynamic qt4
  ${Qt5Core_LIBRARIES}          # dynamic qt5
  ${Qt5Gui_LIBRARIES}           # dynamic qt5
  ${Qt5Network_LIBRARIES}       # dynamic qt5
  ${Qt5PrintSupport_LIBRARIES}  # dynamic qt5  
  ${Qt5Sql_LIBRARIES}           # dynamic qt5
  ${Qt5Widgets_LIBRARIES}       # dynamic qt5
  ${Qt5Xml_LIBRARIES}           # dynamic qt5
  ${QSCINTILLA_LIBRARIES}       # dynamic
  ${TORA_LOKI_LIB} 		# static
  ermodel                       # static
  )

IF (APPLE)
   LIST(APPEND TORA_LIBS ${Qt5MacExtras_LIBRARIES})
ENDIF()

IF (POPPLER_QT4_FOUND)
  LIST(APPEND TORA_LIBS ${POPPLER_QT4_LIBRARIES})
ENDIF(POPPLER_QT4_FOUND)

IF (WANT_INTERNAL_QSCINTILLA)
  LIST(APPEND TORA_LIBS ${TORA_QSCINTILLA_LIB})
ENDIF (WANT_INTERNAL_QSCINTILLA)

IF (WANT_INTERNAL_LOKI)
  LIST(APPEND TORA_LIBS ${TORA_LOKI_LIB})
ENDIF (WANT_INTERNAL_LOKI)

# prevent opening the cmd.exe window
IF (WIN32)
  LIST(APPEND TORA_LIBS "ws2_32.lib")
  # don't use console subsystem
  IF (MINGW)
    LIST(APPEND TORA_LIBS "-mwindows")
  ENDIF (MINGW)
ELSE (WIN32)
  LIST(APPEND TORA_LIBS ${CMAKE_DL_LIBS})
ENDIF (WIN32)

IF (POSTGRESQL_FOUND)
   LIST(APPEND TORA_LIBS ${POSTGRESQL_LIBRARIES})
ENDIF (POSTGRESQL_FOUND)

IF(USE_EXPERIMENTAL)
  LIST(APPEND TORA_LIBS antlr3c)
ENDIF(USE_EXPERIMENTAL)

SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
SET(CMAKE_INSTALL_RPATH "$ORIGIN/:$$ORIGIN/")

# The main executable
ADD_EXECUTABLE(${EXE_NAME} ${GUI_TYPE}
  main/main.cpp
  ${TORA_UI_SOURCES}  
  ${TORA_I18N_QM}
  ${TORA_MOC_SOURCES}
  ${TORA_SOURCES}
  ${TORA_RCC_SRCS}
  ${APPLE_BUNDLE_SOURCES}
  )
TARGET_LINK_LIBRARIES(${EXE_NAME} ${TORA_LIBS})
SET_TARGET_PROPERTIES(${EXE_NAME} PROPERTIES ENABLE_EXPORTS ON)
ADD_DEPENDENCIES(${EXE_NAME} revisiontag) # explicitly say that the executable depends on the svnheader
IF(PCH_DEFINED)
  # NOTE: this macro calls SET_SOURCE_FILES_PROPERTIES macro which set props for source within ALL tagets
  ADD_PRECOMPILED_HEADER(${EXE_NAME} ${PCH_HEADER} FORCEINCLUDE)
ENDIF()

FILE(GLOB CORE_OBJECT_SOURCES "core/*.cpp" "*core/*.cxx")

FOREACH(item ${TORA_MOC_SOURCES})
STRING(REGEX MATCH ".*/core/moc_.*c..$" item ${item})
IF(item)
 LIST(APPEND CORE_OBJECT_SOURCES ${item})
ENDIF(item)
ENDFOREACH(item ${TORA_MOC_SOURCES})

IF(TORA_DEBUG AND TEST_APP1)
# test1
SET(TEST1_SOURCES
  parsing/tsqllexeroracle2.cc
  parsing/PLSQLGuiLexer.cc
  parsing/persistenttrie.cpp
  #disabled parsing/tsqlparse.cpp
  ${PCH_SOURCE}
  )
SET(TEST1_MOC_SOURCES
  core/moc_toglobalevent.cxx
  parsing/moc_tsqllexer.cxx
  )
ADD_EXECUTABLE("test1" ${GUI_TYPE}
  tests/test1.cpp
  ${TEST1_MOC_SOURCES}
  ${TEST1_SOURCES}
  ${TORA_RCC_SRCS}
  )
TARGET_LINK_LIBRARIES("test1" ${TORA_LIBS})
SET_TARGET_PROPERTIES("test1" PROPERTIES ENABLE_EXPORTS ON)
ADD_DEPENDENCIES("test1" revisiontag) # explicitly say that the executable depends on the svnheader
IF(PCH_DEFINED)
  ADD_PRECOMPILED_HEADER("test1" ${PCH_HEADER} FORCEINCLUDE)
ENDIF(PCH_DEFINED)
ENDIF(TORA_DEBUG AND TEST_APP1)

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

################################################################################
#                       Connection provider libraries                          #
################################################################################
# Oracle connection provider
ADD_DEFINITIONS("-DTROTL_DLL")
IF(ORACLE_FOUND)
  SET(PROVIDER_ORACLE "poracle")
  ADD_LIBRARY(${PROVIDER_ORACLE} SHARED 
  	"connection/tooracleprovider.cpp" 
  	"connection/tooracletraits.cpp" 
	"connection/tooracleconnection.cpp" 
	"connection/tooraclequery.cpp")
  IF(QT5_BUILD)      
    TARGET_LINK_LIBRARIES(${PROVIDER_ORACLE} ${ORACLE_LIBRARIES} Qt5::Core Qt5::Widgets ${TORA_LIB} "trotl")
  ELSE()
    TARGET_LINK_LIBRARIES(${PROVIDER_ORACLE} ${ORACLE_LIBRARIES} ${QT_LIBRARIES} ${TORA_LIB} "trotl")    
  ENDIF()
  IF(DEFINED TORA_LIB)
    ADD_DEPENDENCIES(${PROVIDER_ORACLE} ${TORA_LIB})
  ELSE()
    ADD_DEPENDENCIES(${PROVIDER_ORACLE} ${EXE_NAME})
  ENDIF()
  MESSAGE(STATUS "${PROVIDER_ORACLE} ${ORACLE_LIBRARIES} ${QT_LIBRARIES} ${TORA_LIB} trotl") 
ENDIF(ORACLE_FOUND)

# Teradata connection provider
IF(TERADATA_FOUND)
  SET(PROVIDER_TERADATA "pteradata")
  ADD_DEFINITIONS("-DNO_CLIV2_ERROR_T")
  ADD_LIBRARY(${PROVIDER_TERADATA} SHARED 
	"connection/toteradataprovider.cpp" 
	"connection/toteradatatraits.cpp")
  TARGET_LINK_LIBRARIES(${PROVIDER_TERADATA} ${TERADATA_LIBRARIES} ${QT_LIBRARIES} ${TORA_LIB})
  IF(DEFINED TORA_LIB)
    ADD_DEPENDENCIES(${PROVIDER_TERADATA} ${TORA_LIB})
  ELSE()
    ADD_DEPENDENCIES(${PROVIDER_TERADATA} ${EXE_NAME})
  ENDIF()
ENDIF (TERADATA_FOUND)

################################################################################
#                                  Install                                     #
################################################################################
IF (WIN32)
    # Windows install
    INSTALL(TARGETS ${EXE_NAME} ${PROVIDER_ORACLE} ${PROVIDER_TERADATA} RUNTIME DESTINATION bin LIBRARY DESTINATION bin)
    INSTALL(FILES ${TORA_I18N_QM} DESTINATION ./i18n )
    
ELSEIF (APPLE)
    # mac's bundle install
    SET_TARGET_PROPERTIES(${EXE_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/osx_tools/Info.plist.in")
    SET(MACOSX_BUNDLE_ICON_FILE tora.icns)
    SET(MACOSX_BUNDLE_INFO_STRING "${EXE_NAME} ${VERSION}")
    SET(MACOSX_BUNDLE_GUI_IDENTIFIER "com.torasql")
    SET(MACOSX_BUNDLE_LONG_VERSION_STRING "${VERSION}")
    SET(MACOSX_BUNDLE_BUNDLE_NAME "${EXE_NAME}")
    SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION}")
    SET(MACOSX_BUNDLE_BUNDLE_VERSION "${VERSION}")
    SET(MACOSX_BUNDLE_COPYRIGHT "(c) TOra developers")
    SET_SOURCE_FILES_PROPERTIES(${CMAKE_SOURCE_DIR}/src/icons/tora.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

    INSTALL(TARGETS ${EXE_NAME} BUNDLE DESTINATION ${CMAKE_INSTALL_PREFIX})

    INSTALL(FILES ${TORA_I18N_QM} DESTINATION ${CMAKE_INSTALL_PREFIX}/${EXE_NAME}.app/Contents/Resources/i18n)

    add_definitions(-DSHARE_DIR=\"../Resources/i18n\")
    add_definitions(-DPROVIDERS_PATH=\"../Libraries\")

ELSEIF (UNIX)

    # Unix install
    install(TARGETS ${EXE_NAME} RUNTIME DESTINATION bin LIBRARY DESTINATION lib${LIB_SUFFIX})
    install(FILES ${TORA_I18N_QM} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/tora-${VERSION}/i18n)
    install(TARGETS ${PROVIDER_ORACLE} RUNTIME DESTINATION bin LIBRARY DESTINATION lib${LIB_SUFFIX}/tora-${VERSION})
    install(TARGETS ${PROVIDER_TERADATA} RUNTIME DESTINATION bin LIBRARY DESTINATION lib${LIB_SUFFIX}/tora-${VERSION})
    install(FILES tora.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications/ )
    add_definitions(-DPROVIDERS_PATH=\"${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/tora-${VERSION}\")
    add_definitions(-DSHARE_DIR=\"${CMAKE_INSTALL_PREFIX}/share/tora-${VERSION}\")
ENDIF()

#set (FILES_TO_TRANSLATE ${TORA_UI_SRC} ${TORA_SOURCES} ${TORA_MOC_CLASSES})
#qt4_create_translation(QM_FILES ${FILES_TO_TRANSLATE} ${TORA_I18N_TS})
#add_custom_command (lupdate DEPENDS ${QM_FILES} )
