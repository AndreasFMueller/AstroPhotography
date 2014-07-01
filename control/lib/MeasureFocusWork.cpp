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

namespace astro {
namespace focusing {

/**
 * \brief Main function of the Focusing process
 */
void	MeasureFocusWork::main(astro::thread::Thread<FocusWork>& thread) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start focusing work");
	if (!complete()) {
		focusingstatus(Focusing::FAILED);
		throw std::runtime_error("focuser not completely specified");
	}

	focusingstatus(Focusing::FAILED);
}

} // namespace focusing
} // namespace astro
