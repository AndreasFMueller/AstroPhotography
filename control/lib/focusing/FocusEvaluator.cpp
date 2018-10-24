/*
 * FocusEvaluator.cpp -- implementation of focus evaluators 
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroTypes.h>
#include <AstroFocus.h>
#include <AstroDebug.h>
#include "FocusEvaluator.h"

namespace astro {
namespace focusing {

FocusEvaluatorImplementation::FocusEvaluatorImplementation(
	const ImageRectangle& rectangle) : _rectangle(rectangle) {
}

FocusableImage	FocusEvaluatorImplementation::extractimage(const ImagePtr image) {
	FocusableImageConverterPtr	converter
		= FocusableImageConverter::get();
	if (!converter) {
		throw std::runtime_error("cannot get an image converter");
	}
	return (*converter)(image);
}

} // namespace focusing
} // namespace astro
