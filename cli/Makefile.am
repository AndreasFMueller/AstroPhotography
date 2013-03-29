#
# Makefile.am -- build the command line interpreter
#
# (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
include_HEADERS = cli.h
noinst_HEADERS = variables.h

# build the astrocli library, it contains everything but the main program
lib_LIBRARIES = libastrocli.a

libastrocli_a_SOURCES = variables.cpp cli.cpp tokens.cpp grammar.cpp

EXTRA_DIST = tokens.l grammar.y testscript

BUILT_SOURCES = tokens.cpp grammar.cpp grammar.hpp

# generating the scanner source from tokens.l
tokens.cpp:	tokens.l
	$(FLEX) -+ --yylineno -o tokens.cpp $<

# generating the parser source from grammar.y
grammar.cpp grammar.hpp:	grammar.y
	$(BISON) -o grammar.cpp -d $< 

# build the astrocli program
bin_PROGRAMS = astrocli

astrocli_SOURCES = astrocli.cpp
astrocli_LDADD = -L. -lastrocli
astrocli_DEPENDENCIES = libastrocli.a

# test targets
test:	astrocli
	./astrocli -d < testscript
