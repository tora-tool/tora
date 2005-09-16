
# require version or better
AUTOMAKE_OPTIONS = 1.6.1

bin_PROGRAMS=tora

METASOURCES = AUTO

TRANSLATIONS= \
	tora_se.ts \
	tora_fr.ts \
	tora_it.ts \
	tora_toad.ts

INCLUDES = @KDE_INCLUDES@ @QT_INCLUDES@ $(LTDLINCL)
AM_CPPFLAGS  = $(PCRE_CFLAGS) -DDEFAULT_PLUGIN_DIR=\"$(libdir)\" -DTOVERSION=\"@VERSION@\" $(ORACLE_CXXFLAGS)
AM_LDFLAGS = $(PCRE_LIBS) \
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
	-lstdc++ \
	@LIBPTHREAD@ \
	$(ORACLE_LDFLAGS)

# fix for annoying automake naming problems.
tora_CXXFLAGS  = $(AM_CXXFLAGS)

tora_LDADD = $(ORACLE_LIBS) $(@TORA_LINK_OTHERS@)

if HAVE_ORACLE
EXTRA_ORACLE_MONO_SOURCE = tooracleconnection.cpp \
	tooracleextract.cpp \
	tooraclesettingui.cpp tooraclesettingui.h
endif

if ENABLE_MONO
EXTRA_tora_SOURCES = toalert.cpp toalert.h \
    toanalyze.cpp toanalyze.h  \
    tobackup.cpp tobackup.h \
    tobrowser.cpp tobrowser.h  \
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
    tosecurity.cpp tosecurity.h  \
    tosession.cpp tosession.h \
    tosgatrace.cpp tosgatrace.h \
    tosqledit.cpp tosqledit.h \
    tostorage.cpp tostorage.h  \
    tostoragedefinition.cpp tostoragedefinition.h  \
    totemplate.cpp totemplate.h  \
    totemporary.cpp totemporary.h \
    totuning.cpp totuning.h  \
    tochangeconnection.cpp tochangeconnection.h  \
    toworksheet.cpp toworksheet.h  \
    toworksheetstatistic.cpp toworksheetstatistic.h \
    tobrowserconstraint.cpp tobrowserconstraint.h  \
    tobrowserconstraintui.cpp tobrowserconstraintui.h  \
    tobrowserfilterui.cpp tobrowserfilterui.h  \
    tobrowserindex.cpp tobrowserindex.h  \
    tobrowserindexui.cpp tobrowserindexui.h  \
    tobrowsertable.cpp tobrowsertable.h  \
    tobrowsertableui.cpp tobrowsertableui.h \
    tochartalarmui.cpp tochartalarmui.h  \
    tochartmanager.cpp tochartmanager.h  \
    tochartsetupui.cpp tochartsetupui.h  \
    tolegendchart.cpp tolegendchart.h  \
    tolinechart.cpp tolinechart.h  \
    tolinechartsetupui.cpp tolinechartsetupui.h  \
    topiechart.cpp topiechart.h  \
    toresultbar.cpp toresultbar.h  \
    toresultline.cpp toresultline.h  \
    toresultpie.cpp toresultpie.h \
    todebugwatch.cpp todebugwatch.h  \
    todebugtext.cpp todebugtext.h  \
    todebugchangeui.cpp todebugchangeui.h \
    toeditextensiongotoui.cpp toeditextensiongotoui.h  \
    toeditextensionsetupui.cpp toeditextensionsetupui.h \
    toextract.cpp toextract.h  \
    toreport.cpp toreport.h  \
    toresultconstraint.cpp toresultconstraint.h  \
    toresultextract.cpp toresultextract.h \
    toresultcols.cpp toresultcols.h  \
    toresultcombo.cpp toresultcombo.h  \
    toresultcontent.cpp toresultcontent.h  \
    toresultcontentfilterui.cpp toresultcontentfilterui.h  \
    toresultdepend.cpp toresultdepend.h  \
    toresultindexes.cpp toresultindexes.h  \
    toresultlabel.cpp toresultlabel.h  \
    toresultlock.cpp toresultlock.h  \
    toresultlong.cpp toresultlong.h  \
    toresultplan.cpp toresultplan.h  \
    toresultparam.cpp toresultparam.h  \
    toresultreferences.cpp toresultreferences.h  \
    toresultstorage.cpp toresultstorage.h \
    torollbackdialogui.cpp torollbackdialogui.h \
    toscriptui.cpp toscriptui.h \
    tosecurityquotaui.cpp tosecurityquotaui.h  \
    tosecurityroleui.cpp tosecurityroleui.h  \
    tosecurityuserui.cpp tosecurityuserui.h \
    tostoragedatafileui.cpp tostoragedatafileui.h  \
    tostoragedialogui.cpp tostoragedialogui.h  \
    tostorageprefsui.cpp tostorageprefsui.h  \
    tostoragetablespaceui.cpp tostoragetablespaceui.h \
    tostoragedefinitionui.cpp tostoragedefinitionui.h \
    totemplateaddfileui.cpp totemplateaddfileui.h  \
    totemplateeditui.cpp totemplateeditui.h  \
    totemplatesetupui.cpp totemplatesetupui.h \
    totuningoverviewui.cpp totuningoverviewui.h  \
    totuningsettingui.cpp totuningsettingui.h \
    tofilesize.cpp tofilesize.h  \
    tosgastatement.cpp tosgastatement.h  \
    totableselect.cpp totableselect.h  \
    tovisualize.cpp tovisualize.h  \
    towaitevents.cpp towaitevents.h \
    toworksheetsetupui.cpp toworksheetsetupui.h \
    $(EXTRA_ORACLE_MONO_SOURCE)
