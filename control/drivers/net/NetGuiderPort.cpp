/*
 * NetGuiderPort.cpp -- network based guider port interface
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <NetGuiderPort.h>
#include <Conversions.h>

namespace astro {
namespace camera {
namespace net {

NetGuiderPort::NetGuiderPort(Astro::GuiderPort_var guiderport)
	: _guiderport(guiderport) {
	Astro::GuiderPort_Helper::duplicate(_guiderport);
}

NetGuiderPort::~NetGuiderPort() {
	Astro::GuiderPort_Helper::release(_guiderport);
}

uint8_t	NetGuiderPort::active() {
	return convert_octet2relaybits(_guiderport->active());
}

void	NetGuiderPort::activate(float raplus, float raminus,
				float decplus, float decminus) {
	_guiderport->activate(raplus - raminus, decplus - decminus);
}


} // namespace net
} // namespace camera
} // namespace astro
