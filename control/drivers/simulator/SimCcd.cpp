/*
 * SimCcd.cpp -- simulate a CCD
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SimCcd.h>
#include <SimUtil.h>
#include <AstroExceptions.h>
#include <SimGuiderPort.h>
#include <SimCooler.h>
#include <SimFocuser.h>
#include <includes.h>

using namespace astro::image;

namespace astro {
namespace camera {
namespace simulator {

#define STARFIELD_OVERSHOOT	100
#define	NUMBER_OF_STARS		200

/**
 * \brief Create a simulated CCD
 */
SimCcd::SimCcd(const CcdInfo& _info, SimLocator& locator)
	: Ccd(_info), _locator(locator),
	  starfield(_info.size(), STARFIELD_OVERSHOOT, NUMBER_OF_STARS),
	  starcamera(ImageRectangle(_info.size())) {
	starcamera.addHotPixels(6);
}

/**
 * \brief Start simulated exposure
 */
void    SimCcd::startExposure(const Exposure& exposure) {
	Ccd::startExposure(exposure);
	starttime = simtime();
	state = Exposure::exposing;
	shutter = exposure.shutter;
}

/**
 * \brief query the exposure state
 *
 * This also changes the value of the state member
 */
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
	// this exception is mainly thrown to silence the compiler, it should
	// never happen
	throw std::runtime_error("illegal state");
}

/**
 * \brief cancel the exposure
 */
void    SimCcd::cancelExposure() {
	if (Exposure::idle == state) {
		throw BadState("no exposure in progress");
	}
	state = Exposure::idle;
}

/**
 * \brief Wait for completion of the exopsure
 *
 * This is a reimplementation of the wait method because during tests we don't
 * really want to wait for the exposure time to really elapse. So we fake it.
 * Note that this doesn't affect the exposureStatus method. If one only looks
 * at the exposureStatus, one can still wait (the Ccd::wait method would do
 * that).
 */
bool    SimCcd::wait() {
	if ((Exposure::idle == state) || (Exposure::cancelling == state)) {
		throw BadState("no exposure in progress");
	}
	if (Exposure::exposed == state) {
		return true;
	}
	// compute the remaining exposure time
	double	remaining = exposure.exposuretime
			- (simtime() - starttime);
	if (remaining > 0) {
		unsigned int	remainingus = 1000000 * remaining;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "sleeping for %.3f", remaining);
		usleep(remainingus);
	}

	// exposure is now complete
	state = Exposure::exposed;
	return true;
}

/**
 * \brief Remember the shutter state
 */
void    SimCcd::setShuterState(const shutter_state& state) {
	shutter = state;
}

/**
 * \brief Retrieve an image
 */
ImagePtr  SimCcd::getImage() {
	// we need a camera to convert the starfield into an image
	starcamera.rectangle(exposure.frame);

	// exposure influence
	starcamera.stretch(exposure.exposuretime);
	starcamera.light(exposure.shutter == SHUTTER_OPEN);

	// geometric distortion (guiderport)
	starcamera.translation(_locator.simguiderport()->offset());
	starcamera.alpha(_locator.simguiderport()->alpha());

	// color (filterwheel)
	starcamera.colorfactor(_locator.filterwheel()->currentPosition());

	// temperature influence on noise
	starcamera.noise(0.2 * exp2(-_locator.simcooler()->belowambient()));

	// focuser effect
	double	radius = _locator.simfocuser()->radius();
	starcamera.radius(radius);
	starcamera.innerradius(0.4 * radius);

	// binning mode
	starcamera.binning(exposure.mode);

	ImagePtr	image = starcamera(starfield);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got an %s image",
		image->getFrame().toString().c_str());
	state = Exposure::idle;

	// origin
	image->setOrigin(exposure.frame.origin());
	return image;
}

} // namespace simulator
} // namespace camera
} // namespace astro

