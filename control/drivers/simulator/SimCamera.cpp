/*
 * SimCamera.cpp -- Simulator Camera implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SimCamera.h>
#include <SimCcd.h>
#include <SimFilterWheel.h>
#include <SimGuiderPort.h>
#include <AstroExceptions.h>

using namespace astro::image;

namespace astro {
namespace camera {
namespace simulator {

/**
 * \brief Simulator camera construction
 *
 * The simulator camera supports two binning modes and has a shutter
 */
SimCamera::SimCamera(SimLocator& locator)
	: Camera("camera:simulator/camera"), _locator(locator) {
	DeviceName	ccdname = CcdInfo::defaultname(name(), "ccd");
	CcdInfo	ccdi(ccdname, ImageSize(640, 480), 0);
	ccdi.addMode(Binning(1,1));
	ccdi.addMode(Binning(2,2));
	ccdi.shutter(true);
	ccdi.pixelwidth(0.00001);
	ccdi.pixelheight(0.00001);
	ccdinfo.push_back(ccdi);
}

/**
 * \brief Get the simulated CCD
 *
 * The simulator camera only implements a single ccd.
 */
CcdPtr	SimCamera::getCcd0(size_t ccdid) {
	if (0 != ccdid) {
		throw NotFound("only ccd 0 exists");
	}
	CcdInfo	info = getCcdInfo(0);
	return CcdPtr(new SimCcd(info, _locator));
}

/**
 * \brief Get the filterwheel
 */
FilterWheelPtr	SimCamera::getFilterWheel0() {
	return _locator.filterwheel();
}

/**
 * \brief Get the guider port
 */
GuiderPortPtr	SimCamera::getGuiderPort0() {
	return _locator.guiderport();
}

} // namespace simulator
} // namespace camera
} // namespace astro
