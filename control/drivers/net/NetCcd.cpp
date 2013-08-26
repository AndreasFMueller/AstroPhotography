/*
 * NetCcd.cpp -- network based CCD implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <NetCcd.h>
#include <NetCooler.h>
#include <Conversions.h>

namespace astro {
namespace camera {
namespace net {

NetCcd::NetCcd(const CcdInfo& _info, Astro::Ccd_ptr ccd)
	: Ccd(_info), _ccd(ccd) {
	Astro::Ccd_Helper::duplicate(_ccd);
}

NetCcd::~NetCcd() {
	Astro::Ccd_Helper::release(_ccd);
}

void	NetCcd::startExposure(const Exposure& _exposure) {
	Ccd::startExposure(_exposure);
	_ccd->startExposure(convert(_exposure));
	exposure = convert(_ccd->getExposure());
}

Exposure::State	NetCcd::exposureStatus() {
	return convert(_ccd->exposureStatus());
}

void	NetCcd::cancelExposure() {
	_ccd->cancelExposure();
}

ImagePtr	NetCcd::getImage() {
	return ImagePtr();
}

bool	NetCcd::hasCooler() const {
	return _ccd->hasCooler();
}

CoolerPtr	NetCcd::getCooler0() {
	Astro::Cooler_var	cooler = _ccd->getCooler();
	return CoolerPtr(new NetCooler(cooler));
}

} // namespace net
} // namespace camera
} // namespace astro
