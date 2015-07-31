#
# Makefile.am -- build catalog related routines
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
SUBDIRS = . test

catalogs_ldadd = -lcppunit						\
	-L$(top_builddir)/lib/catalogs -lastrocatalogs			\
	-L$(top_builddir)/lib/image -lastroimage			\
	-L$(top_builddir)/lib/utils -lastroutils
catalogs_dependencies =							\
	$(top_builddir)/lib/utils/libastroutils.la			\
	$(top_builddir)/lib/image/libastroimage.la			\
	$(top_builddir)/lib/catalogs/libastrocatalogs.la


noinst_PROGRAMS = listcatalog

noinst_HEADERS =							\
	BSC.h								\
	CatalogBackend.h						\
	CatalogIterator.h						\
	Hipparcos.h							\
	MappedFile.h							\
	NGCIC.h								\
	Tycho2.h							\
	Ucac4.h

noinst_LTLIBRARIES = libastrocatalogs.la

listcatalog_SOURCES = listcatalog.cpp
listcatalog_DEPENDENCIES = $(catalogs_dependencies)
listcatalog_LDADD = $(catalogs_ldadd)

libastrocatalogs_la_SOURCES =						\
	BSC.cpp								\
	Catalog.cpp							\
	CatalogBackend.cpp						\
	CatalogFactory.cpp						\
	CatalogIterator.cpp						\
	Chart.cpp							\
	ConditionIterator.cpp						\
	DatabaseBackend.cpp						\
	DatabaseBackendCreator.cpp					\
	DatabaseBackendIterator.cpp					\
	FileBackend.cpp							\
	FileBackendIterator.cpp						\
	FileBackendWindowIterator.cpp					\
	Hipparcos.cpp							\
	ImageGeometry.cpp						\
	ImageNormalizer.cpp						\
	IteratorImplementation.cpp					\
	MappedFile.cpp							\
	NGCIC.cpp							\
	SkyRectangle.cpp						\
	SkyWindow.cpp							\
	Star.cpp							\
	StereographicChart.cpp						\
	Tycho2.cpp							\
	Ucac4Iterator.cpp						\
	Ucac4Star.cpp							\
	Ucac4StarNumber.cpp						\
	Ucac4WindowIterator.cpp						\
	Ucac4ZoneIterator.cpp						\
	Ucac4Zone.cpp							\
	Ucac4.cpp							\
	WindowPredicate.cpp

libastrocatalogs_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" \
	-DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"