endif

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
	utils.cpp utils.h \
	$(EXTRA_tora_SOURCES)

EXTRA_DIST = debian \
	help \
	icons \
	qtlegacy \
	rpm \
	templates \
	windows \
	*.ui \
	BUGS \
	COPYRIGHT \
	Makefile.* \
	README* \
	LICENSE.h \
	chex.pl \
	qtkde.m4 \
	toconf.h \
	todefaultkeywords.h \
	toextratranslations.cpp \
	tomainwindow.h \
	tomainwindow.kde.h \
	tomysqlkeywords.h \
	toocistub.cpp \
	tooraclepreload.cpp \
	toqvalue.h \
	toresultresources.h \
	$(TRANSLATIONS) \
	autogen.sh \
	otlv4.h \
	LICENSE.h

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
	toworksheetsetupui.cpp \
	tora_toad.h \
	tora_toad.qm \
	$(filter %.qm,$(TRANSLATIONS:%.ts=%.qm))

# have_oracle conditional. these libraries are not built if oracle is
# not present.
if HAVE_ORACLE
OPTLIB_ORA = libtooracleconnection.la
else
OPTlIB_ORA =
endif

plugins = libtoalert.la \
	libtoanalyze.la \
	libtobackup.la \
	libtobrowser.la \
	libtochart.la \
	libtocurrent.la \
	libtodebug.la \
	libtoeditextensions.la \
	libtoextract.la \
	libtoinvalid.la \
	libtooutput.la \
	libtopassword.la \
	libtoprofiler.la \
	libtoproject.la \
	libtoqsqlconnection.la \
	libtoresult.la \
	libtorollback.la \
	libtoscript.la \
	libtosecurity.la \
	libtosession.la \
	libtosgatrace.la \
	libtosqledit.la \
	libtostorage.la \
	libtostoragedefinition.la \
	libtotemplate.la \
	libtotemporary.la \
	libtotuning.la \
	libtowidget.la \
	libtoworksheet.la \
	$(OPTLIB_ORA)

if !ENABLE_MONO
lib_LTLIBRARIES = $(plugins)
endif

PFLAGS = -avoid-version -rpath $(libdir) -module -shared

libtoalert_la_LDFLAGS = $(PFLAGS)
libtoalert_la_SOURCES = toalert.cpp toalert.h

