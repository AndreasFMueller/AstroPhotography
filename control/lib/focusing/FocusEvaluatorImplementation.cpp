/*
 * FocusEvaluator.cpp -- implementation of focus evaluators 
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroTypes.h>
#include <AstroFocus.h>
#include <AstroDebug.h>
#include "FocusEvaluatorImplementation.h"

namespace astro {
namespace focusing {

/**
 * \brief Construct a FocusEvaluator without rectangle
 */
FocusEvaluatorImplementation::FocusEvaluatorImplementation() {
}

/**
 * \brief Construct a FocusEvaluator
 */
FocusEvaluatorImplementation::FocusEvaluatorImplementation(
	const ImageRectangle& rectangle) : _rectangle(rectangle) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "use rectangle %s",
		_rectangle.toString().c_str());
}

/**
 * \brief Extract a focusable image from the input image
 *
 * The extraction process also copies the metadata
 */
FocusableImage	FocusEvaluatorImplementation::extractimage(const ImagePtr image) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "extractimage for rectangle %s",
		_rectangle.toString().c_str());
	FocusableImageConverterPtr	converter
		= FocusableImageConverter::get(_rectangle);
	if (!converter) {
		throw std::runtime_error("cannot get an image converter");
	}
	FocusableImage	fimage = (*converter)(image);

        // add metadata to the image
	ImageMetadata::const_iterator   i;
	for (i = image->begin(); i != image->end(); i++) {
		fimage->setMetadata(i->second);
	}

	// return the image
	return fimage;
}

/**
 * \brief Evaluate the image
 *
 * \param image		input image with any input type
 */
double	FocusEvaluatorImplementation::operator()(ImagePtr image) {
	// extract the image
	FocusableImage	fimage = extractimage(image);

	// evaluate the image
	return this->evaluate(fimage);
}

} // namespace focusing
} // namespace astro
