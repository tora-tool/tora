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
  ${CMAKE_SOURCE_DIR}/extlibs/dtl
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

################################################################################
#                              User interface files                            #
################################################################################
SET(TORA_UI_SRC
  connection/toqmysqlsettingui.ui
  connection/toqpsqlsettingui.ui

  main/toconnectionimportui.ui
  main/tomessageui.ui
  main/tonewconnectionui.ui
  main/topreferencesui.ui
  main/tosyntaxsetupui.ui

  result/toresulttabledataui.ui

  shortcuteditor/shortcuteditordialog.ui

  tools/tobrowserfilterui.ui
  tools/todescribeui.ui
  tools/todroptablespaceui.ui
  tools/tolinechartsetupui.ui
  tools/tooutputsettingui.ui
  tools/toparamgetui.ui
  tools/torollbackdialogui.ui
  tools/torollbacksettingui.ui
  tools/toscriptschemawidgetui.ui
  tools/toscriptui.ui
  tools/tosecurityquotaui.ui
  tools/tosecurityroleui.ui
  tools/tosecurityuserui.ui
  tools/tosessiondisconnectdlgui.ui
  tools/tosgatracesettingui.ui
  tools/tostoragedatafileui.ui
  tools/tostoragedefinitionui.ui
  tools/tostoragedialogui.ui
  tools/tostorageprefsui.ui
  tools/tostoragetablespaceui.ui
  tools/totuningoverviewui.ui
  tools/totuningsettingui.ui

  widgets/connectioncolorsdialogui.ui
  widgets/toaboutcopyingui.ui
  widgets/toaboutui.ui
  widgets/todatabasesettingui.ui
  widgets/toglobalsettingui.ui
  widgets/tohelpaddfileui.ui
  widgets/tohelpsetupui.ui
  widgets/tohelpui.ui
  widgets/toresultcontentfilterui.ui
  widgets/toresultlistformatui.ui
  widgets/tosearchreplaceui.ui
  widgets/tosessionsetupui.ui
  widgets/totoolsettingui.ui
  widgets/toworksheetsetupui.ui

  #obsolete tools/tobrowserconstraintui.ui
  #obsolete tools/tobrowserindexui.ui
  #obsolete tools/tobrowsertableui.ui

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
  LIST (APPEND TORA_UI_SRC main/tooraclesettingui.ui)
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
  connection/toqmysqlsetting.h
  connection/toqpsqlsetting.h
  core/tobackground.h
  core/tocache.h
  core/tochangeconnection.h
  core/tocodemodel.h
  core/toconfenum.h
  core/toconfiguration.h
  core/toconnection.h
  core/toconnectionmodel.h
  core/toconnectionregistry.h
  core/tocontextmenu.h
  core/todatabaseconfig.h
  core/todocklet.h
  core/toeditablemenu.h
  core/toeditmenu.h
  core/toeditorconfiguration.h
  core/toeventquery.h
  core/toeventqueryworker.h
  core/toextract.h
  core/toglobalconfiguration.h
  core/toglobalevent.h
  core/tohelpcontext.h
  core/tolistviewformatter.h
  core/tomainwindow.h
  core/toquery.h
  core/toqueryimpl.h
  core/toresult.h
  core/tosettingtab.h
  core/tostyle.h
  core/tosyntaxanalyzer.h
  core/totextview.h
  core/totool.h
  core/toupdater.h
  core/utils.h
  docklets/toviewconnections.h
  docklets/toviewdirectory.h
  editor/tobaseeditor.h
  editor/tocomplpopup.h
  editor/todebugeditor.h
  editor/todebugtext.h
  editor/todifftext.h
  editor/tohighlightededitor.h
  editor/tomarkededitor.h
  editor/tomemoeditor.h
  editor/tomodeleditor.h
  editor/toscintilla.h
  editor/tosqltext.h
  editor/tosyntaxanalyzermysql.h
  editor/tosyntaxanalyzernl.h
  editor/tosyntaxanalyzeroracle.h
  editor/tosyntaxanalyzerpostgresql.h
  editor/toworksheettext.h

  main/toconnectionimport.h
  main/tomain.h
  main/tomessage.h
  main/tonewconnection.h
  main/topreferences.h
  main/tosyntaxsetup.h

  parsing/tolexermysql.h
  parsing/tolexeroracle.h
  parsing/tolexeroracleapis.h
  parsing/tolexerpostgresql.h
  parsing/toscilexersql.h
  parsing/tsqllexer.h

  result/toproviderobserver.h
  result/toresultlock.h
  result/toresultplan.h
  result/toresulttabledata.h
  result/toresultwaitchains.h
  result/totablemodel.h
  result/totableview.h
  result/totreemodel.h
  result/totreeview.h

  shortcuteditor/shortcuteditordialog.h
  shortcuteditor/shortcutmodel.h

  templates/tosqledit.h
  templates/totemplate.h

  tools/toanalyze.h
  tools/toawr.h
  tools/tobackup.h
  tools/tobackuptool.h
  tools/tobarchart.h
  tools/tobrowser.h
  tools/tobrowseraccesswidget.h
  tools/tobrowserbasewidget.h # this one is used by widgets/todescribe.h
  tools/tobrowsercodewidget.h
  tools/tobrowserdblinkswidget.h
  tools/tobrowserdirectorieswidget.h
  tools/tobrowserindexwidget.h
  tools/tobrowserschemawidget.h
  tools/tobrowsersequencewidget.h
  tools/tobrowsersynonymwidget.h
  tools/tobrowsertablewidget.h
  tools/tobrowsertriggerwidget.h
  tools/tobrowserviewwidget.h
  tools/tocurrent.h
  tools/todescribe.h
  tools/tofilesize.h
  tools/toinvalid.h
  tools/tolinechart.h
  tools/tooutput.h
  tools/toparamget.h
  tools/topiechart.h
  tools/toplsqleditor.h
  tools/toplsqltext.h
  tools/toprofiler.h
  tools/toresultbar.h
  tools/toresultcode.h
  tools/toresultcols.h
  tools/toresultdatasingle.h
  tools/toresultdepend.h
  tools/toresultdrawing.h
  tools/toresultextent.h
  tools/toresultfield.h
  tools/toresultgrants.h
  tools/toresultline.h
  tools/toresultlong.h
  tools/toresultparam.h
  tools/toresultpie.h
  tools/toresultplan.h
  tools/toresultstats.h
  tools/toresultstorage.h
  tools/toresulttableview.h
  tools/toresulttableviewedit.h
  tools/toresultview.h
  tools/torollback.h
  tools/toscriptschemawidget.h
  tools/toscripttreeitem.h
  tools/toscripttreemodel.h
  tools/tosecurity.h
  tools/tosecuritytreeitem.h
  tools/tosecuritytreemodel.h
  tools/tosession.h
  tools/tosgastatement.h
  tools/tosgatrace.h
  tools/tostorage.h
  tools/totemporary.h
  tools/totuning.h
  tools/totuningcharts.h
  tools/totuningfileio.h
  tools/totuningoverview.h
  tools/tounittest.h
  tools/towaitevents.h
  tools/toworksheet.h
  tools/toworksheeteditor.h
  tools/toworksheetstatistic.h

  ts_log/toostream.h

  widgets/ExtendedTabWidget.h
  widgets/toabout.h
  widgets/toaboutcopying.h
  widgets/tobackgroundlabel.h
  widgets/toconnectionwidget.h
  widgets/todatabasesetting.h
  widgets/todockbar.h
  widgets/toglobalsetting.h
  widgets/tohelp.h
  widgets/tohelpsetup.h
  widgets/toplaintextview.h
  widgets/topushbutton.h
  widgets/torefreshcombo.h
  widgets/toresultcolscomment.h
  widgets/toresultcombo.h
  widgets/toresultitem.h
  widgets/toresultlistformat.h
  widgets/toresultmodel.h
  widgets/toresultmodeledit.h
  widgets/toresultschema.h
  widgets/tosearchreplace.h
  widgets/tosplash.h
  widgets/tosqltextview.h
  widgets/totabledetailview.h
  widgets/totableselect.h
  widgets/totabwidget.h
  widgets/totoolwidget.h
  widgets/totreewidget.h
  widgets/toworkingwidget.h
  widgets/toworkspace.h

  #obsolete core/todatatype.h
  #obsolete core/torunnable.h
  #obsolete core/totaskrunner.h
  #obsolete core/tothread.h
  #obsolete core/tovisualize.h
  #core/toconfiguration.h
  #core/toconfigurationpriv.h  
  #obsolete tools/tobrowserconstraint.h
  #obsolete tools/tobrowserindex.h
  #obsolete tools/tobrowsertable.h

  #     toproject.h
  
  #TODO tools/toalert.h
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
  LIST(APPEND TORA_MOC_CLASSES main/tooraclesetting.h connection/tooracleconfiguration.h)
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
  LIST(APPEND TORA_MOC_CLASSES docklets/tologging.h)
  LIST(APPEND TORA_MOC_CLASSES docklets/tobindvars.h)
ENDIF (USE_EXPERIMENTAL)

################################################################################
#                              Source files .cpp .rc                           #
################################################################################
# sources
SET(TORA_SOURCES
  connection/tooracleextract.cpp
  connection/tooraclesql.cpp
  connection/toqmysqlconnection.cpp
  connection/toqmysqlprovider.cpp
  connection/toqmysqlquery.cpp
  connection/toqmysqlsetting.cpp
  connection/toqmysqltraits.cpp
  connection/toqodbcprovider.cpp
  connection/toqpsqlconnection.cpp
  connection/toqpsqlprovider.cpp
  connection/toqpsqlquery.cpp
  connection/toqpsqlsetting.cpp
  connection/toqpsqltraits.cpp
  connection/toqsqlconnection.cpp
  connection/toqsqlfind.cpp
  connection/toqsqlprovider.cpp
  connection/toqsqlquery.cpp
  connection/toteradatafind.cpp

  core/persistenttrie.cpp
  core/tobackground.cpp
  core/tocache.cpp
  core/tochangeconnection.cpp
  core/tocodemodel.cpp
  core/toconfenum.cpp
  core/toconfiguration.cpp
  core/toconnection.cpp
  core/toconnectionmodel.cpp
  core/toconnectionoptions.cpp
  core/toconnectionprovider.cpp
  core/toconnectionregistry.cpp
  core/toconnectionsub.cpp
  core/toconnectionsubloan.cpp
  core/tocontextmenu.cpp
  core/todatabaseconfig.cpp
  core/todocklet.cpp
  core/toeditablemenu.cpp
  core/toeditmenu.cpp
  core/toeditorconfiguration.cpp
  core/toeditwidget.cpp
  core/toeventquery.cpp
  core/toeventqueryworker.cpp
  core/toextract.cpp
  core/toglobalconfiguration.cpp
  core/toglobalevent.cpp
  core/tohelpcontext.cpp
  core/tohtml.cpp
  core/tolistviewformatter.cpp
  core/tolistviewformattercsv.cpp
  core/tolistviewformatterhtml.cpp
  core/tolistviewformattersql.cpp
  core/tolistviewformattertabdel.cpp
  core/tolistviewformattertext.cpp
  core/tolistviewformatterxlsx.cpp
  core/tomainwindow.cpp
  core/toquery.cpp
  core/toqvalue.cpp
  core/toresult.cpp
  core/tosettingtab.cpp
  core/tosql.cpp
  core/tostyle.cpp
  core/tosyntaxanalyzer.cpp
  core/totextview.cpp
  core/totool.cpp
  core/toupdater.cpp
  core/utils.cpp
  core/utils_part.cpp

  docklets/toviewconnections.cpp
  docklets/toviewdirectory.cpp

  editor/tobaseeditor.cpp
  editor/tocomplpopup.cpp
  editor/todebugeditor.cpp
  editor/todebugtext.cpp
  editor/todifftext.cpp
  editor/tohighlightededitor.cpp
  editor/tomarkededitor.cpp
  editor/tomemoeditor.cpp
  editor/tomodeleditor.cpp
  editor/toscintilla.cpp
  editor/tosqltext.cpp
  editor/tosyntaxanalyzermysql.cpp
  editor/tosyntaxanalyzernl.cpp
  editor/tosyntaxanalyzeroracle.cpp
  editor/tosyntaxanalyzerpostgresql.cpp
  editor/toworksheettext.cpp

  main/toconnectionimport.cpp
  main/tomain.cpp
  main/tomessage.cpp
  main/tonewconnection.cpp
  main/topreferences.cpp
  main/tosyntaxsetup.cpp

  migratetool/sqldeveloper.cpp
  migratetool/squirrelsql.cpp
  migratetool/tora3.cpp

  parsing/MySQLGuiLexer.cc
  parsing/MySQLLexer.cc
  parsing/OracleDML.cc
  parsing/OracleDML.hpp
  parsing/OracleDMLLexer.cc
  parsing/OracleDMLLexer.hpp
  parsing/OracleDMLTraits.hpp
  parsing/OracleDML_OracleDMLCommons.cc
  parsing/OracleDML_OracleDMLCommons.hpp
  parsing/OracleDML_OracleDMLKeys.cc
  parsing/OracleDML_OracleDMLKeys.hpp
  parsing/PLSQLGuiLexer.cc
  parsing/PostgreSQLGuiLexer.cc
  parsing/tolexermysql.cpp
  parsing/tolexeroracle.cpp
  parsing/tolexeroracleapis.cpp
  parsing/tolexerpostgresql.cpp
  parsing/toscilexersql.cpp
  parsing/tsqllexermysql.cc
  parsing/tsqllexermysql2.cc
  parsing/tsqllexeroracle2.cc
  parsing/tsqllexerpostgresql.cc
  parsing/tsqlparse.cpp

  result/toproviderobserver.cpp
  result/toresultlock.cpp
  result/toresultplan.cpp
  result/toresulttabledata.cpp
  result/toresultwaitchains.cpp
  result/totablemodel.cpp
  result/totableview.cpp
  result/totreemodel.cpp
  result/totreeview.cpp

  shortcuteditor/shortcuteditordialog.cpp
  shortcuteditor/shortcutmodel.cpp

  templates/tosqledit.cpp
  templates/totemplate.cpp
  templates/totemplateprovider.cpp

  tools/toanalyze.cpp
  tools/toawr.cpp
  tools/tobackup.cpp
  tools/tobackuptool.cpp
  tools/tobarchart.cpp
  tools/tobrowser.cpp
  tools/tobrowseraccesswidget.cpp
  tools/tobrowserbasewidget.cpp # this one is used by widgets/todescribe.h
  tools/tobrowsercodewidget.cpp
  tools/tobrowserdblinkswidget.cpp
  tools/tobrowserdirectorieswidget.cpp
  tools/tobrowserindexwidget.cpp
  tools/tobrowserschemawidget.cpp
  tools/tobrowsersequencewidget.cpp
  tools/tobrowsersynonymwidget.cpp
  tools/tobrowsertablewidget.cpp
  tools/tobrowsertriggerwidget.cpp
  tools/tobrowserviewwidget.cpp
  tools/tocurrent.cpp
  tools/todescribe.cpp
  tools/tofilesize.cpp
  tools/toinvalid.cpp
  tools/tolinechart.cpp
  tools/tooutput.cpp
  tools/toparamget.cpp
  tools/topiechart.cpp
  tools/toplsqleditor.cpp
  tools/toplsqltext.cpp
  tools/toprofiler.cpp
  tools/toresultbar.cpp
  tools/toresultcode.cpp
  tools/toresultcols.cpp
  tools/toresultdatasingle.cpp
  tools/toresultdepend.cpp
  tools/toresultdrawing.cpp
  tools/toresultextent.cpp
  tools/toresultfield.cpp
  tools/toresultgrants.cpp
  tools/toresultline.cpp
  tools/toresultlong.cpp
  tools/toresultparam.cpp
  tools/toresultpie.cpp
  tools/toresultplan.cpp
  tools/toresultstats.cpp
  tools/toresultstorage.cpp
  tools/toresulttableview.cpp
  tools/toresulttableviewedit.cpp
  tools/toresultview.cpp
  tools/torollback.cpp
  tools/toscriptschemawidget.cpp
  tools/toscripttreeitem.cpp
  tools/toscripttreemodel.cpp
  tools/tosecurity.cpp
  tools/tosecuritytreeitem.cpp
  tools/tosecuritytreemodel.cpp
  tools/tosession.cpp
  tools/tosgastatement.cpp
  tools/tosgatrace.cpp
  tools/tostorage.cpp
  tools/tostoragedefinition.cpp
  tools/totemporary.cpp
  tools/totuning.cpp
  tools/totuningcharts.cpp
  tools/totuningfileio.cpp
  tools/totuningoverview.cpp
  tools/tounittest.cpp
  tools/towaitevents.cpp
  tools/toworksheet.cpp
  tools/toworksheeteditor.cpp
  tools/toworksheetstatistic.cpp

  widgets/ExtendedTabWidget.cpp
  widgets/toabout.cpp
  widgets/toaboutcopying.cpp
  widgets/tobackgroundlabel.cpp
  widgets/toconnectionwidget.cpp
  widgets/todatabasesetting.cpp
  widgets/todockbar.cpp
  widgets/toglobalsetting.cpp
  widgets/tohelp.cpp
  widgets/tohelpsetup.cpp
  widgets/toplaintextview.cpp
  widgets/topushbutton.cpp
  widgets/torefreshcombo.cpp
  widgets/toresultcolscomment.cpp
  widgets/toresultcombo.cpp
  widgets/toresultitem.cpp
  widgets/toresultlistformat.cpp
  widgets/toresultmodel.cpp
  widgets/toresultmodeledit.cpp
  widgets/toresultschema.cpp
  widgets/tosearchreplace.cpp
  widgets/tosplash.cpp
  widgets/tosqltextview.cpp
  widgets/totabledetailview.cpp
  widgets/totableselect.cpp
  widgets/totabwidget.cpp
  widgets/totoolwidget.cpp
  widgets/totreewidget.cpp
  widgets/toworkingwidget.cpp
  widgets/toworkspace.cpp

  #core/toconfiguration.cpp
  #core/toconfigurationpriv.cpp
  #obsolete core/todatatype.cpp
  #obsolete core/totaskrunner.cpp
  #obsolete core/tovisualize.cpp
  #obsolete tools/tobrowserconstraint.cpp
  #obsolete tools/tobrowserindex.cpp
  #obsolete tools/tobrowsertable.cpp

  #     toproject.cpp

  #TODO tools/toalert.cpp

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
  LIST(APPEND TORA_SOURCES main/tooraclesetting.cpp connection/tooracleconfiguration.cpp connection/tooraclefind.cpp)
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
    )
  LIST (APPEND TORA_SOURCES tools/tosandboxtool.cpp)
  LIST (APPEND TORA_SOURCES docklets/tologging.cpp)
  LIST (APPEND TORA_SOURCES docklets/tobindvars.cpp)
  LIST (APPEND TORA_SOURCES core/tomemory.cpp)
ENDIF (USE_EXPERIMENTAL)

IF(POPPLER_QT4_FOUND)
  #    LIST(APPEND TORA_SOURCES pdfview/pdfviewwidget.cpp pdfview/documentwidget.cpp)
ENDIF (POPPLER_QT4_FOUND)

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
if(Qt5LinguistTools_FOUND)
QT5_ADD_TRANSLATION(TORA_I18N_QM ${TORA_I18N_TS} ${TORA_I18N_TS_TOAD})
endif()
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
ELSE()
  SET (TORA_QSCINTILLA_LIB ${QSCINTILLA_LIBRARIES})  
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
  ${TORA_QSCINTILLA_LIB}        # dynamic
  ${TORA_LOKI_LIB} 		# dynamic/static
  ${Boost_SYSTEM_LIBRARY}       # Linux only, lastest boost releases require this lib for singleton
  ermodel                       # static
  )

