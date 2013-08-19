/*
 * SimCcd.cpp --
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SimCcd.h>
#include <SimUtil.h>
#include <AstroExceptions.h>

using namespace astro::image;

namespace astro {
namespace camera {
namespace simulator {

SimCcd::SimCcd(const CcdInfo& _info, SimLocator& locator) : Ccd(_info), _locator(locator) {
}

void    SimCcd::startExposure(const Exposure& exposure) {
	Ccd::startExposure(exposure);
	starttime = simtime();
	state = Exposure::exposing;
	shutter = exposure.shutter;
}

Exposure::State	SimCcd::exposureStatus() {
	double	now = simtime();
	double	timepast = now - starttime;
	switch (state) {
	case Exposure::idle:
	case Exposure::exposed:
	case Exposure::cancelling:
		return state;
	case Exposure::exposing:
		if (timepast > exposure.exposuretime) {
			state = Exposure::exposed;
		}
		return state;
	}
}

void    SimCcd::cancelExposure() {
	if ((Exposure::exposing == state) || (Exposure::exposed == state)) {
		throw BadState("no exposure in progress");
	}
	state = Exposure::idle;
}

bool    SimCcd::wait() {
	if ((Exposure::idle == state) || (Exposure::cancelling == state)) {
		throw BadState("no exposure in progress");
	}
	if (Exposure::exposed == state) {
		return true;
	}
	double	now = simtime();
	double	remaining = exposure.exposuretime - (now - starttime);
	if (remaining > 0) {
		int	w = 1000000 * remaining;
		usleep(w);
	}
	state = Exposure::exposed;
	return true;
}

void    SimCcd::setShuterState(const shutter_state& state) {
	shutter = state;
}

ImagePtr  SimCcd::getImage() {
	return ImagePtr();
}

} // namespace simulator
} // namespace camera
} // namespace astro

