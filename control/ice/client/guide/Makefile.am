#
# Makefile.am -- build ICE test client for the Astro project
#
# (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

ICEDEPENDENCIES = $(top_builddir)/lib/libastro.la \
	$(top_builddir)/ice/lib/libiceastro.la
ICELIBS = @ICELIBS@
ICELDADD = -L$(top_builddir)/lib -lastro \
	-L$(top_builddir)/ice/lib -liceastro $(ICELIBS)
ICECPPFLAGS = -I$(top_builddir)/ice/include @ICEINCLUDE@

if ENABLE_ICE

bin_PROGRAMS = snowguide 

noinst_HEADERS = display.h monitor.h guide.h

snowguide_SOURCES = display.cpp monitor.cpp guide.cpp			\
	guidecmd.cpp calcmd.cpp generalcmd.cpp monitorcmd.cpp 		\
	snowguide.cpp
snowguide_DEPENDENCIES = $(ICEDEPENDENCIES)
snowguide_LDADD = $(ICELDADD)
snowguide_CPPFLAGS = $(ICECPPFLAGS)

calibrate:	snowguide
	./snowguide --debug --star=349,252 --width=60 \
		--exposure=1 server GUIDESIM calibrate
guide:	snowguide
	./snowguide --debug --star=349,252 --width=60 \
		--exposure=1 --interval=3 server GUIDESIM guide

endif