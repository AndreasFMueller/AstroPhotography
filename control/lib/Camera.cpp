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

Camera::Camera(const std::string& name) : Device(name) {
}

Camera::~Camera() {
}

unsigned int	Camera::nCcds() const {
	return ccdinfo.size();
}

const CcdInfo&	Camera::getCcdInfo(size_t ccdid) const {
	return ccdinfo[ccdid];
}

FilterWheelPtr	Camera::getFilterWheel0() throw (not_implemented) {
	throw not_implemented("filter wheel not implemented");
}

/**
 * \brief Get FilterWheel, using the cached object if available
 */
FilterWheelPtr	Camera::getFilterWheel() throw (not_implemented) {
	if (!filterwheel) {
		filterwheel = this->getFilterWheel0();
	}
	return filterwheel;
}

GuiderPortPtr	Camera::getGuiderPort0() throw (not_implemented) {
	throw not_implemented("guider port not implemented");
}

/**
 * \brief Get GuiderPort, using the cached object if available
 */
GuiderPortPtr	Camera::getGuiderPort() throw (not_implemented) {
	if (!guiderport) {
		guiderport = this->getGuiderPort0();
	}
	return guiderport;
}

} // namespace camera
} // namespace astro
