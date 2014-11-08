#
# Makefile.am -- build ICE test client for the Astro project
#
# (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
if ENABLE_ICE

bin_PROGRAMS = snowscan snowimages snowfocus snowguide snowmount snowrepo \
	snowtask snowproject
noinst_PROGRAMS = snowflake 

ICEDEPENDENCIES = $(top_builddir)/lib/libastro.la \
	$(top_builddir)/ice/lib/libiceastro.la
ICELIBS = @ICELIBS@
ICELDADD = -L$(top_builddir)/lib -lastro \
	-L$(top_builddir)/ice/lib -liceastro $(ICELIBS)
ICECPPFLAGS = -I$(top_builddir)/ice/include @ICEINCLUDE@

snowflake_SOURCES = snowflake.cpp
snowflake_DEPENDENCIES = $(ICEDEPENDENCIES)
snowflake_LDADD = $(ICELDADD)
snowflake_CPPFLAGS = $(ICECPPFLAGS)

snowscan_SOURCES = snowscan.cpp
snowscan_DEPENDENCIES = $(ICEDEPENDENCIES)
snowscan_LDADD = $(ICELDADD)
snowscan_CPPFLAGS = $(ICECPPFLAGS)

snowimages_SOURCES = snowimages.cpp
snowimages_DEPENDENCIES = $(ICEDEPENDENCIES)
snowimages_LDADD = $(ICELDADD)
snowimages_CPPFLAGS = $(ICECPPFLAGS)

snowfocus_SOURCES = snowfocus.cpp
snowfocus_DEPENDENCIES = $(ICEDEPENDENCIES)
snowfocus_LDADD = $(ICELDADD)
snowfocus_CPPFLAGS = $(ICECPPFLAGS)

snowmount_SOURCES = snowmount.cpp
snowmount_DEPENDENCIES = $(ICEDEPENDENCIES)
snowmount_LDADD = $(ICELDADD)
snowmount_CPPFLAGS = $(ICECPPFLAGS)

snowguide_SOURCES = snowguide.cpp
snowguide_DEPENDENCIES = $(ICEDEPENDENCIES)
snowguide_LDADD = $(ICELDADD)
snowguide_CPPFLAGS = $(ICECPPFLAGS)

snowrepo_SOURCES = snowrepo.cpp RepoReplicators.cpp
snowrepo_DEPENDENCIES = $(ICEDEPENDENCIES)
snowrepo_LDADD = $(ICELDADD)
snowrepo_CPPFLAGS = $(ICECPPFLAGS)

snowtask_SOURCES = snowtask.cpp
snowtask_DEPENDENCIES = $(ICEDEPENDENCIES)
snowtask_LDADD = $(ICELDADD)
snowtask_CPPFLAGS = $(ICECPPFLAGS)

snowproject_SOURCES = snowproject.cpp
snowproject_DEPENDENCIES = $(ICEDEPENDENCIES)
snowproject_LDADD = $(ICELDADD)
snowproject_CPPFLAGS = $(ICECPPFLAGS)

test:	snowflake
	./snowflake -d -m simulator

endif