IF (APPLE)
   LIST(APPEND TORA_LIBS ${Qt5MacExtras_LIBRARIES})
ENDIF()

IF (POPPLER_QT4_FOUND)
  LIST(APPEND TORA_LIBS ${POPPLER_QT4_LIBRARIES})
ENDIF(POPPLER_QT4_FOUND)

# prevent opening the cmd.exe window
IF (WIN32)
  LIST(APPEND TORA_LIBS "ws2_32.lib")
  # don't use console subsystem
  IF (MINGW)
    LIST(APPEND TORA_LIBS "-mwindows")
  ENDIF (MINGW)
ENDIF (WIN32)

IF (UNIX)
  LIST(APPEND TORA_LIBS ${CMAKE_DL_LIBS})
ENDIF()

IF (POSTGRESQL_FOUND)
   LIST(APPEND TORA_LIBS ${POSTGRESQL_LIBRARIES})
ENDIF (POSTGRESQL_FOUND)

IF(USE_EXPERIMENTAL)
  LIST(APPEND TORA_LIBS antlr3c)
ENDIF(USE_EXPERIMENTAL)

IF(UNIX AND NOT APPLE)
  SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
  SET(CMAKE_INSTALL_RPATH "$ORIGIN/")
ELSE()
  #SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
  #SET(CMAKE_INSTALL_RPATH "Plugins/")
