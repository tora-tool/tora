# require version or better
AUTOMAKE_OPTIONS = 1.7

bin_PROGRAMS=tora

METASOURCES = AUTO

TRANSLATIONS= \
	i18n/tora_se.ts \
	i18n/tora_fr.ts \
	i18n/tora_it.ts \
	i18n/tora_toad.ts

LOKIDIR_INC=$(top_srcdir)/src/loki/include
LOKIDIR_SRC=$(top_srcdir)/src/loki/src

AM_CPPFLAGS = -I./otl -I$(LOKIDIR_INC) $(QT_INCLUDES) $(LTDLINCL)
AM_CPPFLAGS += $(PCRE_CFLAGS) \
	-DDEFAULT_PLUGIN_DIR=\"$(libdir)\" \
	-DTOVERSION=\"$(PACKAGE_VERSION)\" \
	$(ORACLE_CXXFLAGS) \
	$(QT_DEFINES) \
	$(WIN_CXXFLAGS) \
	$(QSCINTILLA_CXXFLAGS)

AM_LDFLAGS = $(PCRE_LIBS) \
	$(QT_LDFLAGS) \
	$(LIB_DCOP) \
	-lstdc++ \
	$(LIBPTHREAD) \
	$(ORACLE_LDFLAGS) \
	$(WIN_LDFLAGS) \
	$(QSCINTILLA_LDFLAGS)

# fix for annoying automake naming problems.
tora_CXXFLAGS  = $(AM_CXXFLAGS)

tora_LDADD = $(ORACLE_LIBS) $(PQ_LIBS) $(@TORA_LINK_OTHERS@)

if HAVE_ORACLE
EXTRA_ORACLE_MONO_SOURCE = tooracleconnection.cpp tooraclesetting.h \
	tooracleextract.cpp otl/otlv4.h
EXTRA_ORACLE_MONO_SOURCE_MOC = moc_tooraclesetting.cpp
endif

