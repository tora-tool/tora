
# require version or better
AUTOMAKE_OPTIONS = 1.6.1

bin_PROGRAMS=tora

tora_SOURCES=main.cpp aclocal.m4 \
	toabout.cpp toabout.h \
	toaboutui.cpp toaboutui.h \
	tobackground.cpp tobackground.h \
	toconnection.cpp toconnection.h \
	todatabasesettingui.cpp todatabasesettingui.h \
	toeditwidget.cpp toeditwidget.h \
	toglobalsetting.cpp toglobalsetting.h \
	toglobalsettingui.cpp toglobalsettingui.h \
	tohelp.cpp tohelp.h \
	tohelpaddfileui.cpp tohelpaddfileui.h \
	tohelpsetupui.cpp tohelpsetupui.h \
	tohelpbrowser.h \
	tohighlightedtext.cpp tohighlightedtext.h \
	tohtml.cpp tohtml.h \
	tomain.cpp tomain.h \
	tomarkedtext.cpp tomarkedtext.h \
	tomemoeditor.cpp tomemoeditor.h \
	tomessageui.cpp tomessageui.h \
	tonewconnection.cpp tonewconnection.h \
	tonewconnectionui.cpp tonewconnectionui.h \
	tonoblockquery.cpp tonoblockquery.h \
	toparamget.cpp toparamget.h \
	topreferences.cpp topreferences.h \
	topreferencesui.cpp topreferencesui.h \
	toresult.cpp toresult.h \
	toresultfield.cpp toresultfield.h \
	toresultitem.cpp toresultitem.h \
	toresultlistformatui.cpp toresultlistformatui.h \
	toresultstats.cpp toresultstats.h \
	toresultview.cpp toresultview.h \
	tosearchreplace.cpp tosearchreplace.h \
	tosearchreplaceui.cpp tosearchreplaceui.h \
	tosmtp.cpp tosmtp.h \
	tosql.cpp tosql.h \
	tosqlparse.cpp tosqlparse.h \
	tosyntaxsetup.cpp tosyntaxsetup.h \
	tosyntaxsetupui.cpp tosyntaxsetupui.h \
	totabwidget.cpp totabwidget.h \
	totemplateprovider.cpp \
	totextview.cpp totextview.h \
	tothread.cpp tothread.h \
	totool.cpp totool.h \
	totoolsettingui.cpp totoolsettingui.h \
	utils.cpp utils.h

EXTRA_DIST = debian \
	help \
	icons \
	otl \
	qtlegacy \
	rpm \
	templates \
	windows

# this is a hack (but it *is* mentioned in the automake manual, of all
# places). EXTRA_DIST includes all files, even things like CVS
# folders. so we delete them here. but specifying all the files in
# those subdirs would suck more because many of them are generated
# (help api for one).
dist-hook:
	rm -rf `find $(distdir) -name CVS`

# mrj: this sorta sucks. there's a lot of generated source files...
# the old build system distributed the .depends folder in the source
# tarball, which is the old way of handling the dependencies.  that's
# not fully portable. using BUILT_SOURCES is the (relatively speaking)
# new work around. this is mainly why i've required automake >= 1.6.

