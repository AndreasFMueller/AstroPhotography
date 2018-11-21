/*
 * NiceGuidePort.cpp
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <NiceGuidePort.h>

namespace astro {
namespace camera {
namespace nice {

NiceGuidePort::NiceGuidePort(snowstar::GuidePortPrx guideport,
	const DeviceName& devicename)
	: GuidePort(devicename), NiceDevice(devicename),
	  _guideport(guideport) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "niceguideport constructed");
}

NiceGuidePort::~NiceGuidePort() {
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
