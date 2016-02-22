#!/bin/sh -x
# creates a pdf file with full TOra documentation
texi2pdf -q --pdf tora.texi
mv tora.pdf tora_documentation.pdf
# following statement will clean-up temporary files
texi2pdf --mostly-clean tora.texi
