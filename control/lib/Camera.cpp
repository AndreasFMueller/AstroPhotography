/*
 * Camera.cpp -- camera base class implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>

namespace astro {
namespace camera {

Camera::Camera() {
}

Camera::~Camera() {
}

unsigned int	Camera::nCcds() const {
	return ccdinfo.size();
}

const CcdInfo&	Camera::getCcdInfo(size_t ccdid) const {
	return ccdinfo[ccdid];
}

FilterWheelPtr	Camera::getFilterWheel() throw (not_implemented) {
	throw not_implemented("filter wheel not implemented");
}

} // namespace camera
} // namespace astro
