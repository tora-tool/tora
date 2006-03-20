EXTRA_DIST = help

dist-hook:
	rm -rf `find $(distdir) -name .svn`

docdir = $(libdir)
doc_DATA = help/toc.htm

install-data-hook:
	rm -rf $(docdir)/help
	cp -dpR $(top_builddir)/doc/help/tora $(docdir)/help 
	cp $(docdir)/help/index.html $(docdir)/help/toc.htm
	cp -dpR $(top_srcdir)/doc/help/images $(docdir)/help
	cp -dpR $(top_srcdir)/doc/help/api $(docdir)/help

manualdir = $(top_builddir)/doc/help

all-local:
	echo Making HTML in $(manualdir)
	cd $(manualdir) && make html
