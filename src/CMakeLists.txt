#Subdirectories
ADD_SUBDIRECTORY(loki)
IF (WIN32)
    ADD_SUBDIRECTORY(windows)
ENDIF (WIN32)


# Default: qsci2 as 3rd party lib!
IF (WANT_INTERNAL_QSCINTILLA)
    ADD_SUBDIRECTORY(qscintilla2)
ENDIF(WANT_INTERNAL_QSCINTILLA)

SET(TORA_RCC_FILES tora.qrc)

SET(TORA_UI_SRC
    toaboutui.ui
    tobrowserconstraintui.ui
    tobrowserfilterui.ui
    tobrowserindexui.ui
    tobrowsertableui.ui
    todatabasesettingui.ui
    todebugchangeui.ui
    todebugwatch.ui
    todroptablespaceui.ui
    toeditextensiongotoui.ui
    toeditextensionsetupui.ui
    toglobalsettingui.ui
    tohelpaddfileui.ui
    tohelpui.ui
    tohelpsetupui.ui
    tochartalarmui.ui
    tochartsetupui.ui
    tolinechartsetupui.ui
    tomessageui.ui
    tonewconnectionui.ui
    topreferencesui.ui
    toresultcontentfilterui.ui
    toresultlistformatui.ui
    torollbackdialogui.ui
    toscriptui.ui
    tosearchreplaceui.ui
    tosecurityquotaui.ui
    tosecurityroleui.ui
    tosecurityuserui.ui
    tostoragedatafileui.ui
    tostoragedefinitionui.ui
    tostoragedialogui.ui
    tostorageprefsui.ui
    tostoragetablespaceui.ui
    tosyntaxsetupui.ui
    totemplateaddfileui.ui
    totemplateeditui.ui
    totemplatesetupui.ui
    totoolsettingui.ui
    totuningoverviewui.ui
    totuningsettingui.ui
    toworksheetsetupui.ui
)
IF (ORACLE_FOUND)
    LIST (APPEND TORA_UI_SRC tooraclesettingui.ui)
ENDIF (ORACLE_FOUND)


SET(TORA_MOC_CLASSES
    toabout.h
    toalert.h
    toanalyze.h
#     tobackground.h
#     tobackgroundlabel.h
    tobackup.h
#     tobackuptool.h
    tobarchart.h
    tobrowserconstraint.h
    tobrowser.h
    tobrowserindex.h
    tobrowsertable.h
#     toconf.h
#     toconfiguration.h
    toconnection.h
    tonoblockquery.h
    tocurrent.h
    todatatype.h
    todebug.h
    todebugtext.h
#     todefaultkeywords.h
    toeditextensions.h
#     toeditwidget.h
#     toextract.h
    toeventquery.h
    toeventquerytask.h
    tofilesize.h
    toglobalsetting.h
#     tohelpbrowser.h
    tohelp.h
    tohelpsetup.h
    tohighlightedtext.h
#     tohtml.h
    tochangeconnection.h
    tochartmanager.h
    toinvalid.h
    tolegendchart.h
    tolinechart.h
#     tolistviewformattercsv.h
#     tolistviewformatterfactory.h
#     tolistviewformatter.h
#     tolistviewformatterhtml.h
#     tolistviewformatteridentifier.h
#     tolistviewformattersql.h
#     tolistviewformattertabdel.h
#     tolistviewformattertext.h
    tomain.h
    tomainwindow.h
#     tomainwindow.kde.h
    tomarkedtext.h
    tomemoeditor.h
#     tomysqlkeywords.h
    tonewconnection.h
#     tonoblockquery.h
    tooutput.h
    toparamget.h
    topiechart.h
    topreferences.h
    toprofiler.h
    toproject.h
#     toqvalue.h
#     toreport.h
    toresultbar.h
    toresultcolscomment.h
    toresultcols.h
    toresultcombo.h
    toresultdepend.h
    toresultextract.h
    toresultfield.h
    toresult.h
    toresultitem.h
    toresultlabel.h
    toresultline.h
    toresultlock.h
    toresultlong.h
    toresultmodel.h
    toresultparam.h
    toresultpie.h
    toresultplan.h
#     toresultresources.h
    toresultstats.h
    toresultstorage.h
    toresulttableview.h
    toresulttableviewedit.h
    toresultdata.h
    toresultdatasingle.h
    toresultview.h
    toresultlistformat.h
    torollback.h
    toscript.h
    tosearchreplace.h
    tosecurity.h
    tosession.h
    tosgastatement.h
    tosgatrace.h
    tosmtp.h
#     tosplash.h
    tosqledit.h
#     tosql.h
#     tosqlparse.h
#     tostoragedefinition.h
    tostorage.h
    tosyntaxsetup.h
    totableselect.h
    totabwidget.h
    totemplate.h
    totemporary.h
#     totextview.h
    tothread_p.h
    totreewidget.h
#     totimer.h
    totool.h
    totuning.h
    tovisualize.h
    towaitevents.h
    toworksheet.h
    toworksheetstatistic.h
#     toworksheettext.h
    toworksheetwidget.h
    utils.h
)
IF (ORACLE_FOUND)
    LIST(APPEND TORA_MOC_CLASSES tooraclesetting.h)