tora_SOURCES=main.cpp \
	tomainwindow.cpp tomainwindow.h \
	toabout.cpp toabout.h \
	tobackground.cpp tobackground.h \
	tobackgroundlabel.cpp tobackgroundlabel.h \
	toconfiguration.cpp toconfiguration.h \
	toconnection.cpp toconnection.h \
	toqvalue.cpp toqvalue.h \
	toconnectionimport.cpp toconnectionimport.h \
	toconnectionpool.cpp toconnectionpool.h \
    todescribe.cpp todescribe.h \
	torunnable.cpp torunnable.h \
	toeditwidget.cpp toeditwidget.h \
	toglobalsetting.cpp toglobalsetting.h \
	tohelp.cpp tohelp.h tohelpsetup.h \
	tohighlightedtext.cpp tohighlightedtext.h \
	tohtml.cpp tohtml.h \
	tolistviewformatter.cpp tolistviewformatter.h \
	tolistviewformattercsv.cpp tolistviewformattercsv.h \
	tolistviewformatterfactory.h \
	tolistviewformatterhtml.cpp tolistviewformatterhtml.h \
	tolistviewformatteridentifier.h \
	tolistviewformattersql.cpp tolistviewformattersql.h \
	tolistviewformattertabdel.cpp tolistviewformattertabdel.h \
	tolistviewformattertext.cpp tolistviewformattertext.h \
	tomain.cpp tomain.h \
	tomarkedtext.cpp tomarkedtext.h \
	tomemoeditor.cpp tomemoeditor.h \
	tomodeleditor.cpp tomodeleditor.h \
	tomessage.cpp tomessage.h \
	tonewconnection.cpp tonewconnection.h \
	tonoblockquery.cpp tonoblockquery.h \
	toeventquery.cpp toeventquery.h \
	toeventquerytask.cpp toeventquerytask.h \
	toparamget.cpp toparamget.h \
	topreferences.cpp topreferences.h \
	toplsqleditor.cpp  toplsqleditor.h \
	toplsqltext.cpp  toplsqltext.h \
	toresult.cpp toresult.h \
	toresultfield.cpp toresultfield.h \
	toresultitem.cpp toresultitem.h \
	toresultstats.cpp toresultstats.h \
	toresultview.cpp toresultview.h \
	toresultlistformat.cpp toresultlistformat.h \
	toresultmodel.cpp toresultmodel.h \
	toresulttableview.cpp toresulttableview.h \
	toresulttableviewedit.cpp toresulttableviewedit.h \
	toresultdata.cpp toresultdata.h \
	toresultdatasingle.cpp toresultdatasingle.h \
	totreewidget.cpp totreewidget.h \
	tosearchreplace.cpp tosearchreplace.h \
	tosmtp.cpp tosmtp.h \
	tosplash.cpp tosplash.h \
	tosql.cpp tosql.h \
	tosqlparse.cpp tosqlparse.h \
	tosyntaxsetup.cpp tosyntaxsetup.h \
	totabwidget.cpp totabwidget.h \
	totemplateprovider.cpp \
	totextview.cpp totextview.h \
	tothread.cpp tothread.h tothread_p.h \
	totimer.cpp totimer.h \
	totool.cpp totool.h \
	utils.cpp utils.h \
	toalert.cpp toalert.h \
	toanalyze.cpp toanalyze.h  \
	tobackup.cpp tobackup.h \
	tobackuptool.cpp tobackuptool.h \
	tobrowser.cpp tobrowser.h  \
	todocklet.cpp todocklet.h  \
	todockbar.cpp todockbar.h  \
	topushbutton.cpp topushbutton.h  \
	docklets/toviewdirectory.cpp docklets/toviewdirectory.h  \
	docklets/toviewconnections.cpp docklets/toviewconnections.h  \
	tobarchart.cpp tobarchart.h  \
	tocurrent.cpp tocurrent.h \
	todebug.cpp todebug.h  \
	toeditextensions.cpp toeditextensions.h  \
	todatatype.cpp todatatype.h  \
	toinvalid.cpp toinvalid.h \
	tooutput.cpp tooutput.h \
	topassword.cpp \
	toprofiler.cpp toprofiler.h \
	toproject.cpp toproject.h \
	toqsqlconnection.cpp \
	torollback.cpp torollback.h  \
	toscript.cpp toscript.h  \
	toscriptschemawidget.cpp toscriptschemawidget.h \
	toscripttreeitem.cpp toscripttreeitem.h \
	tocodemodel.cpp tocodemodel.h \
	toscripttreemodel.cpp toscripttreemodel.h \
	tosecurity.cpp tosecurity.h  \
    tosecuritytreemodel.cpp tosecuritytreemodel.h \
    tosecuritytreeitem.cpp tosecuritytreeitem.h \
	tosession.cpp tosession.h \
	tosgatrace.cpp tosgatrace.h \
	tosqledit.cpp tosqledit.h \
	tostorage.cpp tostorage.h  \
	tostoragedefinition.cpp tostoragedefinition.h  \
	totemplate.cpp totemplate.h  \
	totemporary.cpp totemporary.h \
	totuning.cpp totuning.h  \
	tounittest.cpp tounittest.h \
	tochangeconnection.cpp tochangeconnection.h  \
	toworkingwidget.cpp toworkingwidget.h \
	toworksheet.cpp toworksheet.h  \
	toeditablemenu.cpp toeditablemenu.h  \
	toworksheetwidget.cpp toworksheetwidget.h  \
	toworksheettext.cpp toworksheettext.h  \
	toworksheetstatistic.cpp toworksheetstatistic.h \
	tobrowserconstraint.cpp tobrowserconstraint.h  \
	tobrowserindex.cpp tobrowserindex.h  \
	tobrowsertable.cpp tobrowsertable.h  \
	tochartmanager.cpp tochartmanager.h  \
	tolegendchart.cpp tolegendchart.h  \
	tolinechart.cpp tolinechart.h  \
	topiechart.cpp topiechart.h  \
	toresultbar.cpp toresultbar.h  \
	toresultline.cpp toresultline.h  \
	toresultpie.cpp toresultpie.h \
	todebugtext.cpp todebugtext.h  \
	toextract.cpp toextract.h  \
	toreport.cpp toreport.h  \
	toresultextract.cpp toresultextract.h \
	toresultcols.cpp toresultcols.h  \
	toresultcolscomment.cpp toresultcolscomment.h \
	toresultcombo.cpp toresultcombo.h  \
	toresultschema.cpp toresultschema.h  \
	toresultdepend.cpp toresultdepend.h  \
	toresultlabel.cpp toresultlabel.h  \
	toresultlock.cpp toresultlock.h  \
	toresultlong.cpp toresultlong.h  \
	toresultplan.cpp toresultplan.h  \
	toresultparam.cpp toresultparam.h  \
	toresultstorage.cpp toresultstorage.h \
	tofilesize.cpp tofilesize.h  \
	tosgastatement.cpp tosgastatement.h  \
	totableselect.cpp totableselect.h  \
	tovisualize.cpp tovisualize.h  \
	towaitevents.cpp towaitevents.h \
	toconnectionmodel.cpp toconnectionmodel.h \
    tobrowserbasewidget.cpp tobrowserbasewidget.h \
    tobrowseraccesswidget.cpp tobrowseraccesswidget.h \
    tobrowsertablewidget.cpp tobrowsertablewidget.h \
    tobrowserviewwidget.cpp tobrowserviewwidget.h \
    tobrowserindexwidget.cpp tobrowserindexwidget.h \
    tobrowserschemawidget.cpp tobrowserschemawidget.h \
    tobrowsersequencewidget.cpp tobrowsersequencewidget.h \
    tobrowsersynonymwidget.cpp tobrowsersynonymwidget.h \
    tobrowsercodewidget.cpp tobrowsercodewidget.h \
    tobrowsertriggerwidget.cpp tobrowsertriggerwidget.h \
    tobrowserdblinkswidget.cpp tobrowserdblinkswidget.h \
    toresultgrants.cpp toresultgrants.h \
	migratetool/sqldeveloper.cpp migratetool/sqldeveloper.h \
	migratetool/squirrelsql.cpp migratetool/squirrelsql.h \
	migratetool/tora3.cpp migratetool/tora3.h \
	shortcuteditor/shortcuteditordialog.cpp shortcuteditor/shortcuteditordialog.h \
	shortcuteditor/shortcutmodel.cpp shortcuteditor/shortcutmodel.h \
	$(EXTRA_ORACLE_MONO_SOURCE) \
	$(LOKIDIR_SRC)/SmallObj.cpp \
	$(LOKIDIR_SRC)/Singleton.cpp \
	$(EXTRA_tora_SOURCES) \
	$(BUILT_SOURCES)

