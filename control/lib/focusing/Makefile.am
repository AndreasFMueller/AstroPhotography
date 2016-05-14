#
# Makefile.am -- build focusing classes
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
SUBDIRS = . test

noinst_LTLIBRARIES = libastrofocusing.la

libastrofocusing_la_SOURCES =						\
	AbsoluteValueSolver.cpp						\
	BrennerSolver.cpp						\
	CentroidSolver.cpp						\
	FocusableImageConverter.cpp					\
	FocusCompute.cpp						\
	FocusEvaluator.cpp						\
	FocusWork.cpp							\
	Focusing.cpp							\
	MeasureFocusWork.cpp						\
	ParabolicFocusSolver.cpp					\
	VCurveFocusWork.cpp

libastrofocusing_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" 
	-DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"

