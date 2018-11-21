#
# Makefile.am -- Makefile for the Astro project
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
SUBDIRS = include lib ice drivers tools test man doc

documentation:
	cd doc; make doc

codesize:
	find drivers guider ice include lib tools \
		-name slice -prune -o \
		-name common -prune -o \
		-name \*.h -print -o \
		-name \*.c -print -o \
		-name \*.cpp -print | xargs wc -l