ENDIF (ORACLE_FOUND)


SET(TORA_SOURCES
    main.cpp
    toabout.cpp
    toalert.cpp
    toanalyze.cpp
    tobackground.cpp
    tobackgroundlabel.cpp
    tobackup.cpp
    tobackuptool.cpp
    tobarchart.cpp
    tobrowserconstraint.cpp
    tobrowser.cpp
    tobrowserindex.cpp
    tobrowsertable.cpp
    toconfiguration.cpp
    toconnection.cpp
    tocurrent.cpp
    todatatype.cpp
    todebug.cpp
    todebugtext.cpp
    toeditextensions.cpp
    toeditwidget.cpp
    toextract.cpp
    toextratranslations.cpp
    toeventquery.cpp
    toeventquerytask.cpp
    tofilesize.cpp
# temporarily disabled due the lack of firebird here    tofyracleconnection.cpp
    toglobalsetting.cpp
    tohelp.cpp
    tohighlightedtext.cpp
    tohtml.cpp
    tochangeconnection.cpp
    tochartmanager.cpp
    toinvalid.cpp
    tolegendchart.cpp
    tolinechart.cpp
    tolistviewformatter.cpp
    tolistviewformattercsv.cpp
    tolistviewformatterhtml.cpp
    tolistviewformattersql.cpp
    tolistviewformattertabdel.cpp
    tolistviewformattertext.cpp
    tomain.cpp
    tomainwindow.cpp
    tomarkedtext.cpp
    tomemoeditor.cpp
    tonewconnection.cpp
    tonoblockquery.cpp
#win only?    toocistub.cpp
    tooracleconnection.cpp
    tooracleextract.cpp
    tooraclepreload.cpp
    tooutput.cpp
    toparamget.cpp
    topassword.cpp
    topiechart.cpp
    topreferences.cpp
    toprofiler.cpp
    toproject.cpp
    toqsqlconnection.cpp
    toreport.cpp
    toresultbar.cpp
    toresultcolscomment.cpp
    toresultcols.cpp
    toresultcombo.cpp
    toresult.cpp
    toresultdepend.cpp
    toresultextract.cpp
    toresultfield.cpp
    toresultitem.cpp
    toresultlabel.cpp
    toresultline.cpp
    toresultlock.cpp
    toresultlong.cpp
    toresultmodel.cpp
    toresultparam.cpp
    toresultpie.cpp
    toresultplan.cpp
    toresultstats.cpp
    toresultstorage.cpp
    toresulttableview.cpp
    toresulttableviewedit.cpp
    toresultdata.cpp
    toresultdatasingle.cpp
    toresultview.cpp
    toresultlistformat.cpp
    torollback.cpp
    toscript.cpp
    tosearchreplace.cpp
    tosecurity.cpp
    tosession.cpp
    tosgastatement.cpp
    tosgatrace.cpp
    tosmtp.cpp
    tosplash.cpp
    tosql.cpp
    tosqledit.cpp
    tosqlparse.cpp
    tostorage.cpp
    tostoragedefinition.cpp
    tosyntaxsetup.cpp
    totableselect.cpp
    totabwidget.cpp
    totemplate.cpp
    totemplateprovider.cpp
    totemporary.cpp
    totextview.cpp
    tothread.cpp
    totimer.cpp
    totool.cpp
    totreewidget.cpp
    totuning.cpp
    tovisualize.cpp
    towaitevents.cpp
    toworksheet.cpp
    toworksheetstatistic.cpp
    toworksheettext.cpp
    toworksheetwidget.cpp
    utils.cpp
)

