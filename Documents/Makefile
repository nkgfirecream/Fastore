# $Id$
.SUFFIXES: .ms .dvi .ps .pdf

MS != ls *.ms
DVI = $(MS:.ms=.dvi)
PDF = $(MS:.ms=.pdf)
PS  = $(MS:.ms=.ps)

dvi: $(DVI)

pdf: $(PDF)

.ms.dvi:
	groff -pet -ms -Tdvi $(.ALLSRC) > $@~
	@ mv $@~ $@
.ms.ps:
	groff -pet -ms -Tps $(.ALLSRC) > $@~
	@ mv $@~ $@
.ps.pdf:
	ps2pdf $(.ALLSRC) $@
