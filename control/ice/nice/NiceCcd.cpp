/*
 * NiceCcd.cpp  -- Nice CCD wrapper implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil 
 */
#include <NiceCcd.h>
#include <IceConversions.h>
#include <NiceCooler.h>
#include <AstroDebug.h>
#include <CommunicatorSingleton.h>

namespace astro {
namespace camera {
namespace nice {

static CcdInfo	ccd_rename(const CcdInfo info, const DeviceName& devicename) {
	CcdInfo	result(devicename, info.size(), info.getId());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "renamed to: %s",
		result.name().toString().c_str());
	result.addModes(info.modes());
	result.shutter(info.shutter());
	return result;
}

void	NiceCcdCallbackI::stop(const Ice::Current& /* current */) {
}

void	NiceCcdCallbackI::state(snowstar::ExposureState s,
	const Ice::Current& /* current */) {
	_ccd.stateUpdate(convert(s));
}

NiceCcd::NiceCcd(snowstar::CcdPrx ccd, const DeviceName& devicename)
	: Ccd(ccd_rename(snowstar::convert(ccd->getInfo()), devicename)),
	  NiceDevice(devicename), _ccd(ccd) {
	_ccd_callback = new NiceCcdCallbackI(*this);
	_ccd_identity = snowstar::CommunicatorSingleton::add(_ccd_callback);
	_ccd->registerCallback(_ccd_identity);
	
}

NiceCcd::~NiceCcd() {
	_ccd->unregisterCallback(_ccd_identity);
	snowstar::CommunicatorSingleton::remove(_ccd_identity);
}

void	NiceCcd::startExposure(const Exposure& exposure) {
	_ccd->startExposure(snowstar::convert(exposure));
}

CcdState::State	NiceCcd::exposureStatus() {
	state(snowstar::convert(_ccd->exposureStatus()));
	return state();
}

void	NiceCcd::cancelExposure() {
	_ccd->cancelExposure();
	state(snowstar::convert(_ccd->exposureStatus()));
}

Shutter::state	NiceCcd::getShutterState() {
	return snowstar::convert(_ccd->getShutterState());
}

void	NiceCcd::setShutterState(const Shutter::state& state) {
	_ccd->setShutterState(snowstar::convert(state));
}

astro::image::ImagePtr	NiceCcd::getRawImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve raw image");
	snowstar::ImagePrx	image = _ccd->getImage();
	return snowstar::convert(image);
}

bool	NiceCcd::hasGain() {
	return _ccd->hasGain();
}

float	NiceCcd::getGain() {
	return _ccd->getGain();
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
