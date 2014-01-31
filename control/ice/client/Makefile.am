#
# Makefile.am -- build ICE test client for the Astro project
#
# (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
if ENABLE_ICE

noinst_PROGRAMS = snowflake

snowflake_SOURCES = snowflake.cpp

snowflake_DEPENDENCIES = $(top_builddir)/lib/libastro.la \
	$(top_builddir)/ice/lib/libiceastro.la
snowflake_LDADD = -L$(top_builddir)/lib -lastro \
	-L$(top_builddir)/ice/lib -liceastro
snowflake_LDFLAGS = -L@ICEDIR@/lib/c++11 -Wl,-rpath,@ICEDIR@/lib/c++11 -lIce -lIceUtil
snowflake_CPPFLAGS = -I$(top_builddir)/ice/include -I@ICEDIR@/include

test:	snowflake
	./snowflake -d -m simulator

endif
