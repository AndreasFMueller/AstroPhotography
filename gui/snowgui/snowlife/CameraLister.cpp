/*
 * CameraLister.cpp -- implementation of CameraLister class
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "CameraLister.h"
#include <AstroDebug.h>
#include <AstroLoader.h>

using namespace astro::device;
using namespace astro::module;

namespace snowgui {

/**
 * \brief Construct a camera lister
 */
CameraLister::CameraLister(QObject *parent) : QThread(parent) {
}

/**
 * \brief Destroy the camera lister
 */
CameraLister::~CameraLister() {
}

/**
 * \brief Add cameras from a specified locator
 *
 * This method retrieves a list of all cameras available from the 
 * locator and emits the camera name through the camera signal
 *
 * \param locator	DeviceLocator for the cameras
 */
void	CameraLister::addCameras(DeviceLocatorPtr locator) {
	auto	names = locator->getDevicelist(astro::DeviceName::Ccd);
	for (auto n = names.begin(); n != names.end(); n++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "CCD '%s' found",
			n->c_str());
		emit camera(*n);
	}
}

/**
 * \brief Add focusers from a specified locator
 *
 * This method retrieves a list of all focusers available from the 
 * locator and emits the focuser name through the focuser signal
 *
 * \param locator	DeviceLocator for the focusers
 */
void	CameraLister::addFocusers(DeviceLocatorPtr locator) {
	auto	names = locator->getDevicelist(astro::DeviceName::Focuser);
	for (auto n = names.begin(); n != names.end(); n++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "focuser '%s' found",
			n->c_str());
		emit focuser(*n);
	}
}

/**
 * \brief Run method for the camera lister thread
 */
void	CameraLister::run() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "listing cameras");
	// get a module repository
	ModuleRepositoryPtr	repo = getModuleRepository();

	// go through all the modules
	std::vector<std::string>	modulenames = repo->moduleNames();
	for (auto m = modulenames.begin(); m != modulenames.end(); m++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "scanning module %s",
			m->c_str());
		ModulePtr	module = repo->getModule(*m);
		if (module->getDescriptor()->hasDeviceLocator()) {
			auto	locator = module->getDeviceLocator();
			addCameras(locator);
			addFocusers(locator);
		}
		
	}
}

} // namespace snowgui
