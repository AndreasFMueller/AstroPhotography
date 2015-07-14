/*
 * NiceGuiderPort.cpp
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <NiceGuiderPort.h>

namespace astro {
namespace camera {
namespace nice {

NiceGuiderPort::NiceGuiderPort(snowstar::GuiderPortPrx guiderport,
	const DeviceName& devicename)
	: GuiderPort(devicename), NiceDevice(devicename),
	  _guiderport(guiderport) {
}

NiceGuiderPort::~NiceGuiderPort() {
}

uint8_t	NiceGuiderPort::active() {
	return _guiderport->active();
}

void	NiceGuiderPort::activate(float raplus, float raminus,
		float decplus, float decminus) {
	return _guiderport->activate(raplus - raminus, decplus - decminus);
}

} // namespace nice
} // namespace camera
} // namespace astro
