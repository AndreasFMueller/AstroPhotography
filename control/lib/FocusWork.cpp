/*
 * FocusWork.cpp -- implement the work creating the focus
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <FocusWork.h>
#include <AstroDebug.h>
#include <limits>
#include <AstroFormat.h>
#include <FocusCompute.h>
#include <AstroAdapter.h>
#include <AstroFilter.h>

using namespace astro::adapter;
using namespace astro::image::filter;

namespace astro {
namespace focusing {

/**
 * \brief Construct a FocusWork controller
 */
FocusWork::FocusWork(Focusing& focusing) : _focusing(focusing) {
	_steps = 3;
	_min = std::numeric_limits<unsigned short>::max();
	_max = std::numeric_limits<unsigned short>::min();
	_exposure.exposuretime = -1;
}

/**
 * \brief Set the number of steps
 */
void	FocusWork::steps(unsigned short s) {
	if (s < 3) {
		throw std::invalid_argument("at least three steps needed");
	}
	if (s > 100) {
		throw std::invalid_argument("more than 100 steps no reasonable");
	}
	_steps = s;
}

/**
 * \brief Check that the focusing parameters are all set
 */
bool	FocusWork::complete() {
	if (_exposure.exposuretime < 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "exposure time not set");
		return false;
	}
	if (_min == std::numeric_limits<unsigned short>::max()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "minimum not set");
		return false;
	}
	if (_max == std::numeric_limits<unsigned short>::min()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "maximum not set");
		return false;
	}
	if (_min >= _max) {
		debug(LOG_ERR, DEBUG_LOG, 0, "maximum < minimum");
		return false;
	}
	if (!ccd()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "ccd not set");
		return false;
	}
	if (!focuser()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "focuser not set");
		return false;
	}
	return true;
}

/**
 * \brief Call the callback with image and focus value
 */
void	FocusWork::callback(ImagePtr image, double value) {
	// send the callback data
	if (!callback()) {
		return;
	}
	try {
		astro::callback::CallbackDataPtr	data(new FocusCallbackData(image, value));
		(*callback())(data);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "callback complete");
	} catch (...) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "exception during callback");
	}
}

/**
 */

#define	convert_to_unsigned_char(image, Pixel, green)			\
if (NULL == green) {							\
	Image<Pixel>	*imagep = dynamic_cast<Image<Pixel> *>(&*image);	\
	if (NULL != imagep) {						\
		green = new Image<unsigned char>(*imagep);		\
	}								\
}

/**
 * \brief Extract and rescale the image as the green channel
 */
Image<unsigned char>	*FocusWork::green(ImagePtr image) {
	Image<unsigned char>	*result = NULL;
	convert_to_unsigned_char(image, unsigned char, result);
	convert_to_unsigned_char(image, unsigned short, result);
	convert_to_unsigned_char(image, unsigned int, result);
	convert_to_unsigned_char(image, unsigned long, result);
	if (NULL == result) {
		throw std::runtime_error("cannot convert image to 8bit");
	}

	// get the maximum value of the image
	double	maxvalue = Max<unsigned char, double>().filter(*result);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "maximum value of image: %f", maxvalue);

	// new rescale all pixels
	double	multiplier = 255 / maxvalue;
	unsigned int	width = image->size().width();
	unsigned int	height = image->size().height();
	for (unsigned int x = 0; x < width; x++) {
		for (unsigned int y = 0; y < height; y++) {
			result->writablepixel(x, y) = 
				result->pixel(x, y) * multiplier;
		}
	}

	// return the converted result
	return result;
}

} // namespace focusing
} // namespace astro
