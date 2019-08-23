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
	$(top_builddir)/lib/image/libastroimage.la


noinst_PROGRAMS = listcatalog

noinst_HEADERS =							\
	BSC.h								\
	CatalogBackend.h						\
	CatalogIterator.h						\
	CutoverConditions.h						\
	DeepSkyCatalogs.h						\
	Hipparcos.h							\
	MappedFile.h							\
	NGCIC.h								\
	PGC.h								\
	Stellarium.h							\
	Tycho2.h							\
	Ucac4.h

noinst_LTLIBRARIES = libastrocatalogs.la

listcatalog_SOURCES = listcatalog.cpp
listcatalog_DEPENDENCIES = $(catalogs_dependencies) libastrocatalogs.la
listcatalog_LDADD = $(catalogs_ldadd) 

libastrocatalogs_la_SOURCES =						\
	BSC.cpp								\
	BSCStar.cpp							\
	BSCIterator.cpp							\
	Catalog.cpp							\
	CatalogBackend.cpp						\
	CatalogFactory.cpp						\
	CatalogIterator.cpp						\
	CelestialObject.cpp						\
	Chart.cpp							\
	ChartFactory.cpp						\
	ChartFactoryBase.cpp						\
	ConditionIterator.cpp						\
	CutoverConditions.cpp						\
	DatabaseBackend.cpp						\
	DatabaseBackendCreator.cpp					\
	DatabaseBackendIterator.cpp					\
	DeepSkyObject.cpp						\
	DeepSkyCatalogFactory.cpp					\
	EndIterator.cpp							\
	FileBackend.cpp							\
	FileBackendIterator.cpp						\
	FileBackendWindowIterator.cpp					\
	Hipparcos.cpp							\
	HipparcosStar.cpp						\
	HipparcosIterator.cpp						\
	ImageGeometry.cpp						\
	ImageNormalizer.cpp						\
	IteratorImplementation.cpp					\
	MagnitudeRange.cpp						\
	MappedFile.cpp							\
	MessierCatalog.cpp						\
	NGCIC.cpp							\
	NGCICCatalog.cpp						\
	PGCDatabase.cpp							\
	PGC.cpp								\
	PGCCatalog.cpp							\
	Outline.cpp							\
	OutlineCatalog.cpp						\
	PointSpreadFunctionAdapter.cpp					\
	PointSpreadFunction.cpp						\
	SkyRectangle.cpp						\
	SkyWindow.cpp							\
	Star.cpp							\
	Stellarium.cpp							\
	StereographicChart.cpp						\
	Tycho2.cpp							\
	Tycho2Star.cpp							\
	Tycho2Iterator.cpp						\
	Ucac4Iterator.cpp						\
	Ucac4Star.cpp							\
	Ucac4StarNumber.cpp						\
	Ucac4WindowIterator.cpp						\
	Ucac4ZoneIterator.cpp						\
	Ucac4Zone.cpp							\
	Ucac4.cpp							\
	WindowPredicate.cpp

libastrocatalogs_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\"		\
	-DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"			\
	-DDATAROOTDIR=\"${datarootdir}\"