# files listed in BUILT_SOURCES are built before anything else.
BUILT_SOURCES = \
	qtlegacy/qtmultilineedit.moc \
	qtlegacy/qttableview.moc \
	toabout.moc \
	toaboutui.moc \
	toalert.moc \
	toanalyze.moc \
	tobackup.moc \
	tobarchart.moc \
	tobrowser.moc \
	tobrowserconstraint.moc \
	tobrowserconstraintui.moc \
	tobrowserfilterui.moc \
	tobrowserindex.moc \
	tobrowserindexui.moc \
	tobrowsertable.moc \
	tobrowsertableui.moc \
	tochangeconnection.moc \
	tochartalarmui.moc \
	tochartmanager.moc \
	tochartsetupui.moc \
	tocurrent.moc \
	todatabasesettingui.moc \
	todatatype.moc \
	todebug.moc \
	todebugchangeui.moc \
	todebugwatch.moc \
	toeditextensiongotoui.moc \
	toeditextensions.moc \
	toeditextensionsetupui.moc \
	tofilesize.moc \
	toglobalsettingui.moc \
	tohelp.moc \
	tohelpbrowser.moc \
	tohelpaddfileui.moc \
	tohelpbrowser.moc \
	tohelpsetupui.moc \
	tohighlightedtext.moc \
	toinvalid.moc \
	tolegendchart.moc \
	tolinechart.moc \
	tolinechartsetupui.moc \
	tomain.moc \
	tomainwindow.kde.moc \
	tomainwindow.moc \
	tomarkedtext.2.moc \
	tomarkedtext.3.moc \
	tomarkedtext.moc \
	tomemoeditor.moc \
	tomessageui.moc \
	tonewconnection.moc \
	tonewconnectionui.moc \
	tooraclesettingui.moc \
	tooutput.moc \
	toparamget.moc \
	topiechart.moc \
	topreferences.moc \
	topreferencesui.moc \
	toprofiler.moc \
	toproject.moc \
	toresult.moc \
	toresultbar.moc \
	toresultcols.moc \
	toresultcombo.moc \
	toresultconstraint.moc \
	toresultcontent.moc \
	toresultcontentfilterui.moc \
	toresultdepend.moc \
	toresultextract.moc \
	toresultfield.moc \
	toresultindexes.moc \
	toresultitem.moc \
	toresultlabel.moc \
	toresultline.moc \
	toresultlistformatui.moc \
	toresultlock.moc \
	toresultlong.moc \
	toresultparam.moc \
	toresultpie.moc \
	toresultplan.moc \
	toresultreferences.moc \
	toresultstats.moc \
	toresultstorage.moc \
	toresultview.moc \
	torollback.moc \
	torollbackdialogui.moc \
	toscript.moc \
	toscriptui.moc \
	tosearchreplace.moc \
	tosearchreplaceui.moc \
	tosecurity.moc \
	tosecurityquotaui.moc \
	tosecurityroleui.moc \
	tosecurityuserui.moc \
	tosession.moc \
	tosgastatement.moc \
	tosgatrace.moc \
	tosmtp.moc \
	tosqledit.moc \
	tostorage.moc \
	tostoragedatafileui.moc \
	tostoragedefinitionui.moc \
	tostoragedialogui.moc \
	tostorageprefsui.moc \
	tostoragetablespaceui.moc \
	tosyntaxsetup.moc \
	tosyntaxsetupui.moc \
	totableselect.moc \
	totabwidget.moc \
	totemplate.moc \
	totemplateaddfileui.moc \
	totemplateeditui.moc \
	totemplatesetupui.moc \
	totemporary.moc \
	totool.moc \
	totoolsettingui.moc \
	totuning.moc \
	totuningoverviewui.moc \
	totuningsettingui.moc \
	tovisualize.moc \
	towaitevents.moc \
	toworksheet.moc \
	toworksheetsetupui.moc \
	toworksheetstatistic.moc \
	utils.moc \
	toaboutui.cpp \
	tobrowserconstraintui.cpp \
	tobrowserfilterui.cpp \
	tobrowserindexui.cpp \
	tobrowsertableui.cpp \
	tochartalarmui.cpp \
	tochartsetupui.cpp \
	todatabasesettingui.cpp \
	todebugchangeui.cpp \
	todebugwatch.cpp \
	toaboutui.h \
	tobrowserconstraintui.h \
	tobrowserfilterui.h \
	tobrowserindexui.h \
	tobrowsertableui.h \
	tochartalarmui.h \
	tochartsetupui.h \
	todatabasesettingui.h \
	todebugchangeui.h \
	todebugwatch.h \
	toeditextensiongotoui.h \
	toeditextensionsetupui.h \
	toglobalsettingui.h \
	tohelpaddfileui.h \
	tohelpsetupui.h \
	tolinechartsetupui.h \
	tomessageui.h \
	tonewconnectionui.h \
	tooraclesettingui.h \
	topreferencesui.h \
	toresultcontentfilterui.h \
	toresultlistformatui.h \
	torollbackdialogui.h \
	toscriptui.h \
	tosearchreplaceui.h \
	tosecurityquotaui.h \
	tosecurityroleui.h \
	tosecurityuserui.h \
	tostoragedatafileui.h \
	tostoragedefinitionui.h \
	tostoragedialogui.h \
	tostorageprefsui.h \
	tostoragetablespaceui.h \
	tosyntaxsetupui.h \
	totemplateaddfileui.h \
	totemplateeditui.h \
	totemplatesetupui.h \
	totoolsettingui.h \
	totuningoverviewui.h \
	totuningsettingui.h \
	toworksheetsetupui.h \
	toaboutui.cpp \
	tobrowserconstraintui.cpp \
	tobrowserfilterui.cpp \
	tobrowserindexui.cpp \
	tobrowsertableui.cpp \
	tochartalarmui.cpp \
	tochartsetupui.cpp \
	todatabasesettingui.cpp \
	todebugchangeui.cpp \
	todebugwatch.cpp \
	toeditextensiongotoui.cpp \
	toeditextensionsetupui.cpp \
	toglobalsettingui.cpp \
	tohelpaddfileui.cpp \
	tohelpsetupui.cpp \
	tolinechartsetupui.cpp \
	tomessageui.cpp \
	tonewconnectionui.cpp \
	tooraclesettingui.cpp \
	topreferencesui.cpp \
	toresultcontentfilterui.cpp \
	toresultlistformatui.cpp \
	torollbackdialogui.cpp \
	toscriptui.cpp \
	tosearchreplaceui.cpp \
	tosecurityquotaui.cpp \
	tosecurityroleui.cpp \
	tosecurityuserui.cpp \
	tostoragedatafileui.cpp \
	tostoragedefinitionui.cpp \
	tostoragedialogui.cpp \
	tostorageprefsui.cpp \
	tostoragetablespaceui.cpp \
	tosyntaxsetupui.cpp \
	totemplateaddfileui.cpp \
	totemplateeditui.cpp \
	totemplatesetupui.cpp \
	totoolsettingui.cpp \
	totuningoverviewui.cpp \
	totuningsettingui.cpp \
	toworksheetsetupui.cpp

