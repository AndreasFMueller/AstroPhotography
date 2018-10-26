#
# Makefile.am -- build focusing classes
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
SUBDIRS = . test

noinst_LTLIBRARIES = libastrofocusing.la

noinst_HEADERS =							\
	BrennerEvaluator.h						\
	FocusEvaluatorImplementation.h					\
	FocusSolvers.h							\
	FocusableImageConverterImpl.h					\
	FWHM2Evaluator.h						\
	MeasureEvaluator.h						\
	SymmetricSolver.h

libastrofocusing_la_SOURCES =						\
	AbsoluteValueSolver.cpp						\
	BrennerEvaluator.cpp						\
	BrennerSolver.cpp						\
	CentroidSolver.cpp						\
	Focus.cpp							\
	FocusableImageConverter.cpp					\
	FocusableImageConverterImpl.cpp					\
	FocusCallbackData.cpp						\
	FocusCompute.cpp						\
	FocusElement.cpp						\
	FocusEvaluatorImplementation.cpp				\
	FocusEvaluatorFactory.cpp					\
	FocusInputBase.cpp						\
	FocusInputImages.cpp						\
	FocusInput.cpp							\
	FocusOutput.cpp							\
	FocusParameters.cpp						\
	FocusProcessor.cpp						\
	FocusProcessBase.cpp						\
	FocusProcess.cpp						\
	FocusSolverFactory.cpp						\
	FocusWork.cpp							\
	Focusing.cpp							\
	FWHM2Evaluator.cpp						\
	MaximumSolver.cpp						\
	MinimumSolver.cpp						\
	MeasureEvaluator.cpp						\
	MeasureFocusWork.cpp						\
	ParabolicFocusSolver.cpp					\
	SymmetricSolver.cpp						\
	UnsignedCharImage.cpp						\
	VCurveFocusWork.cpp

libastrofocusing_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" 
	-DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"

