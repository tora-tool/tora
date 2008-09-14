ACLOCAL_AMFLAGS = -I config/m4
READMES = README\
	README.CODESTYLE \
	README.LICENSE \
	README.OSX \
	README.RELEASE \
	README.WINDOWS 

EXTRA_DIST = debian \
	rpm \
	BUGS \
	COPYING \
	Makefile.OSX \
	Makefile.mingw \
	$(READMES) \
	config/m4 \
	autogen.sh

dist-hook:
	rm -rf `find $(distdir) -name .svn`

SUBDIRS = utils src test doc