UI_FILES = toaboutui.ui \
		tobrowserconstraintui.ui \
		tobrowserfilterui.ui \
		tobrowserindexui.ui \
		tobrowsertableui.ui \
		toconnectionimportui.ui \
        todescribeui.ui \
		tooraclesettingui.ui \
		tostoragetablespaceui.ui \
		tochartalarmui.ui \
		topreferencesui.ui \
		tosyntaxsetupui.ui \
		tochartsetupui.ui \
		toresultcontentfilterui.ui \
		totemplateaddfileui.ui \
		todatabasesettingui.ui \
		toresultlistformatui.ui \
		totemplateeditui.ui \
		todebugchangeui.ui \
		torollbackdialogui.ui \
		totemplatesetupui.ui \
		todebugwatch.ui \
		toscriptui.ui \
		toscriptschemawidgetui.ui \
		totoolsettingui.ui \
		toeditextensiongotoui.ui \
		tosearchreplaceui.ui \
		totuningoverviewui.ui \
		toeditextensionsetupui.ui \
		tosecurityquotaui.ui \
		totuningsettingui.ui \
		toglobalsettingui.ui \
		tosecurityroleui.ui \
		toworksheetsetupui.ui \
		tohelpui.ui \
		tohelpaddfileui.ui \
		tosecurityuserui.ui \
		tohelpsetupui.ui \
		tostoragedatafileui.ui \
		tolinechartsetupui.ui \
		tostoragedefinitionui.ui \
		tomessageui.ui \
		tostoragedialogui.ui \
		tonewconnectionui.ui \
		tostorageprefsui.ui \
		todroptablespaceui.ui \
		toscriptschemawidgetui.ui \
		shortcuteditor/shortcuteditordialog.ui


EXTRA_DIST = icons \
	templates \
	windows \
	COPYRIGHT.h \
	help \
	$(LOKIDIR_INC) \
	$(UI_FILES) \
	CMakeLists.txt \
	loki/CMakeLists.txt \
	Makefile.am \
	toconf.h \
	todefaultkeywords.h \
	toextratranslations.cpp \
	tomysqlkeywords.h \
	toocistub.cpp \
	tooraclepreload.cpp \
	toresultresources.h \
	$(TRANSLATIONS) \
	LICENSE.h \
	tora.qrc

# this is a hack (but it *is* mentioned in the automake manual, of all
# places). EXTRA_DIST includes all files, even things like CVS
# folders. so we delete them here. but specifying all the files in
# those subdirs would suck more because many of them are generated
# (help api for one).
dist-hook:
	rm -rf `find $(distdir) -name .svn`

# mrj: this sorta sucks. there's a lot of generated source files...
# the old build system distributed the .depends folder in the source
# tarball, which is the old way of handling the dependencies.  that's
# not fully portable. using BUILT_SOURCES is the (relatively speaking)
# new work around. this is mainly why i've required automake >= 1.6.

