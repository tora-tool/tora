all:
	doxygen
	echo "*** See error.log for errors and warnings"

clean:
	rm *.ttf *.dot *.html *.png *.css error.log *.map *.md5 *.gif *.tmp

tgz:
	tar -cf sqliteman-devel-docs.tar
	gzip sqliteman-devel-docs.tar

tbz:
	tar -cf sqliteman-devel-docs.tar *
	bzip2 sqliteman-devel-docs.tar

codecheck:
	flawfinder --html -c ../sqliteman > code-check.html