# have_oracle conditional. these libraries are not built if oracle is
# not present.
if HAVE_ORACLE
OPTLIB_ORA = tooracleconnection.la tooraclepreload.la
else
OPTlIB_ORA =
endif

# plugins
lib_LTLIBRARIES = toalert.la \
	toanalyze.la \
	tobackup.la \
	tobrowser.la \
	tochart.la \
	tocurrent.la \
	todebug.la \
	toeditextensions.la \
	toextract.la \
	toinvalid.la \
	tooutput.la \
	topassword.la \
	toprofiler.la \
	toproject.la \
	toqsqlconnection.la \
	toresult.la \
	torollback.la \
	toscript.la \
	tosecurity.la \
	tosession.la \
	tosgatrace.la \
	tosqledit.la \
	tostorage.la \
	tostoragedefinition.la \
	totemplate.la \
	totemporary.la \
	totuning.la \
	towidget.la \
	toworksheet.la \
	$(OPTLIB_ORA)

METASOURCES = AUTO

# these are for all targets.
#LDADD = $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE) $(LIBSOCKET)
INCLUDES = @KDE_INCLUDES@ @QT_INCLUDES@
# these aren't autoconf'ed yet:
CPPFLAGS  = @PCRE_CFLAGS@ -DDEFAULT_PLUGIN_DIR=\"${exec_prefix}/lib\"
LDFLAGS = @PCRE_LIBS@ \
	@KDE_LDFLAGS@ \
	@X_LDFLAGS@ \
	@QT_LDFLAGS@ \
	@LIB_KPARTS@ \
	@LIB_KDEPRINT@ \
	@LIB_KDECORE@ \
	@LIB_KDEUI@ \
	@LIB_KIO@ \
	@LIB_KFILE@ \
	@LIB_DCOP@ \
	@LIB_KHTML@ \
	-lstdc++

# just tora. must --export-dynamic on linux in order to dlopen().
tora_LDFLAGS = -Wl,-export-dynamic

#  uncomment the following if tora requires the math library
#tora_LDADD=-lm

#EXTRA_DIST=tora.lsm.in NAME.texinfo tora.spec.in

PFLAGS = -module -shared -version-info 1:0:0

toalert_la_LDFLAGS = $(PFLAGS)
toalert_la_SOURCES = toalert.cpp toalert.h

