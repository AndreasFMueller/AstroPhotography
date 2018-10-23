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
	: _output(new FocusOutput(input)) {
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
void	FocusProcessor::process(const FocusElement& element) {
	FocusElement	result = element;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "processing position %lu", result.pos());

	// first make sure we have the input image, open it if we
	// don't have it.
	if (!element.raw_image) {
		result.raw_image = element.image();
	}

	// now process the image:
	// 1. get an evaluator for this type of image
	FocusEvaluatorFactory	evaluatorfactory;
	FocusEvaluatorPtr	evaluator
		= evaluatorfactory.get(_output->method());

	// 2. run the image through the evaluator, adding the info to the
	//    element
	result.value = (*evaluator)(result.raw_image);
	result.processed_image = evaluator->evaluated_image();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%lu -> %f", result.pos(), result.value);

	// 3. send a callback message XXX

	// if we are not to keep the images, throw them away now
	if (!_keep_images) {
		result.raw_image.reset();
		result.processed_image.reset();
	}

	// add the element to the output
	_output->insert(std::make_pair(result.pos(), result));
}

/**
 * \brief Process the input 
 *
 * Process the input by applying the process method to each element of the
 * input.
 *
 * \param input		input to process
 */
void	FocusProcessor::process(const FocusInput& input) {
	FocusProcessor	*fp = this;
	std::for_each(input.begin(), input.end(),
		[fp,input](const std::pair<unsigned int, std::string>& p)
			mutable {
			FocusElement	fe(p.first);
			fe.filename = p.second;
			fp->process(fe);
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
void	FocusProcessor::process(const FocusInputImages& input) {
	FocusProcessor	*fp = this;
	std::for_each(input.begin(), input.end(),
		[fp,input](const std::pair<unsigned int, ImagePtr>& p)
			mutable {
			FocusElement	fe(p.first);
			fe.raw_image = p.second;
			fp->process(fe);
		}
	);
}

} // namespace focusing
} // namespace astro
