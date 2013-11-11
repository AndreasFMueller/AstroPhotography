/*
 * Camera.cpp -- camera base class implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>
#include <AstroExceptions.h>

namespace astro {
namespace camera {

//////////////////////////////////////////////////////////////////////
// CameraDeviceAdapter implementation
//////////////////////////////////////////////////////////////////////
template<>
CcdPtr	CameraDeviceAdapter<Ccd>::get(const DeviceName& name) {
	return _camera->getCcd(name);
}

template<>
GuiderPortPtr	CameraDeviceAdapter<GuiderPort>::get(const DeviceName& name) {
	return _camera->getGuiderPort();
}

template<>
FilterWheelPtr	CameraDeviceAdapter<FilterWheel>::get(const DeviceName& name) {
	return _camera->getFilterWheel();
}

//////////////////////////////////////////////////////////////////////
// Camera implementation
//////////////////////////////////////////////////////////////////////
DeviceName::device_type	Camera::devicetype = DeviceName::Camera;

DeviceName	Camera::defaultname(const DeviceName& parent,
			const std::string& unitname) {
	return DeviceName(parent, DeviceName::Camera, unitname);
}

Camera::Camera(const std::string& name) : Device(name) {
}

Camera::Camera(const DeviceName& name) : Device(name) {
}

Camera::~Camera() {
}

/**
 * \brief Default implementation of reset does nothing
 *
 * Most camera drivers cannot reset a camera, because camera vendors most
 * often forgot this function.
 */
void	Camera::reset() {
}

/**
 * \brief Get the number of CCDs this camera has
 */
unsigned int	Camera::nCcds() const {
	return ccdinfo.size();
}

/**
 * \brief Get the info object for a CCD
 */
const CcdInfo&	Camera::getCcdInfo(size_t ccdid) const {
	if (ccdid > ccds.size()) {
		throw NotFound("ccd id too large");
	}
	return ccdinfo[ccdid];
}

/**
 * \brief Get A Ccd from the cache, if available
 */
CcdPtr	Camera::getCcd(size_t ccdid) {
	// ensure empty pointers are present
	if (ccds.size() < nCcds()) {
		for (unsigned int i = 0; i < nCcds(); i++) {
			ccds.push_back(CcdPtr());
		}
	}

	// make sure the index is reasonable
	if (ccdid >= nCcds()) {
		throw NotFound("ccd id too large");
	}

	// get the ccd from the cache
	CcdPtr	ccd = ccds[ccdid];
	if (!ccd) {
		ccd = this->getCcd0(ccdid);
		ccds[ccdid] = ccd;
	}
	return ccd;
}

/**
 * \brief Get a Ccd by name
 */
CcdPtr	Camera::getCcd(const DeviceName& ccdname) {
	for (int i = 0; i < nCcds(); i++) {
		if (getCcdInfo(i).name() == ccdname) {
			return getCcd(i);
		}
	}
	throw std::invalid_argument("no ccd with this name found");
}

/**
 * \brief Default FilterWheel implementation just throws an exception
 */
FilterWheelPtr	Camera::getFilterWheel0() {
	throw NotImplemented("filter wheel not implemented");
}

/**
 * \brief Get FilterWheel, using the cached object if available
 */
FilterWheelPtr	Camera::getFilterWheel() {
	if (!this->hasFilterWheel()) {
		throw NotImplemented("cannot request filter wheel");
	}
	if (!filterwheel) {
		filterwheel = this->getFilterWheel0();
	}
	return filterwheel;
}

/**
 * \brief Default GuiderPort implementation just throws an exception
 */
GuiderPortPtr	Camera::getGuiderPort0() {
	throw NotImplemented("guider port not implemented");
}

/**
 * \brief Get GuiderPort, using the cached object if available
 */
GuiderPortPtr	Camera::getGuiderPort() {
	if (!this->hasGuiderPort()) {
		throw NotImplemented("cannot request guider port");
	}
	if (!guiderport) {
		guiderport = this->getGuiderPort0();
	}
	return guiderport;
}

} // namespace camera
} // namespace astro
