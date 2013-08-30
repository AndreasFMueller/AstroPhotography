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

/** 
 * \brief Create a network GuiderPort client
 *
 * The constructor duplicates a reference to a remote guider port, which
 * is kept by the client class until it is destroyed again
 */
NetGuiderPort::NetGuiderPort(Astro::GuiderPort_var guiderport)
	: _guiderport(guiderport) {
	Astro::GuiderPort_Helper::duplicate(_guiderport);
}

/**
 * \brief Destroy the GuiderPort object
 *
 * release the reference to the remote guider port
 */
NetGuiderPort::~NetGuiderPort() {
	Astro::GuiderPort_Helper::release(_guiderport);
}

/**
 * \brief Test which guider port outputs are currently activated
 */
uint8_t	NetGuiderPort::active() {
	return convert_octet2relaybits(_guiderport->active());
}

/**
 * \brief Activate guider port ports for a given time.
 */
void	NetGuiderPort::activate(float raplus, float raminus,
				float decplus, float decminus) {
	_guiderport->activate(raplus - raminus, decplus - decminus);
}


} // namespace net
} // namespace camera
} // namespace astro