libtoanalyze_la_LDFLAGS = $(PFLAGS)
libtoanalyze_la_SOURCES = toanalyze.cpp toanalyze.h \
	toworksheetstatistic.cpp toworksheetstatistic.h

libtobackup_la_LDFLAGS = $(PFLAGS)
libtobackup_la_SOURCES = tobackup.cpp tobackup.h

libtobrowser_la_LDFLAGS = $(PFLAGS)
libtobrowser_la_SOURCES = tobrowser.cpp tobrowser.h \
	tobrowserconstraint.cpp tobrowserconstraint.h \
	tobrowserconstraintui.cpp tobrowserconstraintui.h \
	tobrowserfilterui.cpp tobrowserfilterui.h \
	tobrowserindex.cpp tobrowserindex.h \
	tobrowserindexui.cpp tobrowserindexui.h \
	tobrowsertable.cpp tobrowsertable.h \
	tobrowsertableui.cpp tobrowsertableui.h

libtochart_la_LDFLAGS = $(PFLAGS)
libtochart_la_SOURCES = tobarchart.cpp tobarchart.h \
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

libtocurrent_la_LDFLAGS = $(PFLAGS)
libtocurrent_la_SOURCES = tocurrent.cpp tocurrent.h

libtodebug_la_LDFLAGS = $(PFLAGS)
libtodebug_la_SOURCES = todebug.cpp todebug.h \
	todebugwatch.cpp todebugwatch.h \
	todebugtext.cpp todebugtext.h \
	todebugchangeui.cpp todebugchangeui.h

libtoeditextensions_la_LDFLAGS = $(PFLAGS)
libtoeditextensions_la_SOURCES = toeditextensions.cpp toeditextensions.h \
	toeditextensiongotoui.cpp toeditextensiongotoui.h \
	toeditextensionsetupui.cpp toeditextensionsetupui.h

libtoextract_la_LDFLAGS = $(PFLAGS)
libtoextract_la_SOURCES = todatatype.cpp todatatype.h \
	toextract.cpp toextract.h \
	toreport.cpp toreport.h \
	toresultconstraint.cpp toresultconstraint.h \
	toresultextract.cpp toresultextract.h

libtoinvalid_la_LDFLAGS = $(PFLAGS)
libtoinvalid_la_SOURCES = toinvalid.cpp toinvalid.h

if HAVE_ORACLE
libtooracleconnection_la_LIBADD    = $(ORACLE_LIBS)
libtooracleconnection_la_CXXFLAGS  = $(ORACLE_CXXFLAGS) -DOTL_FUNC_THROW_SPEC_ON
libtooracleconnection_la_LDFLAGS   = $(PFLAGS) $(ORACLE_LDFLAGS)
libtooracleconnection_la_SOURCES   =	tooracleconnection.cpp \
	tooracleextract.cpp \
	tooraclesettingui.cpp tooraclesettingui.h
endif

libtooutput_la_LDFLAGS = $(PFLAGS)
libtooutput_la_SOURCES = tooutput.cpp tooutput.h

libtopassword_la_LDFLAGS = $(PFLAGS)
libtopassword_la_SOURCES =	topassword.cpp

libtoprofiler_la_LDFLAGS = $(PFLAGS)
libtoprofiler_la_SOURCES = toprofiler.cpp toprofiler.h

libtoproject_la_LDFLAGS = $(PFLAGS)
libtoproject_la_SOURCES = toproject.cpp toproject.h

libtoqsqlconnection_la_LDFLAGS = $(PFLAGS)
libtoqsqlconnection_la_SOURCES = toqsqlconnection.cpp

libtoresult_la_LDFLAGS = $(PFLAGS)
libtoresult_la_SOURCES = \
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

libtorollback_la_LDFLAGS = $(PFLAGS)
libtorollback_la_SOURCES =	torollback.cpp torollback.h \
	torollbackdialogui.cpp torollbackdialogui.h

libtoscript_la_LDFLAGS = $(PFLAGS)
libtoscript_la_SOURCES = toscript.cpp toscript.h \
	toscriptui.cpp toscriptui.h

