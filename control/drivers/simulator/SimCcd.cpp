/*
 * SimCcd.cpp -- simulate a CCD
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SimCcd.h>
#include <SimUtil.h>
#include <AstroExceptions.h>
#include <SimGuidePort.h>
#include <SimCooler.h>
#include <SimFocuser.h>
#include <SimAdaptiveOptics.h>
#include <includes.h>
#include <AstroCatalog.h>

using namespace astro::image;

namespace astro {
namespace camera {
namespace simulator {

#define STARFIELD_OVERSHOOT	100
#define	NUMBER_OF_STARS		200

/**
 * \brief Auxiliary function to compute the number of stars to create
 *
 * This method attempts to create the same star density for every
 * ccd size.
 */
static unsigned int	number_of_stars(const ImageSize& size) {
	unsigned int	l = ImageSize(640, 480).getPixels();
	unsigned int	s = (NUMBER_OF_STARS * size.getPixels()) / l;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating %u stars", s);
	return s;
}

/**
 * \brief Create a simulated CCD
 */
SimCcd::SimCcd(const CcdInfo& _info, SimLocator& locator)
	: Ccd(_info), _locator(locator),
	  starfield(_info.size(), STARFIELD_OVERSHOOT,
			number_of_stars(_info.size())),
	  starcamera(ImageRectangle(_info.size())) {
	starcamera.addHotPixels(6);

	// set the last direction to an impossible direction to ensure
	// that the first time around, a star field will be generated
	// from the star catalog
	_last_direction.ra().degrees(-1);

	// add parameter descriptors for focal length and limiting magnitude
	device::ParameterDescription	focallength_description(
						"focallength", 0.01, 4.0);
	Device::add(focallength_description);
	device::ParameterDescription	azimuth_description(
						"azimuth", 0.0, 360.0);
	Device::add(azimuth_description);
	device::ParameterDescription	limit_magnitude_description(
						"limit_magnitude", 0.0, 16.0);
	Device::add(limit_magnitude_description);

	// get focal length parameter
	debug(LOG_DEBUG, DEBUG_LOG, 0, "querying 'focallength' for %s",
		name().toString().c_str());
	float	focallength = 0;
	if (hasProperty("focallength")) {
		focallength = std::stod(getProperty("focallength"));
	} else {
		focallength = 1.1111;
	}
	parameter("focallength", focallength);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "using focallength %.3f[m]",
		parameterValueFloat("focallength"));

	// get the azimuth parameter 
	debug(LOG_DEBUG, DEBUG_LOG, 0, "querying 'azimuth' for %s",
		name().toString().c_str());
	float	azimuth = 0;
	if (hasProperty("azimuth")) {
		azimuth = std::stod(getProperty("azimuth"));
	} else {
		azimuth = 1.1111;
	}
	parameter("azimuth", azimuth);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "using azimuth %.3f[degrees]",
		parameterValueFloat("azimuth"));

	// get limit magnitude parameter
	float	limit_magnitude = 0;
	if (hasProperty("limit_magnitude")) {
		limit_magnitude = std::stod(getProperty("limit_magnitude"));
	} else {
		limit_magnitude = 11.111;
	}
	parameter("limit_magnitude", limit_magnitude);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "using limit magnitude %.2f",
		limit_magnitude);
}

/**
 * \brief Start simulated exposure
 */
void    SimCcd::startExposure(const Exposure& exposure) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting exposure");

	// find the current position, this ensures that the filter wheel
	// has settled
	_locator.filterwheel()->currentPosition();

	// ensure that the guideport ist updated before we start exposing
	_locator.simguideport()->update();

	// find focal length and limit magnitude
	float	focallength = parameterValueFloat("focallength");
	float	limit_magnitude = parameterValueFloat("limit_magnitude");
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"focallength = %.3f, limit_magnitude = %.2f",
		focallength, limit_magnitude);

	// update the mount position
	RaDec	rd = _locator.mount()->getRaDec();
	if (rd != _last_direction) {
		if (rd == RaDec()) {
			double	s = log2(1 + fabs(rd.ra().radians()
				+ rd.dec().radians()));
			s = s - trunc(s) + 30;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "log of seed: %f", s);
			unsigned long	seed = trunc(pow(2, s));
			starfield.rebuild(seed);
		} else {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"create star field from catalog");
			catalogStarfield(rd);
		}
	}

	// start the exposure
	Ccd::startExposure(exposure);
	starttime = simtime();
	state(CcdState::exposing);
	shutter = exposure.shutter();
}


