/*
 * NiceCcd.cpp  -- Nice CCD wrapper implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil 
 */
#include <NiceCcd.h>
#include <IceConversions.h>
#include <NiceCooler.h>

namespace astro {
namespace camera {
namespace nice {

NiceCcd::NiceCcd(snowstar::CcdPrx ccd, const DeviceName& devicename)
	: Ccd(convert(ccd->getInfo())), NiceDevice(devicename), _ccd(ccd) {
}

NiceCcd::~NiceCcd() {
}

void	NiceCcd::startExposure(const Exposure& exposure) {
	_ccd->startExposure(snowstar::convert(exposure));
}

Exposure::State	NiceCcd::exposureStatus() {
	return snowstar::convert(_ccd->exposureStatus());
}

void	NiceCcd::cancelExposure() {
	_ccd->cancelExposure();
}

Shutter::state	NiceCcd::getShutterState() {
	return snowstar::convert(_ccd->getShutterState());
}

void	NiceCcd::setShutterState(const Shutter::state& state) {
	_ccd->setShutterState(snowstar::convert(state));
}

astro::image::ImagePtr	NiceCcd::getRawImage() {
	snowstar::ImagePrx	image = _ccd->getImage();
	return snowstar::convert(image);
}

bool	NiceCcd::hasGain() {
	return _ccd->hasGain();
}

std::pair<float, float>	NiceCcd::gainInterval() {
	return snowstar::convert(_ccd->gainInterval());
}

bool	NiceCcd::hasCooler() const {
	return _ccd->hasCooler();
}

CoolerPtr	NiceCcd::getCooler0() {
	snowstar::CoolerPrx	cooler = _ccd->getCooler();
	return CoolerPtr(new NiceCooler(cooler, nice(cooler->getName())));
}

} // namespace nice
} // namespace camera
} // namespace astro
