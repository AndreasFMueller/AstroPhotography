#
# Makefile.am -- build focusing classes
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
SUBDIRS = . test

noinst_LTLIBRARIES = libastrofocusing.la

noinst_HEADERS =							\
	BackgroundAdapter.h						\
	BrennerEvaluator.h						\
	FocusEvaluatorImplementation.h					\
	FocusSolvers.h							\
	FocusableImageConverterImpl.h					\
	FWHM2Evaluator.h						\
	FWHMEvaluator.h							\
	MeasureEvaluator.h						\
	SymmetricSolver.h

libastrofocusing_la_SOURCES =						\
	AbsoluteValueSolver.cpp						\
	BackgroundAdapter.cpp						\
	BrennerEvaluator.cpp						\
	BrennerSolver.cpp						\
	CentroidSolver.cpp						\
	Focus.cpp							\
	FocusableImageConverter.cpp					\
	FocusableImageConverterImpl.cpp					\
	FocusCallbackData.cpp						\
	FocusCompute.cpp						\
	FocusElement.cpp						\
	FocusElementCallback.cpp					\
	FocusElementCallbackData.cpp					\
	FocusElementQueue.cpp						\
	FocusEvaluatorImplementation.cpp				\
	FocusEvaluatorFactory.cpp					\
	FocusImagePreconditioner.cpp					\
	FocusInputBase.cpp						\
	FocusInputImages.cpp						\
	FocusInput.cpp							\
	FocusOutput.cpp							\
	FocusParameters.cpp						\
	FocusProcessor.cpp						\
	FocusProcessBase.cpp						\
	FocusProcess.cpp						\
	FocusSolverFactory.cpp						\
	Focusing.cpp							\
	FWHM2Evaluator.cpp						\
	FWHMEvaluator.cpp						\
	MaximumSolver.cpp						\
	MinimumSolver.cpp						\
	MeasureEvaluator.cpp						\
	ParabolicFocusSolver.cpp					\
	SymmetricSolver.cpp						\
	UnsignedCharImage.cpp	


libastrofocusing_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" 
	-DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"

