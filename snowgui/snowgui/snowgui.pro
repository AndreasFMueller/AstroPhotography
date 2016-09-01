#
# snowgui.pro -- qt configuration file for the snowgui project
#
# (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

QT       += core gui widgets

TEMPLATE = subdirs
SUBDIRS = common preview test focusing guiding main
CONFIG += ordered

snowgui.depends = common preview