ENDIF()

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

################################################################################
#                                   Tests                                      #
################################################################################

IF (TORA_DEBUG AND USE_EXPERIMENTAL AND QT5_BUILD)
    FILE(GLOB CORE_OBJECT_SOURCES "core/*.cpp")
    FILE(GLOB CORE_OBJECT_HEADERS "core/*.h")
    QT5_WRAP_CPP(CORE_OBJECT_MOC_SOURCES ${CORE_OBJECT_HEADERS})
    #ADD_LIBRARY(CORE_OBJECT_LIB OBJECT ${CORE_OBJECT_SOURCES} ${CORE_OBJECT_MOC_SOURCES} ${PCH_SOURCE})
    SET(CORE_SOURCES ${CORE_OBJECT_SOURCES} ${CORE_OBJECT_MOC_SOURCES})
    
    FILE(GLOB PARSING_OBJECT_SOURCES "parsing/*.cpp" "parsing/*.cc")    
    FILE(GLOB PARSING_OBJECT_HEADERS "parsing/*.h")
    QT5_WRAP_CPP(PARSING_OBJECT_MOC_SOURCES ${PARSING_OBJECT_HEADERS})
    SET(PARSING_SOURCES ${PARSING_OBJECT_SOURCES} ${PARSING_OBJECT_MOC_SOURCES})

    FILE(GLOB WIDGETS_OBJECT_SOURCES "widgets/*.cpp")
    FILE(GLOB WIDGETS_OBJECT_UI "widgets/*.ui")
    QT5_WRAP_UI(WIDGETS_OBJECT_UI_SRC ${WIDGETS_OBJECT_UI})
    FILE(GLOB WIDGETS_OBJECT_HEADERS "widgets/*.h")
    QT5_WRAP_CPP(WIDGETS_OBJECT_MOC_SOURCES ${WIDGETS_OBJECT_HEADERS})
    SET(WIDGETS_SOURCES ${WIDGETS_OBJECT_SOURCES} ${WIDGETS_OBJECT_UI_SRC} ${WIDGETS_OBJECT_MOC_SOURCES})

    FILE(GLOB EDITOR_OBJECT_SOURCES "editor/*.cpp")
    FILE(GLOB EDITOR_OBJECT_UI "editor/*.ui")
    QT5_WRAP_UI(EDITOR_OBJECT_UI_SRC ${EDITOR_OBJECT_UI})
    FILE(GLOB EDITOR_OBJECT_HEADERS "editor/*.h")
    QT5_WRAP_CPP(EDITOR_OBJECT_MOC_SOURCES ${EDITOR_OBJECT_HEADERS})
    SET(EDITOR_SOURCES ${EDITOR_OBJECT_SOURCES} ${EDITOR_OBJECT_UI_SRC} ${EDITOR_OBJECT_MOC_SOURCES})

    FILE(GLOB ORACLE_OBJECT_SOURCES "connection/tooraclefind.cpp" "connection/tooracleconfiguration.cpp" "main/tooraclesetting.cpp")
    FILE(GLOB ORACLE_OBJECT_UI "main/tooraclesettingui.ui")
    QT5_WRAP_UI(ORACLE_OBJECT_UI_SRC ${ORACLE_OBJECT_UI})
    FILE(GLOB ORACLE_OBJECT_HEADERS "connection/tooracleconfiguration.h" "main/tooraclesetting.h")
    QT5_WRAP_CPP(ORACLE_OBJECT_MOC_SOURCES ${ORACLE_OBJECT_HEADERS})
    SET(ORACLE_SOURCES ${ORACLE_OBJECT_SOURCES} ${ORACLE_OBJECT_UI_SRC} ${ORACLE_OBJECT_MOC_SOURCES})

    FILE(GLOB LOGGING_OBJECT_HEADERS "ts_log/toostream.h")
    QT5_WRAP_CPP(LOGGING_OBJECT_MOC_SOURCES ${LOGGING_OBJECT_HEADERS})
    SET(LOGGING_SOURCES ${LOGGING_OBJECT_MOC_SOURCES})

    # add_subdirectory(tests)
    include(tests/CMakeLists.txt)
