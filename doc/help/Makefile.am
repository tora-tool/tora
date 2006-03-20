dist-hook:
	rm -rf `find $(distdir) -name .svn`

info_TEXINFOS = tora.texi

SUFFIXES = .html

html_docs = tora.html

.texi.html:
	$(MAKEINFO) --html $<

.PHONY: html
html: $(html_docs)
