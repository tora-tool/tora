EXTRA_DIST = help

dist-hook:
	rm -rf `find $(distdir) -name .svn`

tora_docdir = $(DESTDIR)$(libdir)

install-data-hook:
	rm -rf $(tora_docdir)/help
	cp -dpR $(top_builddir)/doc/help/tora $(tora_docdir)/help 
	cp $(tora_docdir)/help/index.html $(tora_docdir)/help/toc.html
	cp -dpR $(top_srcdir)/doc/help/images $(tora_docdir)/help
#	cp -dpR $(top_srcdir)/doc/help/api $(tora_docdir)/help

manualdir = $(top_builddir)/doc/help

all-local:
	echo Making HTML in $(manualdir)
	cd $(manualdir) && make html

SUBDIRS = help
