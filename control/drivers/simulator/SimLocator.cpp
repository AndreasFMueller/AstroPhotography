/*
 * SimLocator.cpp -- camera locator class for simulator camera
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SimLocator.h>
#include <SimCamera.h>
#include <SimCcd.h>
#include <SimUtil.h>
#include <SimGuiderPort.h>
#include <SimFilterWheel.h>
#include <SimCooler.h>
#include <SimFocuser.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <AstroLoader.h>
#include <AstroExceptions.h>
#include <includes.h>

namespace astro {
namespace module {
namespace simulator {

//////////////////////////////////////////////////////////////////////
// Implementation of the Simulator Module Descriptor
//////////////////////////////////////////////////////////////////////

static std::string      sim_name("simulator");
static std::string      sim_version(VERSION);

/**
 * \brief Module descriptor for the Starlight express module
 */
class SimDescriptor : public ModuleDescriptor {
public:
	SimDescriptor() { }
	~SimDescriptor() { }
        virtual std::string     name() const {
                return sim_name;
        }
        virtual std::string     version() const {
                return sim_version;
        }
	virtual bool	hasDeviceLocator() const {
		return true;
	}
};

} // namespace simulator
} // namespace module
} // namespace astro

extern "C"
astro::module::ModuleDescriptor	*getDescriptor() {
	return new astro::module::simulator::SimDescriptor();
}

namespace astro {
namespace camera {
namespace simulator {

//////////////////////////////////////////////////////////////////////
// Implementation of the Camera Locator for Starlight Express
//////////////////////////////////////////////////////////////////////

SimLocator::SimLocator() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create SimLocator");

	_camera = CameraPtr(new SimCamera(*this));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera: %s",
		_camera->name().toString().c_str());

	_ccd = CcdPtr(new SimCcd(_camera->getCcdInfo(0), *this));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ccd: %s",
		_ccd->name().toString().c_str());

	_guiderport = GuiderPortPtr(new SimGuiderPort(*this));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "guiderport: %s",
		_guiderport->name().toString().c_str());

	_filterwheel = FilterWheelPtr(new SimFilterWheel(*this));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filterwheel: %s",
		_filterwheel->name().toString().c_str());

	_cooler = CoolerPtr(new SimCooler(*this));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cooler: %s",
		_cooler->name().toString().c_str());

	_focuser = FocuserPtr(new SimFocuser(*this));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "fouser: %s",
		_focuser->name().toString().c_str());
}

SimLocator::~SimLocator() {
}

/**
 * \brief Get module name.
 */
std::string	SimLocator::getName() const {
	return std::string("simulator");
}

/**
 * \brief Get module version.
 */
std::string	SimLocator::getVersion() const {
	return VERSION;
}

/**
 * \brief Get a list of simulated devices
 *
 * This module implements exactly one device of every type, which is
 * necessary because the have to interact. E.g. when sending signals
 * to the guider port, the image the camera receives moves. Or when sending
 * signals to the Focuser, the image is blurred.
 * \return a vector of strings that uniquely describe devices
 */
std::vector<std::string>	SimLocator::getDevicelist(
	DeviceName::device_type device) {
	std::vector<std::string>	names;
	switch (device) {
	case DeviceName::AdaptiveOptics:
		throw std::runtime_error("SimAO not implemented");
	case DeviceName::Camera:
		names.push_back(std::string("camera:simulator/camera"));
		break;
	case DeviceName::Ccd:
		names.push_back(std::string("ccd:simulator/camera/ccd"));
		break;
	case DeviceName::Guiderport:
		names.push_back(std::string("guiderport:simulator/guiderport"));
		break;
	case DeviceName::Filterwheel:
		names.push_back(std::string("filterwheel:simulator/filterwheel"));
		break;
	case DeviceName::Focuser:
		names.push_back(std::string("focuser:simulator/focuser"));
		break;
	case DeviceName::Cooler:
		names.push_back(std::string("cooler:simulator/cooler"));
		break;
	case DeviceName::Module:
		names.push_back(std::string("module:simulator"));
		break;
	}
	return names;
}

/**
 * \brief Construct a camera from a camera description
 *
 * \param name		Name of the camera
 * \return Camera with that name
 */
CameraPtr	SimLocator::getCamera0(const DeviceName& name) {
	std::string	sname = name;
	if (sname != "camera:simulator/camera") {
		debug(LOG_ERR, DEBUG_LOG, 0, "camera %s does not exist",
			sname.c_str());
		throw NotFound("no such camera");
	}
	return _camera;
}

CcdPtr	SimLocator::getCcd0(const DeviceName& name) {
	std::string	sname = name;
	if (sname != "ccd:simulator/camera/ccd") {
		debug(LOG_ERR, DEBUG_LOG, 0, "ccd %s does not exist",
			sname.c_str());
		throw NotFound("no such ccd");
	}
	return _ccd;
}

GuiderPortPtr	SimLocator::getGuiderPort0(const DeviceName& name) {
	std::string	sname = name;
	if (sname != "guiderport:simulator/guiderport") {
		debug(LOG_ERR, DEBUG_LOG, 0, "guiderport %s does not exist",
			sname.c_str());
		throw NotFound("no such camera");
	}
	return _guiderport;
}

FilterWheelPtr	SimLocator::getFilterWheel0(const DeviceName& name) {
	std::string	sname = name;
	if (sname != "filterwheel:simulator/filterwheel") {
		debug(LOG_ERR, DEBUG_LOG, 0, "filterwheel %s does not exist",
			sname.c_str());
		throw NotFound("no such camera");
	}
	return _filterwheel;
}

CoolerPtr	SimLocator::getCooler0(const DeviceName& name) {
	std::string	sname = name;
	if (sname != "cooler:simulator/cooler") {
		debug(LOG_ERR, DEBUG_LOG, 0, "cooler %s does not exist",
			sname.c_str());
		throw NotFound("no such cooler");
	}
	return _cooler;
}

FocuserPtr	SimLocator::getFocuser0(const DeviceName& name) {
	std::string	sname = name;
	if (sname != "focuser:simulator/focuser") {
		debug(LOG_ERR, DEBUG_LOG, 0, "focuser %s does not exist",
			sname.c_str());
		throw NotFound("no such focuser");
	}
	return _focuser;
}

SimCamera	*SimLocator::simcamera() {
	return dynamic_cast<SimCamera *>(&*_camera);
}

SimCcd	*SimLocator::simccd() {
	return dynamic_cast<SimCcd *>(&*_ccd);
}

SimFilterWheel	*SimLocator::simfilterwheel() {
	return dynamic_cast<SimFilterWheel *>(&*_filterwheel);
}

SimGuiderPort	*SimLocator::simguiderport() {
	return dynamic_cast<SimGuiderPort *>(&*_guiderport);
}

SimCooler	*SimLocator::simcooler() {
	return dynamic_cast<SimCooler *>(&*_cooler);
}

SimFocuser	*SimLocator::simfocuser() {
	return dynamic_cast<SimFocuser *>(&*_focuser);
}

} // namespace simulator
} // namespace camera
} // namespace astro


extern "C"
astro::device::DeviceLocator    *getDeviceLocator() {
	return new astro::camera::simulator::SimLocator();
}
