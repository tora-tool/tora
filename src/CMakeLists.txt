INCLUDE(ToraCompat)
INCLUDE(PCHSupport_26)

#Subdirectories
# ADD_SUBDIRECTORY(i18n)
ADD_SUBDIRECTORY(loki)
IF (WIN32)
    ADD_SUBDIRECTORY(windows)
ENDIF (WIN32)

IF (USE_TROTL)
  ADD_SUBDIRECTORY(trotl)
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/trotl/src)
  LINK_DIRECTORIES(./trotl)
  SET (TROTL_LIB "trotl")
ENDIF (USE_TROTL)

# Default: qsci2 as 3rd party lib!
IF (WANT_INTERNAL_QSCINTILLA)
    ADD_SUBDIRECTORY(qscintilla2)
ENDIF(WANT_INTERNAL_QSCINTILLA)


# only toad bindings are here. All others qm files will blow the binary too much
SET (TORA_I18N_TS
    i18n/tora_toad.ts
)

SET(TORA_RCC_FILES tora.qrc)

SET(TORA_UI_SRC
    toaboutui.ui
    tobrowserconstraintui.ui
    tobrowserfilterui.ui
    tobrowserindexui.ui
    tobrowsertableui.ui
    toconnectionimportui.ui
    todatabasesettingui.ui
    todebugchangeui.ui
    todebugwatch.ui
    todescribeui.ui
    todroptablespaceui.ui
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
#    toschedulerui.ui
    toscriptui.ui
    toscriptschemawidgetui.ui
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
    # shortcuteditor
    shortcuteditor/shortcuteditordialog.ui
    connectioncolorsdialogui.ui
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
    tobackgroundlabel.h
    tobrowserconstraint.h
    tobrowser.h
    toresultgrants.h
    tobrowserindex.h
    tobrowsertable.h
    tobrowserbasewidget.h
    tobrowseraccesswidget.h
    tobrowsertablewidget.h
    tobrowserviewwidget.h
    tobrowserindexwidget.h
    tobrowserschemawidget.h
    tobrowsersequencewidget.h
    tobrowsersynonymwidget.h
    tobrowsercodewidget.h
    tobrowsertriggerwidget.h
    tobrowserdblinkswidget.h
#     tobrowserschemabasewidget.h
#     toconf.h
#     toconfiguration.h
    toconnection.h
    toconnectionimport.h
    toconnectionmodel.h
    toconnectionpool.h
    todocklet.h
    todockbar.h
    topushbutton.h
    torunnable.h
    tonoblockquery.h
    tocurrent.h
    todatatype.h
    todebug.h
    todebugtext.h
    todescribe.h
#     todefaultkeywords.h
    toeditextensions.h
#     toeditwidget.h
#     toextract.h
    toeventquery.h
    toeventquerytask.h
    tofilesize.h
    toglobalsetting.h
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
    tomodeleditor.h
    tomessage.h
#     tomysqlkeywords.h
    tonewconnection.h
#     tonoblockquery.h
    tooutput.h
    toparamget.h
    topiechart.h
    toplsqleditor.h
    toplsqltext.h
    topreferences.h
    toprofiler.h
    toproject.h
#     toqvalue.h
#     toreport.h
    toresultbar.h
    toresultcolscomment.h
    toresultcols.h
    toresultcombo.h
    toresultschema.h
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
    tocodemodel.h
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
#    toscheduler.h
    toscript.h
    toscriptschemawidget.h
#     toscripttreeitem.h
    toscripttreemodel.h
    tosearchreplace.h
    tosecurity.h
    tosecuritytreemodel.h
#     tosecuritytreeitem.h
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
    tounittest.h
    tovisualize.h
    towaitevents.h
    toworksheet.h
    toworkingwidget.h
    toeditablemenu.h
    toworksheetstatistic.h
#     toworksheettext.h
    toworksheetwidget.h
    utils.h
# shortcuteditor
    shortcuteditor/shortcuteditordialog.h
    shortcuteditor/shortcutmodel.h
# docklets
    docklets/toviewconnections.h
    docklets/toviewdirectory.h
)
IF (ORACLE_FOUND)
    LIST(APPEND TORA_MOC_CLASSES tooraclesetting.h)
