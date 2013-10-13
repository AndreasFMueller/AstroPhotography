#
# Makefile.am -- Build the command line guider client
#
# (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
bin_PROGRAMS = guide
noinst_HEADER = cli.h guidecli.h clicommand.h

COMMANDS = listcommand.cpp

guide_SOURCES = guide.cpp cli.cpp grammar.cpp tokens.cpp guidecli.cpp \
	$(COMMANDS)
guide_DEPENDENCIES = ../lib/libastro.la
guide_LDADD = -L../lib -lastro

EXTRA_DIST = tokens.l grammar.y testscript

BUILT_SOURCES = tokens.cpp grammar.cpp grammar.hpp

# generating the scanner source from tokens.l
tokens.cpp:     tokens.l
	$(FLEX) -+ --yylineno -o tokens.cpp $<

# generating the parser source from grammar.y
grammar.cpp grammar.hpp:        grammar.y
	$(BISON) -o grammar.cpp -d $< 

test:	guide
	./guide -ORBInitRef NameService=corbaname::localhost -d -f testscript 
