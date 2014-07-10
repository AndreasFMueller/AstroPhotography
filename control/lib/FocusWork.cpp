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
 * \brief Convert the image to unsigned char pixel type
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
 *
 * Independently of the pixel type of the focus camera, convert the image
 * to 8 bit and rescale the values so that th use the full range of the
 * camera.
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

/**
 * \brief Move Focuser to a given position
 *
 * This method ensures that the movement always comes from the same side.
 * If the current position below the new position, nothing needs to be
 * done. If however the current position is above the new position then
 * the focuser is first moved to the target position minus the backlash
 * amount before being moved to the target position.
 */
void	FocusWork::moveto(unsigned short position) {
	// ensure we are inside the interval
	if (position < min()) {
		throw std::runtime_error("internal error: Focuser move below min");
	}
	if (position > max()) {
		throw std::runtime_error("interval error: focuser move above max()");
	}

	// switch state to moving
	focusingstatus(Focusing::MOVING);

	// check whether backlash compensation is needed
	if ((backlash() > 0) && (focuser()->current() > position)) {
		unsigned short	compensated = position - backlash();
		if (position < backlash()) {
			debug(LOG_WARNING, DEBUG_LOG, 0,
				"not enough room for backlash: current = %hu, "
				"position = %hu, backlash = %hu",
				focuser()->current(), position, backlash());
			compensated = 0;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"moving to compensated position: %hu", compensated);
		focuser()->moveto(compensated);
	}

	// now move to the final position
	debug(LOG_DEBUG, DEBUG_LOG, 0, "move to final position: %hu",
		position);
	focuser()->moveto(position);
}

/**
 * \brief Find backlash amount from Focuser
 */
unsigned short	FocusWork::backlash() {
	return (focuser()) ? focuser()->backlash() : 0;
}

/**
 * \brief get Focusing status
 */
Focusing::focus_status	FocusWork::focusingstatus() {
	return _focusing.status();
}

/**
 * \brief set the focusing status
 */
void	FocusWork::focusingstatus(Focusing::focus_status s) {
	_focusing.status(s);
}

} // namespace focusing
} // namespace astro
