#
# snowgui.pro -- qt configuration file for the snowgui project
#
# (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

QT       += core gui widgets

TEMPLATE = subdirs
SUBDIRS = astrogui icegui image preview test focusing guiding pointing
SUBDIRS += instruments images repository expose task browser main
SUBDIRS += astrobrowser astroviewer
CONFIG += ordered

snowgui.depends = astrogui icegui image focusing guiding instruments images \
	repository expose task browser main
