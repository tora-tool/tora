dist-hook:
	rm -rf `find $(distdir) -name .svn`

info_TEXINFOS = tora.texi 

tora_TEXINFO = \
	additionalhelp.texi \
	alert.texi \
	analyze.texi \
	api.texi \
	browser.texi \
	charts.texi \
	common.texi \
	current.texi \
	editor.texi \
	database.texi \
	debugger.texi \
	editors.texi \
	extendingtora.texi \
	externalapi.texi \
	help.texi \
	history.texi \
	invalid.texi \
	lists.texi \
	newconnection.texi \
	output.texi \
	preferences.texi \
	rollback.texi \
	script.texi \
	searchreplace.texi \
	security.texi \
	session.texi \
	sqledit.texi \
	storage.texi \
	template.texi \
	tools.texi \
	toolsetting.texi \
	tora.texi \
	trace.texi \
	tuning.texi \
	window.texi \
	worksheet.texi \
	workspace.texi 

SUFFIXES = .html

html_docs = tora/index.html

$(html_docs): $(tora_TEXINFO)
	$(MAKEINFO) --html $(top_srcdir)/doc/help/tora.texi

.texi.html:
	$(MAKEINFO) --html $<

all: html

.PHONY: html
html: $(html_docs)
