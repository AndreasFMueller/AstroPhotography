#
# Makefile.am -- build focusing classes
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
SUBDIRS = . test

noinst_LTLIBRARIES = libastrofocusing.la

noinst_HEADERS = FocusEvaluator.h BrennerEvaluator.h FWHM2Evaluator.h \
	MeasureEvaluator.h

libastrofocusing_la_SOURCES =						\
	AbsoluteValueSolver.cpp						\
	BrennerEvaluator.cpp						\
	BrennerSolver.cpp						\
	CentroidSolver.cpp						\
	FocusableImageConverter.cpp					\
	FocusCompute.cpp						\
	FocusEvaluator.cpp						\
	FocusEvaluatorFactory.cpp					\
	FocusWork.cpp							\
	Focusing.cpp							\
	FWHM2Evaluator.cpp						\
	MaximumSolver.cpp						\
	MeasureEvaluator.cpp						\
	MeasureFocusWork.cpp						\
	ParabolicFocusSolver.cpp					\
	UnsignedCharImage.cpp						\
	VCurveFocusWork.cpp

libastrofocusing_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" 
	-DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"