ENDIF (ORACLE_FOUND)


SET(TORA_SOURCES
    # include main.cpp for "normal" TOra build, but comment it out and include
    # tosqlparsetest.cpp when running unit tests for sql parser.
    main.cpp
    #tosqlparsetest.cpp
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
    toresultgrants.cpp
    tobrowserindex.cpp
    tobrowsertable.cpp
    tobrowserbasewidget.cpp
    tobrowseraccesswidget.cpp
    tobrowsertablewidget.cpp
    tobrowserviewwidget.cpp
    tobrowserindexwidget.cpp
    tobrowserschemawidget.cpp
    tobrowsersequencewidget.cpp
    tobrowsersynonymwidget.cpp
    tobrowsercodewidget.cpp
    tobrowsertriggerwidget.cpp
    tobrowserdblinkswidget.cpp
#     tobrowserschemabasewidget.cpp
    toconfiguration.cpp
    toconnection.cpp
    toqvalue.cpp
    toconnectionimport.cpp
    toconnectionmodel.cpp
    toconnectionpool.cpp
    todocklet.cpp
    todockbar.cpp
    topushbutton.cpp
    torunnable.cpp
    tocurrent.cpp
    todatatype.cpp
    todebug.cpp
    todebugtext.cpp
    todescribe.cpp
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
    tomodeleditor.cpp
    tomessage.cpp
    tonewconnection.cpp
    tonoblockquery.cpp
#win only?    toocistub.cpp
#    tooracleconnection.cpp
    tooracleextract.cpp
    tooraclepreload.cpp
    tooutput.cpp
    toparamget.cpp
    topassword.cpp
    topiechart.cpp
    toplsqleditor.cpp
    toplsqltext.cpp
    topreferences.cpp
    toprofiler.cpp
    toproject.cpp
    toqsqlconnection.cpp
    toreport.cpp
    toresultbar.cpp
    toresultcolscomment.cpp
    toresultcols.cpp
    toresultschema.cpp
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
    tocodemodel.cpp
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
#    toscheduler.cpp
    toscript.cpp
    toscriptschemawidget.cpp
    toscripttreeitem.cpp
    toscripttreemodel.cpp
    tosearchreplace.cpp
    tosecurity.cpp
    tosecuritytreemodel.cpp
    tosecuritytreeitem.cpp
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
    tounittest.cpp
    tovisualize.cpp
    towaitevents.cpp
    toworksheet.cpp
    toworkingwidget.cpp
    toeditablemenu.cpp
    toworksheetstatistic.cpp
    toworksheettext.cpp
    toworksheetwidget.cpp
    utils.cpp
    #
    migratetool/sqldeveloper.cpp
    migratetool/squirrelsql.cpp
    migratetool/tora3.cpp
    # shortcuteditor
    shortcuteditor/shortcuteditordialog.cpp
    shortcuteditor/shortcutmodel.cpp
    # docklets
    docklets/toviewconnections.cpp
    docklets/toviewdirectory.cpp
    tooracletable.cpp
)

IF(USE_TROTL)
  LIST(APPEND TORA_SOURCES tooracleconnection_trotl.cpp)
ELSE (USE_TROTL)
  LIST(APPEND TORA_SOURCES tooracleconnection.cpp)
ENDIF (USE_TROTL)
    
IF (DB2_FOUND)
    LIST (APPEND TORA_SOURCES db2/todb2connection.cpp
                              db2/db2connectionsub.cpp
                              db2/db2provider.cpp
                              db2/db2query.cpp)
    #db2/qsql_db2.cpp)
ENDIF (DB2_FOUND)
# compiled in icons for windows
IF (WIN32 AND MSVC)
    SET (TORA_SOURCES ${TORA_SOURCES} tora.rc)
