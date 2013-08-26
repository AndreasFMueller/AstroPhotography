/*
 * NetGuiderPort.cpp -- 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <NetGuiderPort.h>

namespace astro {
namespace camera {
namespace net {

NetGuiderPort::NetGuiderPort(Astro::GuiderPort_var guiderport)
	: _guiderport(guiderport) {
}

} // namespace net
} // namespace camera
} // namespace astro
