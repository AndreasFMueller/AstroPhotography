/*
 * Focusing.cpp -- implementation of auto focusing
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroFocus.h>

using namespace astro::camera;

namespace astro {
namespace focusing {

/**
 * \brief Create 
 */
Focusing::Focusing(CcdPtr ccd, FocuserPtr focuser)
	: FocusProcess(1, 2147483647, ccd, focuser) {
	method(std::string("fwhm"));
	steps(3);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create Focusing @ %p", this);
}

/**
 * \brief destroy the Focusing object
 *
 * If the thread is still running, it must be stopped
 */
Focusing::~Focusing() {
}

/**
 * \brief Start the focusing process in a given interval
 */
void	Focusing::start(int min, int max) {
	minposition(min);
	maxposition(max);

	FocusProcess::start();
}

/**
 * \brief Cancel the focusing process
 */
void	Focusing::cancel() {
	FocusProcess::stop();
}

} // namespace focusing
} // namespace astro