ENDIF (WIN32 AND MSVC)
if (WIN32 AND MINGW)
    # resource compilation for mingw
    ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tora_rc.o
                       COMMAND windres.exe -I${CMAKE_CURRENT_SOURCE_DIR}
                              -i${CMAKE_CURRENT_SOURCE_DIR}/tora.rc
                              -o ${CMAKE_CURRENT_BINARY_DIR}/tora_rc.o)
    SET(TORA_SOURCES ${TORA_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/tora_rc.o)
endif (WIN32 AND MINGW)



QT4_ADD_TRANSLATION(TORA_I18N_QM ${TORA_I18N_TS})
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
  ${CMAKE_CURRENT_SOURCE_DIR}/loki/include
  ${CMAKE_CURRENT_SOURCE_DIR}/otl
)

IF (ORACLE_INCLUDES)
    INCLUDE_DIRECTORIES( ${ORACLE_INCLUDES} )
ENDIF (ORACLE_INCLUDES)

IF (POSTGRESQL_INCLUDE_DIR)
    INCLUDE_DIRECTORIES( ${POSTGRESQL_INCLUDE_DIR} )
ENDIF (POSTGRESQL_INCLUDE_DIR)

IF (DB2_INCLUDES)
    INCLUDE_DIRECTORIES( ${DB2_INCLUDES} )
ENDIF (DB2_INCLUDES)

IF (WANT_INTERNAL_QSCINTILLA)
    INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}/src/qscintilla2/Qt4
        ${CMAKE_SOURCE_DIR}/src/qscintilla2/Qt4/Qci
    )
ELSE (WANT_INTERNAL_QSCINTILLA)
    INCLUDE_DIRECTORIES(${QSCINTILLA_INCLUDE_DIR})
ENDIF (WANT_INTERNAL_QSCINTILLA)


# Handle win/mac special stuff - it's left empty for linux/unix
SET (GUI_TYPE)
IF (APPLE AND WANT_BUNDLE)
    SET(GUI_TYPE MACOSX_BUNDLE)
    SET(APPLE_BUNDLE_SOURCES icons/tora.icns)
ENDIF (APPLE AND WANT_BUNDLE)


IF(WIN32)
    IF (MSVC)
        # this is a must to switch off subsystem:console (console window poping up)
        # WinMain wrapper from QT_QTMAIN_LIBRARY is used and liner must be modified
        LINK_LIBRARIES(${QT_QTMAIN_LIBRARY})
        SET (GUI_TYPE WIN32)
        IF (WANT_DEBUG)
                SET (CMAKE_EXE_LINKER_FLAGS "/NODEFAULTLIB:msvcrtd.lib ${CMAKE_EXE_LINKER_FLAGS}")
        ELSE (WANT_DEBUG)
            SET (CMAKE_EXE_LINKER_FLAGS "/NODEFAULTLIB:msvcrt.lib ${CMAKE_EXE_LINKER_FLAGS}")
        ENDIF (WANT_DEBUG)
    ENDIF (MSVC)
ENDIF(WIN32)


ADD_EXECUTABLE(${EXE_NAME} ${GUI_TYPE}
  ${TORA_I18N_QM}
  ${TORA_MOC_SOURCES}
  ${TORA_SOURCES}
  ${TORA_UI_SOURCES}
  ${TORA_RCC_SRCS}
  ${APPLE_BUNDLE_SOURCES}
)


