/*
 * VCurveFocusWork.cpp -- implement the work finding the focus using a V-Curve
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <FocusWork.h>
#include <AstroDebug.h>
#include <limits>
#include <AstroFormat.h>
#include <FocusCompute.h>
#include <AstroFilterfunc.h>
#include <AstroFilter.h>
#include <AstroAdapter.h>
#include "FWHM2Evaluator.h"
#include <includes.h>

using namespace astro::image::filter;
using namespace astro::adapter;

namespace astro {
namespace focusing {

/**
 * \brief Main function of the Focusing process
 */
void	VCurveFocusWork::main(astro::thread::Thread<FocusWork>& /* thread */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start focusing work");
	if (!complete()) {
		std::string	msg("focuser not completely specified");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		focusingstatus(Focusing::FAILED);
		throw std::runtime_error(msg);
	}

	FocusCompute	fc;

	// determine how many intermediate steps we want to access

	if (min() < focuser()->min()) {
		std::string	msg = stringprintf("minimum %d smaller than allowed %d", min(), focuser()->min());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// based on the exposure specification, build an evaluator
	ImageSize	size = exposure().size();
	int	radius = std::min(size.width(), size.height()) / 2;
	FWHM2Evaluator	evaluator(size.center(), radius);

	unsigned long	delta = max() - min();
	for (int i = 0; i < steps(); i++) {
		// compute new position
		unsigned long	position = min() + (i * delta) / (steps() - 1);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "measuring position %hu",
			position);

		// move to new position
		moveto(position);
		
		// get an image from the Ccd
		focusingstatus(Focusing::MEASURING);
		ccd()->startExposure(exposure());
		usleep(1000000 * exposure().exposuretime());
		ccd()->wait();
		ImagePtr	image = ccd()->getImage();
		
		// turn the image into a value
		FWHMInfo	fwhminfo = focusFWHM2_extended(image,
					size.center(), radius);
		double	value = evaluator(image);

		// add the new value 
		fc.insert(std::pair<unsigned long, double>(position, value));

		// send the callback data
		callback(evaluator.evaluated_image(), position, value);
	}

	// compute the best focus position
	double	focusposition = 0;
	try {
		focusposition = fc.focus();
	} catch (std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no optimal focus position: %s",
			x.what());
		focusingstatus(Focusing::FAILED);
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "optimal focus position: %f",
		focusposition);

	// plausibility check for the position
	if (!((focusposition >= min()) && (focusposition <= max()))) {
		focusingstatus(Focusing::FAILED);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "focusing failed");
		return;
	}

	// move to the focus position
	unsigned long	targetposition = focusposition;
	moveto(targetposition);
	focusingstatus(Focusing::FOCUSED);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "target position reached");
}

} // namespace focusing
} // namespace astro
