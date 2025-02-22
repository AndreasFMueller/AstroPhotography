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
#include <includes.h>

using namespace astro::adapter;
using namespace astro::image::filter;

namespace astro {
namespace focusing {

/**
 * \brief Construct a FocusWork controller
 */
FocusWork::FocusWork(Focusing& focusing) : _focusing(focusing) {
	_min = std::numeric_limits<unsigned long>::max();
	_max = std::numeric_limits<unsigned long>::min();
}

/**
 * \brief Check that the focusing parameters are all set
 */
bool	FocusWork::complete() {
	if (exposure().exposuretime() < 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "exposure time not set");
		return false;
	}
	if (_min == std::numeric_limits<unsigned long>::max()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "minimum not set");
		return false;
	}
	if (_max == std::numeric_limits<unsigned long>::min()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "maximum not set");
		return false;
	}
	if (_min >= _max) {
		debug(LOG_ERR, DEBUG_LOG, 0, "maximum < minimum");
		return false;
	}
	if (steps() < 3) {
		debug(LOG_ERR, DEBUG_LOG, 0, "focusing needs at least 3 points");
		return false;
	}
	if (!_focusing.ccd()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "ccd not set");
		return false;
	}
	if (!_focusing.focuser()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "focuser not set");
		return false;
	}
#if 0
	if (!_focusing.evaluator()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "evaluator not set");
		return false;
	}
	if (!_focusing.solver()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "solver not set");
		return false;
	}
#endif
	return true;
}

/**
 * \brief Call the callback with image and focus value
 */
void	FocusWork::callback(ImagePtr image, int position, double value) {
	// send the callback data
	if (!callback()) {
		return;
	}
	try {
		astro::callback::CallbackDataPtr	data(
			new FocusCallbackData(image, position, value));
		(*callback())(data);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "callback complete");
	} catch (...) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "exception during callback");
	}
}

void	FocusWork::callback(Focus::state_type state) {
	if (!callback()) {
		return;
	}
#if 0
	try {
		astro::callback::CallbackDataPtr	data(
			new FocusCallbackState(state));
		(*callback())(data);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "callback complete");
	} catch (...) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "exception during callback");
	}
#endif
}

/**
 * \brief default main function for focusing
 */
void	FocusWork::main(astro::thread::Thread<FocusWork>& /* thread */) {
	if (!complete()) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"FocusWork is not completely configured");
		focusingstatus(Focus::FAILED);
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting focus process in [%d,%d]",
		min(), max());

	// prepare the set of focus items to base the focus computation on
	FocusItems	focusitems;

	// prepare 
	for (int step = 0; step < steps(); step++) {
		// find position
		unsigned long	position
			= min() + (step * (max() - min())) / (steps() - 1);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "next position: %hu", position);

		// move to this position
		moveto(position);

		// get an image
		focusingstatus(Focus::MEASURING);
		ccd()->startExposure(exposure());
		usleep(1000000 * exposure().exposuretime());
		ccd()->wait();
		ImagePtr	image = ccd()->getImage();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "got an image of size %s",
			image->size().toString().c_str());

		// evaluate the image
		if (!evaluator()) {
			std::string	msg("no evaluator set");
			debug(LOG_ERR, DEBUG_LOG, 0, "no evaluator set");
			throw std::runtime_error(msg);
		}
		double	value = (*evaluator())(image);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "evaluated to %f", value);

		// callback with the evaluated image
		callback(evaluator()->evaluated_image(), position, value);

		// add the information to a set
		focusitems.insert(FocusItem(position, value));
	}

	// now solve we need a suitable solver for the method
	if (!solver()) {
		std::string	msg("no solver set");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	int	targetposition = solver()->position(focusitems);
	if ((targetposition < min()) || (targetposition > max())) {
		std::string	msg = stringprintf(
			"could not find a focus position: %d", targetposition);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		focusingstatus(Focus::FAILED);
		return;
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "final focus position: %d",
		targetposition);

	// move to the final focus position
	focusingstatus(Focus::MOVING);
	moveto(targetposition);
	focusingstatus(Focus::FOCUSED);
}

/**
 * \brief Extract and rescale the image as the green channel
 *
 * Independently of the pixel type of the focus camera, convert the image
 * to 8 bit and rescale the values so that th use the full range of the
 * camera.
 */
Image<unsigned char>	*FocusWork::green(ImagePtr image) {
	return UnsignedCharImage(image);
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
void	FocusWork::moveto(unsigned long position) {
	// ensure we are inside the interval
	if (position < min()) {
		throw std::runtime_error("internal error: Focuser move below min");
	}
	if (position > max()) {
		throw std::runtime_error("interval error: focuser move above max()");
	}

	// if we don't have a focuser, we throw an exception
	if (!focuser()) {
		std::string	msg("no focuser set");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// switch state to moving
	focusingstatus(Focus::MOVING);

	// check whether backlash compensation is needed
	if ((backlash() > 0) && (focuser()->current() > position)) {
		unsigned long	compensated = position - backlash();
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
unsigned long	FocusWork::backlash() {
	return (focuser()) ? focuser()->backlash() : 0;
}

/**
 * \brief get Focusing status
 */
Focus::state_type	FocusWork::focusingstatus() {
	return _focusing.status();
}

/**
 * \brief set the focusing status
 */
void	FocusWork::focusingstatus(Focus::state_type s) {
	callback(s);
	_focusing.status(s);
}

} // namespace focusing
} // namespace astro