QT4_WRAP_UI(TORA_UI_SOURCES ${TORA_UI_SRC} )
QT4_WRAP_CPP(TORA_MOC_SOURCES ${TORA_MOC_CLASSES})
QT4_ADD_RESOURCES(TORA_RCC_SRCS ${TORA_RCC_FILES})


LINK_DIRECTORIES(./loki)
IF (WIN32)
    LINK_DIRECTORIES(./windows)
ENDIF (WIN32)
IF (WANT_INTERNAL_QSCINTILLA)
    LINK_DIRECTORIES(./qscintilla2)
ENDIF (WANT_INTERNAL_QSCINTILLA)


SET(TORA_LOKI_LIB "tora_loki_lib")
IF (WANT_INTERNAL_QSCINTILLA)
    SET (TORA_QSCINTILLA_LIB tora_qscintilla2_lib)
ENDIF (WANT_INTERNAL_QSCINTILLA)


IF (WIN32)
    SET (WINDOWS_LIB "windows_lib")
ENDIF (WIN32)


INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${QT_INCLUDES}
  ${ORACLE_INCLUDES}
  ${CMAKE_CURRENT_SOURCE_DIR}/loki/include
  ${CMAKE_CURRENT_SOURCE_DIR}/otl
)
IF (WANT_INTERNAL_QSCINTILLA)
    INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}/src/qscintilla2/Qt4
        ${CMAKE_SOURCE_DIR}/src/qscintilla2/Qt4/Qci
    )
ELSE (WANT_INTERNAL_QSCINTILLA)
    INCLUDE_DIRECTORIES(${QSCINTILLA_INCLUDE_DIR})
ENDIF (WANT_INTERNAL_QSCINTILLA)


ADD_EXECUTABLE(${EXE_NAME}
  ${TORA_MOC_SOURCES}
  ${TORA_SOURCES}
  ${TORA_UI_SOURCES}
  ${TORA_RCC_SRCS}
)

#Set up libraries to link with all found packages
# dynamic
TARGET_LINK_LIBRARIES(${EXE_NAME}
  ${QT_LIBRARIES}
  ${ORACLE_LIBRARIES}
  ${LIBXML2_LIBRARIES}
  ${QSCINTILLA_LIBRARIES}
)
# static
TARGET_LINK_LIBRARIES(${EXE_NAME} ${TORA_LOKI_LIB})
IF (WANT_INTERNAL_QSCINTILLA)
    TARGET_LINK_LIBRARIES(${EXE_NAME} ${TORA_QSCINTILLA_LIB} ${QT_LIBRARIES})
ENDIF (WANT_INTERNAL_QSCINTILLA)
# prevent opening the cmd.exe window
IF (WIN32)
    TARGET_LINK_LIBRARIES(${EXE_NAME} ${WINDOWS_LIB})
ENDIF (WIN32)


IF (WIN32)
    INSTALL(TARGETS ${EXE_NAME} RUNTIME DESTINATION .)
ELSE (WIN32)
    INSTALL(TARGETS ${EXE_NAME} RUNTIME DESTINATION bin LIBRARY DESTINATION lib)
ENDIF (WIN32)