# files listed in BUILT_SOURCES are built before anything else.
BUILT_SOURCES = \
	moc_toabout.cpp \
	moc_toalert.cpp \
	moc_toanalyze.cpp \
	moc_tobackup.cpp \
	moc_tobarchart.cpp \
	moc_tobackgroundlabel.cpp \
	moc_tobrowser.cpp \
	moc_tobrowserconstraint.cpp \
	moc_tobrowserindex.cpp \
	moc_tobrowsertable.cpp \
	moc_tochangeconnection.cpp \
	moc_tochartmanager.cpp \
	moc_toconnection.cpp \
	moc_toconnectionmodel.cpp \
	moc_tocodemodel.cpp \
	moc_toconnectionpool.cpp \
	moc_toconnectionimport.cpp \
	moc_todescribe.cpp \
	moc_torunnable.cpp \
	moc_tonoblockquery.cpp \
	moc_toeventquery.cpp \
	moc_toeventquerytask.cpp \
	moc_tocurrent.cpp \
	moc_todatatype.cpp \
	moc_todebug.cpp \
	moc_todebugtext.cpp \
	moc_toeditextensions.cpp \
	moc_tofilesize.cpp \
	moc_toglobalsetting.cpp \
	moc_tohelp.cpp \
	moc_tohelpsetup.cpp \
	moc_tohighlightedtext.cpp \
	moc_toinvalid.cpp \
	moc_tolegendchart.cpp \
	moc_tolinechart.cpp \
	moc_tomain.cpp \
	moc_tomainwindow.cpp \
	moc_tomarkedtext.cpp \
	moc_tomemoeditor.cpp \
	moc_tomodeleditor.cpp \
	moc_tomessage.cpp \
	moc_tonewconnection.cpp \
	moc_tooutput.cpp \
	moc_toparamget.cpp \
	moc_topiechart.cpp \
	moc_toplsqleditor.cpp \
	moc_toplsqltext.cpp \
	moc_topreferences.cpp \
	moc_toprofiler.cpp \
	moc_toproject.cpp \
	moc_toresult.cpp \
	moc_toresultbar.cpp \
	moc_toresultcols.cpp \
	moc_toresultcolscomment.cpp \
	moc_toresultcombo.cpp \
	moc_toresultschema.cpp \
	moc_toresultdepend.cpp \
	moc_toresultextract.cpp \
	moc_toresultfield.cpp \
	moc_toresultitem.cpp \
	moc_toresultlabel.cpp \
	moc_toresultline.cpp \
	moc_toresultlock.cpp \
	moc_toresultlong.cpp \
	moc_toresultparam.cpp \
	moc_toresultpie.cpp \
	moc_toresultplan.cpp \
	moc_toresultstats.cpp \
	moc_toresultstorage.cpp \
	moc_toresultview.cpp \
	moc_toresultlistformat.cpp \
	moc_toresultmodel.cpp \
	moc_toresulttableview.cpp \
	moc_toresulttableviewedit.cpp \
	moc_toresultdata.cpp \
	moc_toresultdatasingle.cpp \
	moc_totreewidget.cpp \
	moc_torollback.cpp \
	moc_toscript.cpp \
	moc_tosearchreplace.cpp \
	moc_tosecurity.cpp \
    moc_tosecuritytreemodel.cpp \
	moc_tosession.cpp \
	moc_tosgastatement.cpp \
	moc_tosgatrace.cpp \
	moc_tosmtp.cpp \
	moc_tosqledit.cpp \
	moc_tostorage.cpp \
	moc_tosyntaxsetup.cpp \
	moc_totableselect.cpp \
	moc_totabwidget.cpp \
	moc_totemplate.cpp \
	moc_totemporary.cpp \
	moc_tothread_p.cpp \
	moc_totool.cpp \
	moc_totuning.cpp \
	moc_tounittest.cpp \
	moc_tovisualize.cpp \
	moc_towaitevents.cpp \
	moc_toworksheet.cpp \
	moc_toeditablemenu.cpp \
	moc_toworksheetwidget.cpp \
	moc_toworksheetstatistic.cpp \
	moc_utils.cpp \
	moc_toscriptschemawidget.cpp \
	moc_toscripttreemodel.cpp \
	moc_toworkingwidget.cpp \
	shortcuteditor/moc_shortcuteditordialog.cpp \
	shortcuteditor/moc_shortcutmodel.cpp \
	moc_todocklet.cpp \
	moc_todockbar.cpp \
	moc_topushbutton.cpp \
    moc_tobrowserbasewidget.cpp \
    moc_tobrowseraccesswidget.cpp \
    moc_tobrowsertablewidget.cpp \
    moc_tobrowserviewwidget.cpp \
    moc_tobrowserindexwidget.cpp \
    moc_tobrowserschemawidget.cpp \
    moc_tobrowsersequencewidget.cpp \
    moc_tobrowsersynonymwidget.cpp \
    moc_tobrowsercodewidget.cpp \
    moc_tobrowsertriggerwidget.cpp \
    moc_tobrowserdblinkswidget.cpp \
    moc_toresultgrants.cpp \
	docklets/moc_toviewdirectory.cpp \
	docklets/moc_toviewconnections.cpp \
	ui_toaboutui.h \
	ui_todroptablespaceui.h \
	ui_tobrowserconstraintui.h \
	ui_tobrowserfilterui.h \
	ui_tobrowserindexui.h \
	ui_todescribeui.h \
	ui_tobrowsertableui.h \
	ui_toconnectionimportui.h \
	ui_tochartalarmui.h \
	ui_tochartsetupui.h \
	ui_todatabasesettingui.h \
	ui_todebugchangeui.h \
	ui_todebugwatch.h \
	ui_toeditextensiongotoui.h \
	ui_toeditextensionsetupui.h \
	ui_toglobalsettingui.h \
	ui_tohelpaddfileui.h \
	ui_tohelpsetupui.h \
	ui_tolinechartsetupui.h \
	ui_tomessageui.h \
	ui_tonewconnectionui.h \
	ui_tooraclesettingui.h \
	ui_topreferencesui.h \
	ui_toresultcontentfilterui.h \
	ui_toresultlistformatui.h \
	ui_torollbackdialogui.h \
	ui_toscriptui.h \
	ui_tosearchreplaceui.h \
	ui_tosecurityquotaui.h \
	ui_tosecurityroleui.h \
	ui_tosecurityuserui.h \
	ui_tostoragedatafileui.h \
	ui_tostoragedefinitionui.h \
	ui_tostoragedialogui.h \
	ui_tostorageprefsui.h \
	ui_tostoragetablespaceui.h \
	ui_tosyntaxsetupui.h \
	ui_totemplateaddfileui.h \
	ui_totemplateeditui.h \
	ui_totemplatesetupui.h \
	ui_totoolsettingui.h \
	ui_totuningoverviewui.h \
	ui_totuningsettingui.h \
	ui_toworksheetsetupui.h \
	ui_tohelpui.h \
	ui_toscriptschemawidgetui.h \
	shortcuteditor/ui_shortcuteditordialog.h \
	$(EXTRA_ORACLE_MONO_SOURCE_MOC) \
	$(filter %.qm,$(TRANSLATIONS:%.ts=%.qm)) \
	tora_toad.h \
	qrc_tora.cpp