ENDIF()

################################################################################
#                       Connection provider libraries                          #
################################################################################
# Oracle connection provider
ADD_DEFINITIONS("-DTROTL_DLL")
IF(ORACLE_FOUND)
  SET(PROVIDER_ORACLE "poracle")
  ADD_LIBRARY(${PROVIDER_ORACLE} MODULE
  	"connection/tooracleprovider.cpp" 
  	"connection/tooracletraits.cpp" 
	"connection/tooracleconnection.cpp" 
	"connection/tooraclequery.cpp"
	"connection/tooracledatatype.cpp"
        )
  IF(APPLE AND TORA_DEBUG)
    SET(STACK_LIB "stack_lib")
  ENDIF()
  IF(QT5_BUILD)      
    TARGET_LINK_LIBRARIES(${PROVIDER_ORACLE} ${ORACLE_LIBRARIES} Qt5::Core Qt5::Widgets ${TORA_LIB} ${TROTL_LIB_FLAGS} "trotl" ${STACK_LIB})
  ELSE()
    TARGET_LINK_LIBRARIES(${PROVIDER_ORACLE} ${ORACLE_LIBRARIES} ${QT_LIBRARIES} ${TORA_LIB} ${TROTL_LIB_FLAGS} "trotl" ${STACK_LIB})
  ENDIF()
  IF(DEFINED TORA_LIB)
    ADD_DEPENDENCIES(${PROVIDER_ORACLE} ${TORA_LIB})
  ELSE()
    ADD_DEPENDENCIES(${PROVIDER_ORACLE} ${EXE_NAME})
  ENDIF()

  IF(APPLE)
    get_target_property(existing_link_flags ${PROVIDER_ORACLE} LINK_FLAGS)
    if(existing_link_flags)
      # TODO resolve abs path to TOra
      # set(new_link_flags " ${existing_link_flags} -bundle_loader ${EXE_NAME} ")
      set(new_link_flags "${existing_link_flags} -undefined dynamic_lookup ")
    else()
      # TODO resolve abs path to TOra
      # set(new_link_flags " -bundle_loader ${EXE_NAME} ")
      set(new_link_flags " -undefined dynamic_lookup ")
    endif()
  set_target_properties(${PROVIDER_ORACLE} PROPERTIES LINK_FLAGS ${new_link_flags})
  set_target_properties(${PROVIDER_ORACLE} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/src/TOra.app/Contents/PlugIns)
  MESSAGE(STATUS "${PROVIDER_ORACLE} ${new_link_flags}")
  ENDIF(APPLE)

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
    INSTALL(TARGETS ${EXE_NAME} ${PROVIDER_ORACLE} ${PROVIDER_TERADATA} RUNTIME DESTINATION Tora LIBRARY DESTINATION Tora)
    INSTALL(FILES ${TORA_I18N_QM} DESTINATION Tora/i18n )
    
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
