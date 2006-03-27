AM_CPPFLAGS  = -I$(top_srcdir)/ext/otl $(KDE_INCLUDES) $(QT_INCLUDES) $(LTDLINCL)
AM_CPPFLAGS  += $(PCRE_CFLAGS) \
	-DDEFAULT_PLUGIN_DIR=\"$(libdir)\" \
	-DTOVERSION=\"$(PACKAGE_VERSION)\" \
	$(ORACLE_CXXFLAGS) \
	-I$(top_srcdir)/src \
	$(CPPUNIT_CFLAGS) \
	$(QSCINTILLA_CXXFLAGS)

AM_LDFLAGS = $(PCRE_LIBS) \
	$(KDE_LDFLAGS) \
	$(X_LDFLAGS) \
	$(QT_LDFLAGS) \
	$(LIB_KPARTS) \
	$(LIB_KDEPRINT) \
	$(LIB_KDECORE) \
	$(LIB_KDEUI) \
	$(LIB_KIO) \
	$(LIB_KFILE) \
	$(LIB_DCOP) \
	$(LIB_KHTML) \
	-lqscintilla \
	$(CPPUNIT_LIBS) \
	$(LIBPTHREAD) \
	$(ORACLE_LDFLAGS) \
	$(QSCINTILLA_LDFLAGS)

if CPPUNIT

TESTS = ConfigurationTest
check_PROGRAMS = $(TESTS)

endif
ConfigurationTest_SOURCES = toconfigurationtest.h toconfigurationtest.cpp tomaintest.cpp 

