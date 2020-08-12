#
# Makefile.am -- build ICE test client for the Astro project
#
# (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
SUBDIRS = guide 

ICEDEPENDENCIES = $(top_builddir)/lib/libastro.la \
	$(top_builddir)/ice/lib/libiceastro.la
ICELIBS = @ICELIBS@
ICELDADD = -L$(top_builddir)/lib -lastro \
	-L$(top_builddir)/ice/lib -liceastro $(ICELIBS)
ICECPPFLAGS = -I$(top_builddir)/ice/include -I$(top_builddir)/ice/slice \
	@ICEINCLUDE@

bin_PROGRAMS = snowscan snowimages snowfocus snowmount snowrepo \
	snowtask snowproject snowinstrument snowao snowservers \
	snowevent snowconfig snowstream snowfiles snowdaemon \
	snowgateway snowguideport snowcooler snowheartbeat snowfilterwheel \
	snowadaptiveoptics
noinst_PROGRAMS = snowflake 

noinst_HEADERS = FocusCallbackI.h

snowconfig_SOURCES = snowconfig.cpp
snowconfig_DEPENDENCIES = $(ICEDEPENDENCIES)
snowconfig_LDADD = $(ICELDADD)
snowconfig_CPPFLAGS = $(ICECPPFLAGS)

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

snowfocus_SOURCES = FocusCallbackI.cpp snowfocus.cpp
snowfocus_DEPENDENCIES = $(ICEDEPENDENCIES)
snowfocus_LDADD = $(ICELDADD)
snowfocus_CPPFLAGS = $(ICECPPFLAGS)

snowguideport_SOURCES = snowguideport.cpp
snowguideport_DEPENDENCIES = $(ICEDEPENDENCIES)
snowguideport_LDADD = $(ICELDADD)
snowguideport_CPPFLAGS = $(ICECPPFLAGS)

snowcooler_SOURCES = snowcooler.cpp
snowcooler_DEPENDENCIES = $(ICEDEPENDENCIES)
snowcooler_LDADD = $(ICELDADD)
snowcooler_CPPFLAGS = $(ICECPPFLAGS)

snowmount_SOURCES = snowmount.cpp
snowmount_DEPENDENCIES = $(ICEDEPENDENCIES)
snowmount_LDADD = $(ICELDADD)
snowmount_CPPFLAGS = $(ICECPPFLAGS)

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

snowinstrument_SOURCES = snowinstrument.cpp
snowinstrument_DEPENDENCIES = $(ICEDEPENDENCIES)
snowinstrument_LDADD = $(ICELDADD)
snowinstrument_CPPFLAGS = $(ICECPPFLAGS)

snowao_SOURCES = snowao.cpp
snowao_DEPENDENCIES = $(ICEDEPENDENCIES)
snowao_LDADD = $(ICELDADD)
snowao_CPPFLAGS = $(ICECPPFLAGS)

snowservers_SOURCES = snowservers.cpp
snowservers_DEPENDENCIES = $(ICEDEPENDENCIES)
snowservers_LDADD = $(ICELDADD)
snowservers_CPPFLAGS = $(ICECPPFLAGS)

snowevent_SOURCES = snowevent.cpp
snowevent_DEPENDENCIES = $(ICEDEPENDENCIES)
snowevent_LDADD = $(ICELDADD)
snowevent_CPPFLAGS = $(ICECPPFLAGS)

snowstream_SOURCES = snowstream.cpp
snowstream_DEPENDENCIES = $(ICEDEPENDENCIES)
snowstream_LDADD = $(ICELDADD)
snowstream_CPPFLAGS = $(ICECPPFLAGS)

snowfiles_SOURCES = snowfiles.cpp
snowfiles_DEPENDENCIES = $(ICEDEPENDENCIES)
snowfiles_LDADD = $(ICELDADD)
snowfiles_CPPFLAGS = $(ICECPPFLAGS)

snowdaemon_SOURCES = snowdaemon.cpp
snowdaemon_DEPENDENCIES = $(ICEDEPENDENCIES)
snowdaemon_LDADD = $(ICELDADD)
snowdaemon_CPPFLAGS = $(ICECPPFLAGS)

snowgateway_SOURCES = snowgateway.cpp
snowgateway_DEPENDENCIES = $(ICEDEPENDENCIES)
snowgateway_LDADD = $(ICELDADD)
snowgateway_CPPFLAGS = $(ICECPPFLAGS)

snowheartbeat_SOURCES = snowheartbeat.cpp
snowheartbeat_DEPENDENCIES = $(ICEDEPENDENCIES)
snowheartbeat_LDADD = $(ICELDADD)
snowheartbeat_CPPFLAGS = $(ICECPPFLAGS)

snowfilterwheel_SOURCES = snowfilterwheel.cpp
snowfilterwheel_DEPENDENCIES = $(ICEDEPENDENCIES)
snowfilterwheel_LDADD = $(ICELDADD)
snowfilterwheel_CPPFLAGS = $(ICECPPFLAGS)

snowadaptiveoptics_SOURCES = snowadaptiveoptics.cpp
snowadaptiveoptics_DEPENDENCIES = $(ICEDEPENDENCIES)
snowadaptiveoptics_LDADD = $(ICELDADD)
snowadaptiveoptics_CPPFLAGS = $(ICECPPFLAGS)

test:	snowflake
	./snowflake -d -m simulator

teststream:
	./snowstream --debug --exposure=1 server SIM 

send-test:
	./snowgateway --debug --instrument=TEST				\
		--rightascension=20.69053 --declination=45.28.034	\
		--longitude=8.83 --latitude=47.1925			\
		server send

monitor-test:
	./snowgateway --debug --foreground 				\
		--post=http://cesario.othello.ch/telescope/update.php	\
		telescope monitor

EFENCE = LD_PRELOAD=libefence.so.0.0 EF_PROTECT_FREE=1 EF_FILL=255 EF_ALLOW_MALLOC_0=1
mdns-test:
	./snowservers --timeout=10 --debug 2>&1 | tee test.log

