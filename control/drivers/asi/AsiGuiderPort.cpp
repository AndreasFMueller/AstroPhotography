/*
 * AsiGuiderPort.cpp -- ASI guider port implementation
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AsiGuiderPort.h>
#include <AstroDebug.h>
#include <utils.h>

namespace astro {
namespace camera {
namespace asi {

AsiGuiderPort::AsiGuiderPort(AsiCamera& camera)
	: GuiderPort(asiGuiderportName(camera.index())), _camera(camera) {
}

AsiGuiderPort::~AsiGuiderPort() {
}

uint8_t	AsiGuiderPort::active() {
	return 0;
}

void	AsiGuiderPort::activate(float raplus, float raminus,
		float decplus, float decminus) {
	// XXX implementation missing
}

} // namespace asi
} // namespace camera
} // namespace astro
