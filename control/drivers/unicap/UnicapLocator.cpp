/*
 * UnicapCameraLocator.cpp --
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <UnicapLocator.h>
#include <includes.h>

using namespace astro::camera::unicap;

UnicapCameraLocator::UnicapCameraLocator() {
}

UnicapCameraLocator::~UnicapCameraLocator() {
}

std::string	UnicapCameraLocator::getName() const {
	return std::string("unicap");
}

std::string	UnicapCameraLocator::getVersion() const {
	return VERSION;
}

std::vector<std::string>	UnicapCameraLocator::getCameralist() {
	std::vector<std::string>	cameras;
	return cameras;
}

CameraPtr	UnicapCameraLocator::getCamera(const std::string& name) {
	return CameraPtr();
}
