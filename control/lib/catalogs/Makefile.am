#
# Makefile.am -- build catalog related routines
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
SUBDIRS = . test

noinst_HEADERS =							\
	BSC.h								\
	CatalogBackend.h						\
	Hipparcos.h							\
	MappedFile.h							\
	NGCIC.h								\
	Tycho2.h							\
	Ucac4.h

noinst_LTLIBRARIES = libastrocatalogs.la

libastrocatalogs_la_SOURCES =						\
	BSC.cpp								\
	Catalog.cpp							\
	CatalogBackend.cpp						\
	Chart.cpp							\
	DatabaseBackend.cpp						\
	FileBackend.cpp							\
	Hipparcos.cpp							\
	ImageGeometry.cpp						\
	ImageNormalizer.cpp						\
	MappedFile.cpp							\
	NGCIC.cpp							\
	SkyRectangle.cpp						\
	SkyWindow.cpp							\
	StereographicChart.cpp						\
	Tycho2.cpp							\
	Ucac4.cpp

libastrocatalogs_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" \
	-DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"
