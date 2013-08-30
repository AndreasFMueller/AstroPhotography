/*
 * NetCamera.cpp -- Network based camera
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <NetCamera.h>
#include <NetCcd.h>
#include <NetFilterWheel.h>
#include <NetGuiderPort.h>
#include <AstroExceptions.h>
#include <AstroDebug.h>

using namespace astro::image;

namespace astro {
namespace camera {
namespace net {

/**
 * \brief Create a network camera client
 *
 * A network camera client keeps a reference to a remote camera reference.
 * In addition, its parent class keeps information about the CCDs available,
 * so the constructor retrieves this information from the remote object
 * and caches it locally.
 */
NetCamera::NetCamera(Astro::Camera_var camera) : _camera(camera) {
	// retrieve Ccds from the camera reference and fill the CCDinfo
	int	nccds = _camera->nCcds();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera has %d CCDs", nccds);

	for (int ccdid = 0; ccdid < nccds; ccdid++) {
		Astro::CcdInfo_var	ci = _camera->getCcdinfo(ccdid);
		astro::camera::CcdInfo	ci2 = convert(ci);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "add %s",
			ci2.toString().c_str());
		ccdinfo.push_back(convert(ci));
	}

	_hasfilterwheel = _camera->hasFilterWheel();
	_hasguiderport = _camera->hasGuiderPort();
}

/**
 * \brief Get the CCD 
 */
CcdPtr	NetCamera::getCcd0(size_t ccdid) {
	if (ccdid >= ccdinfo.size()) {
		throw NotFound("ccd id too large");
	}
	Astro::Ccd_ptr	ccdptr = _camera->getCcd(ccdid);
	Astro::Ccd_var	ccdvar = ccdptr;
	return CcdPtr(new NetCcd(ccdinfo[ccdid], ccdvar));
}

/**
 * \brief Check whether the camera has a filter wheel
 */
bool	NetCamera::hasFilterWheel() const {
	return _hasfilterwheel;
}

/**
 * \brief Get the FilterWheel
 */
FilterWheelPtr	NetCamera::getFilterWheel0() {
	if (!_hasfilterwheel) {
		throw NotFound("camera does not have a filter wheel");
	}
	return FilterWheelPtr(new NetFilterWheel(_camera->getFilterWheel()));
}

/**
 * \brief Check whether the camera has guider port
 */
bool	NetCamera::hasGuiderPort() const {
	return _hasguiderport;
}

/**
 * \brief Get the GuiderPort
 */
GuiderPortPtr	NetCamera::getGuiderPort0() {
	if (!_hasguiderport) {
		throw NotFound("camera does not have guider port");
	}
	return GuiderPortPtr(new NetGuiderPort(_camera->getGuiderPort()));
}

} // namespace net
} // namespace camera
} // namespace astro

