#
# snowgui.pro -- qt configuration file for the snowgui project
#
# (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

QT       += core gui widgets

TEMPLATE = subdirs
SUBDIRS = astrogui icegui preview test focusing guiding instruments images
SUBDIRS += repository expose task browser main astrobrowser astroviewer
CONFIG += ordered

snowgui.depends = astrogui icegui preview focusing guiding instruments images \
	repository expose task browser main