toanalyze_la_LDFLAGS = $(PFLAGS)
toanalyze_la_SOURCES = toanalyze.cpp toanalyze.h \
	toworksheetstatistic.cpp toworksheetstatistic.h

tobackup_la_LDFLAGS = $(PFLAGS)
tobackup_la_SOURCES = tobackup.cpp tobackup.h

tobrowser_la_LDFLAGS = $(PFLAGS)
tobrowser_la_SOURCES = tobrowser.cpp tobrowser.h \
	tobrowserconstraint.cpp tobrowserconstraint.h \
	tobrowserconstraintui.cpp tobrowserconstraintui.h \
	tobrowserfilterui.cpp tobrowserfilterui.h \
	tobrowserindex.cpp tobrowserindex.h \
	tobrowserindexui.cpp tobrowserindexui.h \
	tobrowsertable.cpp tobrowsertable.h \
	tobrowsertableui.cpp tobrowsertableui.h

tochart_la_LDFLAGS = $(PFLAGS)
tochart_la_SOURCES = tobarchart.cpp tobarchart.h \
	tochartalarmui.cpp tochartalarmui.h \
	tochartmanager.cpp tochartmanager.h \
	tochartsetupui.cpp tochartsetupui.h \
	tolegendchart.cpp tolegendchart.h \
	tolinechart.cpp tolinechart.h \
	tolinechartsetupui.cpp tolinechartsetupui.h \
	topiechart.cpp topiechart.h \
	toresultbar.cpp toresultbar.h \
	toresultline.cpp toresultline.h \
	toresultpie.cpp toresultpie.h

tocurrent_la_LDFLAGS = $(PFLAGS)
tocurrent_la_SOURCES = tocurrent.cpp tocurrent.h

todebug_la_LDFLAGS = $(PFLAGS)
todebug_la_SOURCES = todebug.cpp todebug.h \
	todebugwatch.cpp todebugwatch.h \
	todebugtext.cpp todebugtext.h \
	todebugchangeui.cpp todebugchangeui.h

toeditextensions_la_LDFLAGS = $(PFLAGS)
toeditextensions_la_SOURCES = toeditextensions.cpp toeditextensions.h \
	toeditextensiongotoui.cpp toeditextensiongotoui.h \
	toeditextensionsetupui.cpp toeditextensionsetupui.h

toextract_la_LDFLAGS = $(PFLAGS)
toextract_la_SOURCES = todatatype.cpp todatatype.h \
	toextract.cpp toextract.h \
	toreport.cpp toreport.h \
	toresultconstraint.cpp toresultconstraint.h \
	toresultextract.cpp toresultextract.h

toinvalid_la_LDFLAGS = $(PFLAGS)
toinvalid_la_SOURCES = toinvalid.cpp toinvalid.h

if HAVE_ORACLE
tooracleconnection_la_CXXFLAGS  = $(ORACLE_CXXFLAGS)
tooracleconnection_la_LDFLAGS = $(PFLAGS) $(ORACLE_LDFLAGS)
tooracleconnection_la_SOURCES =	tooracleconnection.cpp \
	tooracleextract.cpp \
	tooraclesettingui.cpp tooraclesettingui.h

tooraclepreload_la_CXXFLAGS  = $(ORACLE_CXXFLAGS)
tooraclepreload_la_LDFLAGS = $(PFLAGS) $(ORACLE_LDFLAGS)
tooraclepreload_la_SOURCES = tooraclepreload.cpp
endif

tooutput_la_LDFLAGS = $(PFLAGS)
tooutput_la_SOURCES = tooutput.cpp tooutput.h

topassword_la_LDFLAGS = $(PFLAGS)
topassword_la_SOURCES =	topassword.cpp

toprofiler_la_LDFLAGS = $(PFLAGS)
toprofiler_la_SOURCES = toprofiler.cpp toprofiler.h

toproject_la_LDFLAGS = $(PFLAGS)
toproject_la_SOURCES = toproject.cpp toproject.h

toqsqlconnection_la_LDFLAGS = $(PFLAGS)
toqsqlconnection_la_SOURCES = toqsqlconnection.cpp

