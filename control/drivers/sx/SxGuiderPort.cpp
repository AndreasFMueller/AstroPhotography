/*
 * SxGuiderPort.cpp -- guider port implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SxGuiderPort.h>

namespace astro {
namespace camera {
namespace sx {

SxGuiderPort::SxGuiderPort(SxCamera& _camera) : camera(_camera) {
}

SxGuiderPort::~SxGuiderPort() {
}

uint8_t	SxGuiderPort::active() {
	// XXX implementation missing
	return 0;
}

void	SxGuiderPort::activate(float raplus, float raminus,
		float decplus, float decminus) {
	// XXX implementation missing
}

} // namespace sx
} // namespace camera
} // namespace astro
