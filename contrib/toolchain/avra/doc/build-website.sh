#!/bin/sh

VERS="1.3.0"
DATE="2010-06-28"

ASCIIDOC_HTML="asciidoc --unsafe --backend=xhtml11 --conf-file=layout1.conf --attribute icons --attribute iconsdir=./images/icons --attribute=badges --attribute=revision=$VERS  --attribute=date=$DATE"

$ASCIIDOC_HTML -a index-only index.txt
$ASCIIDOC_HTML ChangeLog.txt
$ASCIIDOC_HTML downloads.txt
$ASCIIDOC_HTML README.txt
