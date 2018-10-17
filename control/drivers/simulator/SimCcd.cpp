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
#include <SimMount.h>
#include <SimAdaptiveOptics.h>
#include <includes.h>
#include <AstroCatalog.h>

using namespace astro::image;
using namespace astro::catalog;

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
}

static void	imageconstruction_main(SimCcd *simccd) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting image construction");
	simccd->createimage();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image construction complete");
}

/**
 * \brief Start simulated exposure
 *
 * \param exposure	exposure parameters
 */
void    SimCcd::startExposure(const Exposure& exposure) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting exposure");

	// find the current position, this ensures that the filter wheel
	// has settled
	if (_locator.filterwheel()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "waiting for filterwheel");
		_locator.filterwheel()->currentPosition();
	} else {
		debug(LOG_ERR, DEBUG_LOG, 0, "no filterwheel found");
	}

	// ensure that the guideport ist updated before we start exposing
	if (_locator.simguideport()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "update guideport");
		_locator.simguideport()->update();
	} else {
		debug(LOG_ERR, DEBUG_LOG, 0, "no guideport found");
	}

	// get the orientation from the mount
	if (_locator.simmount()) {
		starcamera.west(_locator.simmount()->telescopePositionWest());
	}

	// find focal length and limit magnitude
	float	focallength = parameterValueFloat("focallength");
	float	limit_magnitude = parameterValueFloat("limit_magnitude");
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"focallength = %.3f, limit_magnitude = %.2f",
		focallength, limit_magnitude);

	// start the image construction thread
	_thread = new std::thread(imageconstruction_main, this);

	// start the exposure
	Ccd::startExposure(exposure);
	starttime = simtime();
	state(CcdState::exposing);
	shutter = exposure.shutter();
}

/**
 * \brief Work function for the thread
 *
 * This method retrieves the image and then rebuilds the image
 * and stores it in _image.
 */
void	SimCcd::createimage() {
	// start timer
	Timer	timer;
	timer.start();

	debug(LOG_DEBUG, DEBUG_LOG, 0, "start image construction thread");
	// update the mount position
	RaDec	rd = _locator.mount()->getRaDec();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "RA/DEC: %s", rd.toString().c_str());
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
		_last_direction = rd;
	}

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

	// check whether exposure time is already over, this can happen
	// if the starfield had to be rebuilt from the catalog
	if ((timer.gettime() - timer.startTime()) > exposure.exposuretime()) {
		state(CcdState::exposed);
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "build a new image");
	ImagePtr	image = starcamera(starfield);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got an %s image: %s",
		image->getFrame().toString().c_str(), image->info().c_str());

	// compute the remaining exposure time
	double	remaining = exposure.exposuretime()
				- (timer.gettime() - timer.startTime());
	if (remaining > 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "wait for remaining time %.3f",
			remaining);
		Timer::sleep(remaining);
	}

	// now signal that the image is exposed
	debug(LOG_DEBUG, DEBUG_LOG, 0, "change state");
	state(CcdState::exposed);

	// origin
	image->setOrigin(exposure.frame().origin());
	_image = image;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image construction thread terminated");
}

/**
 * \brief Construct a star field from the direciton
 *
 * \param direction	coordinates of center of the star field
 */
void	SimCcd::catalogStarfield(const RaDec& direction) {
	// clear the star field
	starfield.clear();

	// get the parameters
	float	focallength = parameterValueFloat("focallength");
	float	azimuth = parameterValueFloat("azimuth");
	float	limit_magnitude = parameterValueFloat("limit_magnitude");

	// angular resolution
	float	pxlx = getInfo().pixelwidth() / focallength;
	float	pxly = getInfo().pixelheight() / focallength;
	Point	center(starfield.size().center());

	// ImageCoordinate converter
	Angle	resolution(getInfo().pixelwidth() / focallength);
	ImageCoordinates	coord(direction, resolution, Angle(0));

	// compute the width and height of the image
	Angle	anglewidth(getInfo().size().width() * pxlx);
	Angle	angleheight(getInfo().size().height() * pxly);

	// get a SkyWindow of appropriate size
	SkyWindow	window = SkyWindow::hull(direction,
					anglewidth, angleheight);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "window: %s", window.toString().c_str());

	// get the appropriate catalog
	CatalogPtr	catalog = CatalogFactory::get(CatalogFactory::Combined);

	// retrieve the stars
	MagnitudeRange	magrange(-30, limit_magnitude);
	Catalog::starsetptr	stars = catalog->find(window, magrange);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got %d stars", stars->size());

	// add the stars to the star field
	Catalog::starset::const_iterator	s;
	for (s = stars->begin(); s != stars->end(); s++) {
		// get the object
		RaDec	pos = s->position(2000);
		Point	position = center + coord(pos);

		// create the new star
		astro::StellarObjectPtr	ns(new astro::Star(position, s->mag()));
		
		// add the star to the field
		starfield.addObject(ns);
	}

	// rotate the star field
	Angle	alpha;
	alpha.degrees(-azimuth);
	image::transform::Transform	transform(alpha.radians(), Point());
	transform = image::transform::Transform(alpha.radians(),
			center - transform(center));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rotation by %.1f degrees, %s",
		alpha.degrees(), transform.toString().c_str());
	starfield.transform(transform);
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
	case CcdState::streaming:
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

#if 0
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "wait()");
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
#endif

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
	// wait for the thread 
	_thread->join();
	delete _thread;

	return _image;
}

/**
 * \brief Get a user friendly name for display
 */
std::string	SimCcd::userFriendlyName() const {
	return std::string("SimCam 1.0");
}

} // namespace simulator
} // namespace camera
} // namespace astro

