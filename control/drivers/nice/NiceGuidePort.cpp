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
}

NiceGuidePort::~NiceGuidePort() {
}

uint8_t	NiceGuidePort::active() {
	return _guideport->active();
}

void	NiceGuidePort::activate(float raplus, float raminus,
		float decplus, float decminus) {
	return _guideport->activate(raplus - raminus, decplus - decminus);
}

} // namespace nice
} // namespace camera
} // namespace astro
