/*
 * Focusing.cpp -- implementation of auto focusing
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroFocus.h>

using namespace astro::camera;

namespace astro {
namespace focusing {

Focusing::Focusing(CameraPtr camera, FocuserPtr focuser)
	: _camera(camera), _focuser(focuser) {
	_mode = TWO_SIDED;
	_status = IDLE;
}

void	Focusing::start() {
}

void	Focusing::cancel() {
}

} // namespace focusing
} // namespace astro
