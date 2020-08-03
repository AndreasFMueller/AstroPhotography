/*
 * NiceGuidePort.cpp
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <NiceGuidePort.h>
#include <CommunicatorSingleton.h>
#include <IceConversions.h>

namespace astro {
namespace camera {
namespace nice {

void	NiceGuidePortCallbackI::activate(
                        const snowstar::GuidePortActivation& activation,
                        const Ice::Current& /* current */) {
	_guideport.callback(convert(activation));
}

void	NiceGuidePortCallbackI::stop(const Ice::Current& /* current */) {
}

NiceGuidePort::NiceGuidePort(snowstar::GuidePortPrx guideport,
	const DeviceName& devicename)
	: GuidePort(devicename), NiceDevice(devicename),
	  _guideport(guideport) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "niceguideport constructed");
	_guideport_callback = new NiceGuidePortCallbackI(*this);
	_guideport_identity = snowstar::CommunicatorSingleton::add(_guideport_callback);
	_guideport->registerCallback(_guideport_identity);
}

NiceGuidePort::~NiceGuidePort() {
	_guideport->unregisterCallback(_guideport_identity);
	snowstar::CommunicatorSingleton::remove(_guideport_identity);
}

uint8_t	NiceGuidePort::active() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "requesting active pins");
	return _guideport->active();
}

void	NiceGuidePort::activate(float raplus, float raminus,
		float decplus, float decminus) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "activating %f %f",
			raplus - raminus, decplus - decminus);
	_guideport->activate(raplus - raminus, decplus - decminus);
}

} // namespace nice
} // namespace camera
} // namespace astro
