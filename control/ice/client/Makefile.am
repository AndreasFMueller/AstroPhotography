#
# Makefile.am -- build ICE test client for the Astro project
#
# (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
if ENABLE_ICE

noinst_PROGRAMS = snowflake snowscan iceimages icefocus snowguide

snowflake_SOURCES = snowflake.cpp


ICEDEPENDENCIES = $(top_builddir)/lib/libastro.la \
	$(top_builddir)/ice/lib/libiceastro.la
ICELDADD = -L$(top_builddir)/lib -lastro \
	-L$(top_builddir)/ice/lib -liceastro
ICELIBS = @ICELIBS@
ICECPPFLAGS = -I$(top_builddir)/ice/include @ICEINCLUDE@

snowflake_DEPENDENCIES = $(ICEDEPENDENCIES)
snowflake_LDADD = $(ICELDADD)
snowflake_LDFLAGS = $(ICELIBS)
snowflake_CPPFLAGS = $(ICECPPFLAGS)

snowscan_SOURCES = snowscan.cpp
snowscan_DEPENDENCIES = $(ICEDEPENDENCIES)
snowscan_LDADD = $(ICELDADD)
snowscan_LDFLAGS = $(ICELIBS)
snowscan_CPPFLAGS = $(ICECPPFLAGS)

iceimages_SOURCES = iceimages.cpp
iceimages_DEPENDENCIES = $(ICEDEPENDENCIES)
iceimages_LDADD = $(ICELDADD)
iceimages_LDFLAGS = $(ICELIBS)
iceimages_CPPFLAGS = $(ICECPPFLAGS)

icefocus_SOURCES = icefocus.cpp
icefocus_DEPENDENCIES = $(ICEDEPENDENCIES)
icefocus_LDADD = $(ICELDADD)
icefocus_LDFLAGS = $(ICELIBS)
icefocus_CPPFLAGS = $(ICECPPFLAGS)

icemount_SOURCES = icemount.cpp
icemount_DEPENDENCIES = $(ICEDEPENDENCIES)
icemount_LDADD = $(ICELDADD)
icemount_LDFLAGS = $(ICELIBS)
icemount_CPPFLAGS = $(ICECPPFLAGS)

snowguide_SOURCES = snowguide.cpp
snowguide_DEPENDENCIES = $(ICEDEPENDENCIES)
snowguide_LDADD = $(ICELDADD)
snowguide_LDFLAGS = $(ICELDFLAGS)
snowguide_CPPFLAGS = $(ICECPPFLAGS)

test:	snowflake
	./snowflake -d -m simulator

endif
