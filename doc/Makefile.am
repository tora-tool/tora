EXTRA_DIST = help

dist-hook:
	rm -rf `find $(distdir) -name .svn`

docdir = $(libdir)
doc_DATA = help/toc.htm

install-data-hook:
	cp -dpR $(top_srcdir)/doc/help $(docdir)