/**
 * \brief Construct a star field from the direciton
 */
void	SimCcd::catalogStarfield(const RaDec& direction) {
	// get the parameters
	float	focallength = parameterValueFloat("focallength");
	float	azimuth = parameterValueFloat("azimuth");
	float	limit_magniture = parameterValueFloat("limit_magnitude");

	// compute the width and height of the image
	Angle	anglewidth(getInfo().size().width() * getInfo().pixelwidth()
			/ focallength);
	Angle	angleheight(getInfo().size().height() * getInfo().pixelheight()
			/ focallength);

	// get a SkyWindow of appropriate size
	catalog::SkyWindow	window(direction, anglewidth, angleheight);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", window.toString().c_str());

	// XXX get the appropriate catalog and retrieve the 

	starfield.rebuild(4711);
}

/**
 * \brief query the exposure state
 *
 * This also changes the value of the state member
 */
CcdState::State	SimCcd::exposureStatus() {
	double	now = simtime();
	double	timepast = now - starttime;
	switch (state()) {
	case CcdState::idle:
	case CcdState::exposed:
	case CcdState::cancelling:
		return state();
	case CcdState::exposing:
		if (timepast > exposure.exposuretime()) {
			state(CcdState::exposed);
		}
		return state();
	}
	// this exception is mainly thrown to silence the compiler, it should
	// never happen
	debug(LOG_DEBUG, DEBUG_LOG, 0, "unknown status");
	throw std::runtime_error("unknown state");
}

/**
 * \brief cancel the exposure
 */
void    SimCcd::cancelExposure() {
	if (CcdState::idle == state()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no exposure in progress");
		throw BadState("no exposure in progress");
	}
	state(CcdState::idle);
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
	if ((CcdState::idle == state()) || (CcdState::cancelling == state())) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no exposure in progress");
		throw BadState("no exposure in progress");
	}
	if (CcdState::exposed == state()) {
		return true;
	}
	// compute the remaining exposure time
	double	remaining = exposure.exposuretime()
			- (simtime() - starttime);
	if (remaining > 0) {
		unsigned int	remainingus = 1000000 * remaining;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "sleeping for %.3f", remaining);
		usleep(remainingus);
	}

	// exposure is now complete
	state(CcdState::exposed);
	return true;
}

/**
 * \brief Remember the shutter state
 */
void    SimCcd::setShuterState(const Shutter::state& state) {
	shutter = state;
}

/**
 * \brief Retrieve an image
 */
ImagePtr  SimCcd::getRawImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get image from simulator");
	// we need a camera to convert the starfield into an image
	starcamera.rectangle(exposure.frame());

	// exposure influence
	starcamera.stretch(exposure.exposuretime());
	starcamera.light(exposure.shutter() == Shutter::OPEN);

	// flat images need special treatment
	if (exposure.purpose() == Exposure::flat) {
		starcamera.light(false);
		starcamera.dark(20000. * exposure.exposuretime());
	}

	// geometric distortion (guideport)
	starcamera.translation(_locator.simguideport()->offset()
		+ _locator.simadaptiveoptics()->offset());

	// color (filterwheel)
	starcamera.colorfactor(_locator.filterwheel()->currentPosition());

	// temperature influence on noise
	starcamera.noise(0.2 * exp2(-_locator.simcooler()->belowambient()));

	// focuser effect
	double	radius = _locator.simfocuser()->radius();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "radius = %f", radius);
	starcamera.radius(radius);
	starcamera.innerradius(0.4 * radius);

	// binning mode
	starcamera.binning(exposure.mode());

	debug(LOG_DEBUG, DEBUG_LOG, 0, "build a new image");
	ImagePtr	image = starcamera(starfield);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got an %s image: %s",
		image->getFrame().toString().c_str(), image->info().c_str());
	state(CcdState::idle);

	// origin
	image->setOrigin(exposure.frame().origin());
	return image;
}

std::string	SimCcd::userFriendlyName() const {
	return std::string("SimCam 1.0");
}

} // namespace simulator
} // namespace camera
} // namespace astro

