/*
 * SimCamera.cpp -- Simulator Camera implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SimCamera.h>
#include <SimCcd.h>
#include <SimFilterWheel.h>
#include <SimGuidePort.h>
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
	// imaging CCD
	{
		DeviceName ccdname = CcdInfo::defaultname(name(), "ccd");
		CcdInfo	ccdi(ccdname, ImageSize(1920, 1080), 0);
		ccdi.addMode(Binning(1,1));
		ccdi.addMode(Binning(2,2));
		ccdi.addMode(Binning(3,3));
		ccdi.shutter(true);
		ccdi.pixelwidth(0.000006);
		ccdi.pixelheight(0.000006);
		ccdinfo.push_back(ccdi);
	}
	// guide CCD
	{
		DeviceName ccdname = CcdInfo::defaultname(name(), "guideccd");
		CcdInfo	ccdi(ccdname, ImageSize(640, 480), 0);
		ccdi.addMode(Binning(1,1));
		ccdi.addMode(Binning(2,2));
		ccdi.shutter(false);
		ccdi.pixelwidth(0.000005);
		ccdi.pixelheight(0.000005);
		ccdinfo.push_back(ccdi);
	}
	// finder ccd
	{
		DeviceName ccdname = CcdInfo::defaultname(name(), "finder");
		CcdInfo	ccdi(ccdname, ImageSize(1024, 1024), 0);
		ccdi.addMode(Binning(1,1));
		ccdi.shutter(false);
		ccdi.pixelwidth(0.000003);
		ccdi.pixelheight(0.000003);
		ccdinfo.push_back(ccdi);
	}
}

/**
 * \brief Get the simulated CCD
 *
 * The simulator camera only implements a single ccd.
 */
CcdPtr	SimCamera::getCcd0(size_t ccdid) {
	if (2 < ccdid) {
		throw NotFound("only ccds 0-2 exist");
	}
	CcdInfo	info = getCcdInfo(ccdid);
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
GuidePortPtr	SimCamera::getGuidePort0() {
	return _locator.guideport();
}

/**
 * \brief Get the camera
 */
std::string	SimCamera::userFriendlyName() const {
	return std::string("SimCam 1.0");
}

} // namespace simulator
} // namespace camera
} // namespace astro
