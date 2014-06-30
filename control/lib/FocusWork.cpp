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
	if (s > 10) {
		throw std::invalid_argument("more than 10 steps no reasonable");
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
	if (!evaluator()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "evaluator not set");
		return false;
	}
	return true;
}

/**
 * \brief Main function of the Focusing process
 */
void	FocusWork::main(astro::thread::Thread<FocusWork>& thread) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start focusing work");
	if (!complete()) {
		throw std::runtime_error("focuser not completely specified");
	}

	FocusCompute	fc;

	// determine how many intermediate steps we want to access

	if (_min < focuser()->min()) {
		throw std::runtime_error("minimum too small");
	}

	unsigned long	delta = max() - min();
	for (int i = 0; i < steps(); i++) {
		// compute new position
		unsigned short	position = min() + (i * delta) / (steps() - 1);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "measuring position %hu",
			position);

		// move to new position
		_focusing.status(Focusing::MOVING);
		focuser()->moveto(position);
		
		// get an image from the Ccd
		_focusing.status(Focusing::MEASURING);
		ccd()->startExposure(exposure());
		ccd()->wait();
		ImagePtr	image = ccd()->getImage();
		
		// turn the image into a value
		double	value = (*evaluator())(image);

		// add the new value 
		fc.insert(std::pair<unsigned short, double>(position, value));

		// send the callback data
		if (callback()) {
			try {
				astro::callback::CallbackDataPtr	data(new FocusCallbackData(image, value));
				(*callback())(data);
				debug(LOG_DEBUG, DEBUG_LOG, 0, "callback complete");
			} catch (...) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"exception during callback");
			}
		}
	}

	// compute the best focus position
	double	focusposition = fc.focus();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "optimal focus position: %f",
		focusposition);

	// move to the focus position
	unsigned short	targetposition = focusposition;
	focuser()->moveto(targetposition);
	_focusing.status(Focusing::FOCUSED);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "target position reached");
}

} // namespace focusing
} // namespace astro
