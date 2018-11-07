/*
 * FocusProcessor.cpp -- process a focus input 
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroFocus.h>
#include <AstroDebug.h>

namespace astro {
namespace focusing {

/**
 * \brief Construct a processor
 */
FocusProcessor::FocusProcessor(const FocusInputBase& input)
	: _output(new FocusOutput(input)), _rectangle(input.rectangle()) {
}

FocusProcessor::FocusProcessor(const std::string& method,
	const std::string& solver)
	: _keep_images(false),
	  _output(new FocusOutput(FocusInputBase(method, solver))) {
}

/**
 * \brief Process a focus element
 *
 * This method does the real work. It process the raw image to produce
 * a processed image and a evaluation result. This information is then
 * add to the element and the element is added to the output.
 *
 * \param element	the element to be processed
 */
void	FocusProcessor::process(FocusElement& element) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "processing position %lu",
		element.pos());

	// first make sure we have the input image, open it if we
	// don't have it.
	if (!element.raw_image) {
		element.raw_image = element.image();
	}

	// remove the UUID and create a new one

	// now process the image:
	// 1. get an evaluator for this type of image
	FocusEvaluatorFactory	evaluatorfactory;
	FocusEvaluatorPtr	evaluator
		= evaluatorfactory.get(_output->method(), rectangle());
	if (!evaluator) {
		std::string	msg = stringprintf("evaluator %s not found",
			_output->method().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "processing image %s",
		element.raw_image->info().c_str());

	// 2. run the image through the evaluator, adding the info to the
	//    element
	element.value = (*evaluator)(element.raw_image);
	element.processed_image = evaluator->evaluated_image();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%lu -> %f", element.pos(),
		element.value);

	// if we are not to keep the images, throw them away now
	if (!_keep_images) {
		element.raw_image.reset();
		element.processed_image.reset();
	}

	// add the element to the output
	_output->insert(std::make_pair(element.pos(), element));
}

/**
 * \brief Process the input 
 *
 * Process the input by applying the process method to each element of the
 * input.
 *
 * \param input		input to process
 */
void	FocusProcessor::process(FocusInput& input) {
	if (image::ImageRectangle() == rectangle()) {
		rectangle(input.rectangle());
	}
	std::for_each(input.begin(), input.end(),
		[&](const std::pair<unsigned int, std::string>& p) {
			FocusElement	fe(p.first);
			fe.filename = p.second;
			process(fe);
		}
	);
}

/**
 * \brief Process the input images
 *
 * Process the input image by applying the process method to each element
 * of the input.
 *
 * \param input		input images to process
 */
void	FocusProcessor::process(FocusInputImages& input) {
	if (image::ImageRectangle() == rectangle()) {
		rectangle(input.rectangle());
	}
	std::for_each(input.begin(), input.end(),
		[&](const std::pair<unsigned int, ImagePtr>& p) {
			FocusElement	fe(p.first);
			fe.raw_image = p.second;
			process(fe);
		}
	);
}

} // namespace focusing
} // namespace astro
