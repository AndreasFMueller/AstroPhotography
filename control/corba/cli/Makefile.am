#
# Makefile.am -- Build the command line guider client
#
# (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
if ENABLE_CORBA

bin_PROGRAMS = astro
noinst_HEADER = cli.h guidecli.h clicommand.h listcommand.h locatorcommand.h \
	modulecommand.h helpcommand.h cameracommand.h ccdcommand.h \
	coolercommand.h sleepcommand.h filterwheelcommand.h focusercommand.h \
	guiderfactorycommand.h guidercommand.h imagecommand.h \
	ObjWrapper.h Cameras.h Ccds.h Filterwheels.h Focusers.h \
	Guiderports.h Guiders.h Images.h \
	exposurecommand.h taskcommand.h Output.h \
	submitcommand.h taskqueuecommand.h guiderportcommand.h

CLASSES = Cameras.cpp Ccds.cpp Filterwheels.cpp Focusers.cpp \
	Guiderports.cpp Guiders.cpp Images.cpp Output.cpp
COMMANDS = listcommand.cpp locatorcommand.cpp modulecommand.cpp \
	helpcommand.cpp cameracommand.cpp ccdcommand.cpp coolercommand.cpp \
	sleepcommand.cpp filterwheelcommand.cpp focusercommand.cpp \
	guiderfactorycommand.cpp guidercommand.cpp imagecommand.cpp \
	exposurecommand.cpp taskcommand.cpp submitcommand.cpp \
	taskqueuecommand.cpp guiderportcommand.cpp

astro_SOURCES = astro.cpp cli.cpp grammar.cpp tokens.cpp guidecli.cpp \
	clicommand.cpp $(CLASSES) $(COMMANDS)
astro_DEPENDENCIES = $(top_builddir)/lib/libastro.la \
	$(top_builddir)/corba/lib/libcorbastro.la
astro_LDADD = -L$(top_builddir)/lib -lastro \
	-L$(top_builddir)/corba/lib -lcorbastro

EXTRA_DIST = tokens.l grammar.y testscript

BUILT_SOURCES = tokens.cpp grammar.cpp grammar.hpp

# generating the scanner source from tokens.l
tokens.cpp:     tokens.l
	$(FLEX) -+ --yylineno -o tokens.cpp $<

# generating the parser source from grammar.y
grammar.cpp grammar.hpp:        grammar.y
	$(BISON) -o grammar.cpp -d $< 

test:	astro
	./astro -ORBInitRef NameService=corbaname::localhost -d -f testscript 

testcli:	astro
	./astro -ORBInitRef NameService=corbaname::localhost -d

endif 