IF(USE_PCH)
ADD_PRECOMPILED_HEADER(${EXE_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/precompiled.h)
ENDIF(USE_PCH)

#Set up libraries to link with all found packages
# dynamic
TARGET_LINK_LIBRARIES(${EXE_NAME}
  ${QT_LIBRARIES}
  ${LIBXML2_LIBRARIES}
  ${QSCINTILLA_LIBRARIES}
)

IF (ORACLE_LIBRARIES)
    TARGET_LINK_LIBRARIES (${EXE_NAME} ${ORACLE_LIBRARIES} )
ENDIF (ORACLE_LIBRARIES)

IF (POSTGRESQL_LIBRARIES)
   TARGET_LINK_LIBRARIES (${EXE_NAME} ${POSTGRESQL_LIBRARIES})
ENDIF (POSTGRESQL_LIBRARIES)

IF (DB2_LIBRARIES)
   TARGET_LINK_LIBRARIES (${EXE_NAME} ${DB2_LIBRARIES})
ENDIF (DB2_LIBRARIES)

IF (USE_TROTL)
  SET_TARGET_PROPERTIES(${EXE_NAME} PROPERTIES COMPILE_FLAGS "-DTROTL_DLL")
  TARGET_LINK_LIBRARIES (${EXE_NAME} ${TROTL_LIB})
ENDIF (USE_TROTL)

# static
TARGET_LINK_LIBRARIES(${EXE_NAME} ${TORA_LOKI_LIB})
IF (WANT_INTERNAL_QSCINTILLA)
    TARGET_LINK_LIBRARIES(${EXE_NAME} ${TORA_QSCINTILLA_LIB} ${QT_LIBRARIES})
ENDIF (WANT_INTERNAL_QSCINTILLA)
# prevent opening the cmd.exe window
IF (WIN32)
    TARGET_LINK_LIBRARIES(${EXE_NAME} ${WINDOWS_LIB})
    # don't use console subsystem
    IF (MINGW)
        TARGET_LINK_LIBRARIES(${EXE_NAME} "-mwindows")
    ENDIF (MINGW)
ENDIF (WIN32)

IF (WIN32)
    INSTALL(TARGETS ${EXE_NAME} RUNTIME DESTINATION .)
ELSE (WIN32)
    IF (NOT WANT_BUNDLE)
        INSTALL(TARGETS ${EXE_NAME} RUNTIME DESTINATION bin LIBRARY DESTINATION lib)
    ELSE (NOT WANT_BUNDLE)

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

        # Warning: This is dangerous to touch. Except you know what are you doing.
        #          I ususaly don't know anything about it. But it's working ;)
 
        INSTALL(TARGETS ${EXE_NAME} BUNDLE DESTINATION ${CMAKE_INSTALL_PREFIX})

      IF (WANT_BUNDLE_STANDALONE)
        # --- If we are on OS X copy all the embedded libraries to the app bundle
        # -------- Function to build OS X Stand Alone Bundles -----------------
        function(MakeOSXBundleApp target binary_dir)
            #-- Set some useful variables
            SET (OSX_MAKE_STANDALONE_BUNDLE_CMAKE_SCRIPT "${binary_dir}/${target}_OSX_MakeStandAloneBundle.cmake")
            SET (OSX_MAKE_STANDALONE_BUNDLE_BASH_SCRIPT "${binary_dir}/${target}_OSX_MakeStandAloneBundle.sh")
            #-- Configure the cmake file and the shell script
            CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/osx_tools/CompleteBundle.cmake.in"
                           "${OSX_MAKE_STANDALONE_BUNDLE_CMAKE_SCRIPT}" @ONLY IMMEDIATE)
            CONFIGURE_FILE("${CMAKE_SOURCE_DIR}/osx_tools/CreateBundle.sh.in"
                           "${OSX_MAKE_STANDALONE_BUNDLE_BASH_SCRIPT}" @ONLY IMMEDIATE)
            #-- Create the installation code
            install(SCRIPT "${OSX_MAKE_STANDALONE_BUNDLE_CMAKE_SCRIPT}")
        endfunction(MakeOSXBundleApp)


        install(DIRECTORY ${QT_PLUGINS_DIR}/
                DESTINATION ${PROJECT_BINARY_DIR}/Contents/Plugins/
                REGEX "(designer|script|phonon_backend|debug|sqlite|ico)" EXCLUDE)

        install(CODE "
            # glob for previously installed qt plugins to include in fixup_bundle
            file(GLOB_RECURSE PluginList
                 ${PROJECT_BINARY_DIR}/Contents/Plugins/*.bundle)
        ")

        MakeOSXBundleApp(${EXE_NAME} ${PROJECT_BINARY_DIR})

        # TODO/FIXME: this is evil! And it should be done automagically...
        set(hack_bundle_pth "${CMAKE_INSTALL_PREFIX}/${EXE_NAME}.app/Contents/MacOS")
        INSTALL(CODE "file(MAKE_DIRECTORY ${hack_bundle_pth}/sqldrivers)")
        INSTALL(CODE "file(MAKE_DIRECTORY ${hack_bundle_pth}/codecs)")
        INSTALL(CODE "file(MAKE_DIRECTORY ${hack_bundle_pth}/accessible)")
        INSTALL(CODE "file(MAKE_DIRECTORY ${hack_bundle_pth}/imageformats)")
        INSTALL(CODE "file(MAKE_DIRECTORY ${hack_bundle_pth}/graphicssystems)")
        function(hack_move_plugins filename targetdir)
            install(CODE "
                message(STATUS \"file:: ${filename} -> targetdir: ${targetdir} ${hack_bundle_pth}\")
            ")
            install(FILES ${hack_bundle_pth}/${filename} DESTINATION ${hack_bundle_pth}/${targetdir})
            install(CODE "file(REMOVE ${hack_bundle_pth}/${filename})")
        endfunction(hack_move_plugins)
        hack_move_plugins("libqcncodecs.bundle" "codecs")
        hack_move_plugins("libqjpcodecs.bundle" "codecs")
        hack_move_plugins("libqtwcodecs.bundle" "codecs")
        hack_move_plugins("libqkrcodecs.bundle" "codecs")
        hack_move_plugins("libqsqlmysql.bundle" "sqldrivers")
        hack_move_plugins("libqsqlodbc.bundle" "sqldrivers")
        hack_move_plugins("libqsqlpsql.bundle" "sqldrivers")
        hack_move_plugins("libqgif.bundle" "imageformats")
        hack_move_plugins("libqjpeg.bundle" "imageformats")
        hack_move_plugins("libqmng.bundle" "imageformats")
        hack_move_plugins("libqsvg.bundle" "imageformats")
        hack_move_plugins("libqtiff.bundle" "imageformats")
        hack_move_plugins("libqtaccessiblewidgets.bundle" "accessible")
        hack_move_plugins("libqtaccessiblecompatwidgets.bundle" "accessible")
        hack_move_plugins("libqglgraphicssystem.bundle" "graphicssystems")
        # end of hack

        #-- Put an empty qt.conf file in the bundle so as to NOT load plugins from the installed Qt.
        INSTALL(CODE "message(STATUS \"${CMAKE_INSTALL_PREFIX}/${EXE_NAME}.app/Contents/Resources/qt.conf\")")
        INSTALL(FILES "${CMAKE_SOURCE_DIR}/osx_tools/qt.conf.in"
                DESTINATION "${CMAKE_INSTALL_PREFIX}/${EXE_NAME}.app/Contents/Resources/"
                RENAME "qt.conf")

        if ("${QT_MAC_USE_COCOA}" STREQUAL "1")
            INSTALL(CODE "message(STATUS \"Copying qt_menu.nib into ${CMAKE_INSTALL_PREFIX}/${EXE_NAME}.app/Contents/Resources\")")
            INSTALL(CODE "file(MAKE_DIRECTORY \"${CMAKE_INSTALL_PREFIX}/${EXE_NAME}.app/Contents/Resources/qt_menu.nib\")")
            INSTALL(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory
                             \"${QT_LIBRARY_DIR}/QtGui.framework/Resources/qt_menu.nib\"
                             \"${CMAKE_INSTALL_PREFIX}/${EXE_NAME}.app/Contents/Resources/qt_menu.nib\")")
        endif()
      ENDIF (WANT_BUNDLE_STANDALONE)

    ENDIF (NOT WANT_BUNDLE)
ENDIF (WIN32)
