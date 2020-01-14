#
# build the basic library
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
SUBDIRS = utils persistence event discovery image processing device config \
	catalogs guiding focusing task usb unicap solarsystem . test

noinst_HEADERS = Version.h

lib_LTLIBRARIES = libastro.la

libastro_la_SOURCES = Version.cpp

libastro_la_LIBADD =							\
	catalogs/libastrocatalogs.la					\
	config/libastroconfig.la					\
	device/libastrodevice.la					\
	discovery/libastrodiscovery.la					\
	event/libastroevent.la						\
	focusing/libastrofocusing.la					\
	guiding/libastroguiding.la					\
	image/libastroimage.la						\
	persistence/libastropersistence.la				\
	processing/libastroprocessing.la				\
	task/libastrotask.la						\
	unicap/libastrounicap.la					\
	usb/libastrousb.la						\
	utils/libastroutils.la						\
	solarsystem/libastrosolarsystem.la
