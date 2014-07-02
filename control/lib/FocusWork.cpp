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

} // namespace focusing
} // namespace astro
