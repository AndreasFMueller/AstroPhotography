#
# Makefile.am -- build the manual
#
# (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
docfiles = 

TEXSOURCES = snowstar.tex imageprocessing.tex imagerepo.tex \
	configuration.tex architecture.tex focusing.tex

EXTRA_DIST = manual.tex $(TEXSOURCES)

if HAVE_PDFLATEX
docfiles += manual.pdf

manual.pdf:	manual.tex manual.ind $(TEXSOURCES)
	$(PDFLATEX) manual.tex

manual.ind:	manual.tex $(TEXSOURCES)
	touch manual.ind
	$(PDFLATEX) manual.tex

doc_DATA = $(docfiles)

endif