#.ui.h:
#	$(UIC) -o $@ $<
ui_%.h: %.ui
	$(UIC) -o $@ $<

%.cpp: %.h %.ui
	$(UIC) -o $@ $<

moc_%.cpp: %.h
	$(MOC) $< -o $@

qrc_%.cpp: %.qrc
	$(RCC) $< -o $@

#make resource file depend on resources
qrc_tora.cpp: help/*.html help/images/*.png icons/*.xpm

#.h.moc:
#	$(MOC) $(AM_CPPFLAGS) -o $@ $<

.ts.qm:
	@if ! test -d i18n; then mkdir i18n; echo "create i18n"; fi
	$(LRELEASE) $< -qm $@ 

clean-local:
	rm -f moc_*
	for file in `ls *.ui`; do \
		rm -f `echo $$file | sed 's/\.ui/\.cpp/g'`; \
		rm -f `echo $$file | sed 's/\.ui/\.h/g'`; \
	done

tora_toad.h: i18n/tora_toad.qm
	$(top_builddir)/utils/chex/chex $< > $@

tora.pro: Makefile.am
	echo "# Not indended to be used for anything except lupdate" > tora.pro
	echo 'SOURCES=	\ ' >> tora.pro
	echo '	main.cpp \ ' >> tora.pro
	echo '	toextratranslations.cpp \ ' >> tora.pro
	echo "	$(tora_SOURCES)" >> tora.pro
	echo 'TRANSLATIONS=$(TRANSLATIONS)' >> tora.pro

lupdate: tora.pro
	$(LUPDATE) tora.pro

lrelease: tora.pro $(TRANSLATIONS)
	$(LRELEASE) tora.pro

install-data-hook:
	mkdir -p $(DESTDIR)$(libdir)
	cp -f $(top_builddir)/src/i18n/*.qm $(DESTDIR)$(libdir)
