/*
 * NetCcd.cpp -- network based CCD implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <NetCcd.h>

namespace astro {
namespace camera {
namespace net {

NetCcd::NetCcd(const CcdInfo& _info, Ccd_var ccd) : Ccd(_info), _ccd(ccd) {
}

void	NetCcd::startExposure(const Exposure& exposure) {
	Ccd::startExposure(exposure);
	Astro::Exposure	exp;
	_ccd->startExposure(exp);
}

Exposure::State	NetCcd::exposureStatus() {
	Astro::ExposureState	expstate = ccd->exposureStatus();
	// XXX convert Astro::ExposureState to Exposure::State, save in "state"
	return state;
}

void	NetCcd::cancelExposure() {
	_ccd->cancelExposure();
}

Exposure	NetCcd::getExposure() {
	Astro::Exposure	_ccd->getExposure();
	// XXX convert Astro::Exposure to Exposure
}

ImagePtr	NetCcd::getImage() {
	return ImagePtr();
}

bool	NetCcd::hasCooler() const {
	return ccd->hasCooler();
}

} // namespace net
} // namespace camera
} // namespace astro