toresult_la_LDFLAGS = $(PFLAGS)
toresult_la_SOURCES = \
	toresultcols.cpp toresultcols.h \
	toresultcombo.cpp toresultcombo.h \
	toresultcontent.cpp toresultcontent.h \
	toresultcontentfilterui.cpp toresultcontentfilterui.h \
	toresultdepend.cpp toresultdepend.h \
	toresultindexes.cpp toresultindexes.h \
	toresultlabel.cpp toresultlabel.h \
	toresultlock.cpp toresultlock.h \
	toresultlong.cpp toresultlong.h \
	toresultplan.cpp toresultplan.h \
	toresultparam.cpp toresultparam.h \
	toresultreferences.cpp toresultreferences.h \
	toresultstorage.cpp toresultstorage.h

torollback_la_LDFLAGS = $(PFLAGS)
torollback_la_SOURCES =	torollback.cpp torollback.h \
	torollbackdialogui.cpp torollbackdialogui.h

toscript_la_LDFLAGS = $(PFLAGS)
toscript_la_SOURCES = toscript.cpp toscript.h \
	toscriptui.cpp toscriptui.h

tosecurity_la_LDFLAGS = $(PFLAGS)
tosecurity_la_SOURCES = tosecurity.cpp tosecurity.h \
	tosecurityquotaui.cpp tosecurityquotaui.h \
	tosecurityroleui.cpp tosecurityroleui.h \
	tosecurityuserui.cpp tosecurityuserui.h

tosession_la_LDFLAGS = $(PFLAGS)
tosession_la_SOURCES = tosession.cpp tosession.h

tosgatrace_la_LDFLAGS = $(PFLAGS)
tosgatrace_la_SOURCES = tosgatrace.cpp tosgatrace.h

tosqledit_la_LDFLAGS = $(PFLAGS)
tosqledit_la_SOURCES = tosqledit.cpp tosqledit.h

tostorage_la_LDFLAGS = $(PFLAGS)
tostorage_la_SOURCES = tostorage.cpp tostorage.h \
	tostoragedatafileui.cpp tostoragedatafileui.h \
	tostoragedialogui.cpp tostoragedialogui.h \
	tostorageprefsui.cpp tostorageprefsui.h \
	tostoragetablespaceui.cpp tostoragetablespaceui.h

tostoragedefinition_la_LDFLAGS = $(PFLAGS)
tostoragedefinition_la_SOURCES = tostoragedefinition.cpp tostoragedefinition.h \
	tostoragedefinitionui.cpp tostoragedefinitionui.h

totemplate_la_LDFLAGS = $(PFLAGS)
totemplate_la_SOURCES = totemplate.cpp totemplate.h \
	totemplateaddfileui.cpp totemplateaddfileui.h \
	totemplateeditui.cpp totemplateeditui.h \
	totemplatesetupui.cpp totemplatesetupui.h

totemporary_la_LDFLAGS = $(PFLAGS)
totemporary_la_SOURCES = totemporary.cpp totemporary.h

totuning_la_LDFLAGS = $(PFLAGS)
totuning_la_SOURCES = totuning.cpp totuning.h \
	totuningoverviewui.cpp totuningoverviewui.h \
	totuningsettingui.cpp totuningsettingui.h

towidget_la_LDFLAGS = $(PFLAGS)
towidget_la_SOURCES = tochangeconnection.cpp tochangeconnection.h \
	tofilesize.cpp tofilesize.h \
	tosgastatement.cpp tosgastatement.h \
	totableselect.cpp totableselect.h \
	tovisualize.cpp tovisualize.h \
	towaitevents.cpp towaitevents.h

toworksheet_la_LDFLAGS = $(PFLAGS)
toworksheet_la_SOURCES = toworksheet.cpp toworksheet.h \
	toworksheetsetupui.cpp toworksheetsetupui.h

%.cpp %.h: %.ui
	echo Generating $(<:%.ui=%.cpp) \& $(<:%.ui=%.h)
	@UIC@ -o $(<:%.ui=%.h) $<
	@UIC@ -i $(<:%.ui=%.h) -o  $(<:%.ui=%.cpp) $<

%.moc: %.h
	@MOC@ -o $@ $<

clean:
	rm -f *.moc
	for file in `ls *.ui`; do \
		rm -f `echo $$file | sed 's/\.ui/\.cpp/g'`; \
		rm -f `echo $$file | sed 's/\.ui/\.h/g'`; \
	done
