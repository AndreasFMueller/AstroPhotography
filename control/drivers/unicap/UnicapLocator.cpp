/*
 * UnicapLocator.cpp --
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <UnicapLocator.h>
#include <includes.h>

UnicapLocator::UnicapLocator() {
}

UnicapLocator::~UnicapLocator() {
}

std::string	UnicapLocator::getName() const {
	return std::string("unicap");
}

std::string	UnicapLocator::getVersion() const {
	return VERSION;
}

std::vector<std::string>	UnicapLocator::getCameralist() {
	std::vector<std::string>	cameras;
	return cameras;
}

CameraPtr	UncaipLocator::getCamera(const std::string& name) {
	return CameraPtr();
}
