/*
 * Guider.cpp -- the main guider process class
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>

namespace astro {
namespace guiding {

Guider::Guider(GuiderPortPtr _guiderport, CameraPtr _camera)
	: camera(_camera), guiderport(_guiderport) {
}




} // namespace guiding
} // namespace astro
