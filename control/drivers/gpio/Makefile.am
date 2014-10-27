#
# Makefile.am -- build the gpio driver
#
# (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
pkglib_LTLIBRARIES = gpio.la
noinst_HEADERS = GpioLocator.h GpioGuiderport.h

SRCFILES = GpioLocator.cpp GpioGuiderport.cpp

gpio_la_SOURCES = $(SRCFILES)
gpio_la_LDFLAGS = -module
gpio_la_DEPENDENCIES = ../../lib/libastro.la
gpio_la_LIBADD = ../../lib/libastro.la