libtosecurity_la_LDFLAGS = $(PFLAGS)
libtosecurity_la_SOURCES = tosecurity.cpp tosecurity.h \
	tosecurityquotaui.cpp tosecurityquotaui.h \
	tosecurityroleui.cpp tosecurityroleui.h \
	tosecurityuserui.cpp tosecurityuserui.h

libtosession_la_LDFLAGS = $(PFLAGS)
libtosession_la_SOURCES = tosession.cpp tosession.h

libtosgatrace_la_LDFLAGS = $(PFLAGS)
libtosgatrace_la_SOURCES = tosgatrace.cpp tosgatrace.h

libtosqledit_la_LDFLAGS = $(PFLAGS)
libtosqledit_la_SOURCES = tosqledit.cpp tosqledit.h

libtostorage_la_LDFLAGS = $(PFLAGS)
libtostorage_la_SOURCES = tostorage.cpp tostorage.h \
	tostoragedatafileui.cpp tostoragedatafileui.h \
	tostoragedialogui.cpp tostoragedialogui.h \
	tostorageprefsui.cpp tostorageprefsui.h \
	tostoragetablespaceui.cpp tostoragetablespaceui.h

libtostoragedefinition_la_LDFLAGS = $(PFLAGS)
libtostoragedefinition_la_SOURCES = tostoragedefinition.cpp tostoragedefinition.h \
	tostoragedefinitionui.cpp tostoragedefinitionui.h

libtotemplate_la_LDFLAGS = $(PFLAGS)
libtotemplate_la_SOURCES = totemplate.cpp totemplate.h \
	totemplateaddfileui.cpp totemplateaddfileui.h \
	totemplateeditui.cpp totemplateeditui.h \
	totemplatesetupui.cpp totemplatesetupui.h

libtotemporary_la_LDFLAGS = $(PFLAGS)
libtotemporary_la_SOURCES = totemporary.cpp totemporary.h

libtotuning_la_LDFLAGS = $(PFLAGS)
libtotuning_la_SOURCES = totuning.cpp totuning.h \
	totuningoverviewui.cpp totuningoverviewui.h \
	totuningsettingui.cpp totuningsettingui.h

libtowidget_la_LDFLAGS = $(PFLAGS)
libtowidget_la_SOURCES = tochangeconnection.cpp tochangeconnection.h \
	tofilesize.cpp tofilesize.h \
	tosgastatement.cpp tosgastatement.h \
	totableselect.cpp totableselect.h \
	tovisualize.cpp tovisualize.h \
	towaitevents.cpp towaitevents.h

libtoworksheet_la_LDFLAGS = $(PFLAGS)
libtoworksheet_la_SOURCES = toworksheet.cpp toworksheet.h \
	toworksheetsetupui.cpp toworksheetsetupui.h

%.cpp %.h: %.ui
	@UIC@ -o $(<:%.ui=%.h) $<
	@UIC@ -i $(<:%.ui=%.h) -o  $(<:%.ui=%.cpp) $<

%.moc: %.h
	@MOC@ -o $@ $<

%.qm: %.ts
	@LRELEASE@ $<

clean:
	rm -f *.moc
	for file in `ls *.ui`; do \
		rm -f `echo $$file | sed 's/\.ui/\.cpp/g'`; \
		rm -f `echo $$file | sed 's/\.ui/\.h/g'`; \
	done

tora_toad.h: tora_toad.qm
	perl chex.pl < $< > $@

tora.pro: Makefile.am
	echo "# Not indended to be used for anything except lupdate" > tora.pro
	echo 'SOURCES=	\ ' >> tora.pro
	echo '	main.cpp \ ' >> tora.pro
	echo '	toextratranslations.cpp \ ' >> tora.pro
	echo "	$(tora_SOURCES)" >> tora.pro
	echo 'TRANSLATIONS=$(TRANSLATIONS)' >> tora.pro

lupdate: tora.pro
	@LUPDATE@ tora.pro

lrelease: tora.pro $(TRANSLATIONS)
	@LRELEASE@ tora.pro
