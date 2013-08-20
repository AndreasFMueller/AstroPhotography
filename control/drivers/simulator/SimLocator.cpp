/*
 * SimLocator.cpp -- camera locator class for simulator camera
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SimLocator.h>
#include <SimCamera.h>
#include <SimUtil.h>
#include <SimGuiderPort.h>
#include <SimFilterWheel.h>
#include <SimCooler.h>
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
	_camera = CameraPtr(new SimCamera(*this));
	_guiderport = GuiderPortPtr(new SimGuiderPort(*this));
	_filterwheel = FilterWheelPtr(new SimFilterWheel(*this));
	_cooler = CoolerPtr(new SimCooler(*this));
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
	DeviceLocator::device_type device) {
	std::vector<std::string>	names;
	switch (device) {
	case DeviceLocator::CAMERA:
		names.push_back(std::string("sim-camera"));
		break;
	case DeviceLocator::GUIDERPORT:
		names.push_back(std::string("sim-guiderport"));
		break;
	case DeviceLocator::FILTERWHEEL:
		names.push_back(std::string("sim-filterwheel"));
		break;
	case DeviceLocator::FOCUSER:
		names.push_back(std::string("sim-focuser"));
		break;
	case DeviceLocator::COOLER:
		names.push_back(std::string("sim-cooler"));
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
CameraPtr	SimLocator::getCamera0(const std::string& name) {
	if (name != "sim-camera") {
		debug(LOG_ERR, DEBUG_LOG, 0, "camera %s does not exist",
			name.c_str());
		throw NotFound("no such camera");
	}
	return _camera;
}

GuiderPortPtr	SimLocator::getGuiderPort0(const std::string& name) {
	if (name != "sim-guiderport") {
		debug(LOG_ERR, DEBUG_LOG, 0, "guiderport %s does not exist",
			name.c_str());
		throw NotFound("no such camera");
	}
	return _guiderport;
}

FilterWheelPtr	SimLocator::getFilterWheel0(const std::string& name) {
	if (name != "sim-filterwheel") {
		debug(LOG_ERR, DEBUG_LOG, 0, "filterwheel %s does not exist",
			name.c_str());
		throw NotFound("no such camera");
	}
	return _filterwheel;
}

CoolerPtr	SimLocator::getCooler0(const std::string& name) {
	if (name != "sim-cooler") {
		debug(LOG_ERR, DEBUG_LOG, 0, "cooler %s does not exist",
			name.c_str());
		throw NotFound("no such cooler");
	}
	return _cooler;
}

SimCamera	*SimLocator::simcamera() {
	return dynamic_cast<SimCamera *>(&*_camera);
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

} // namespace simulator
} // namespace camera
} // namespace astro


extern "C"
astro::device::DeviceLocator    *getDeviceLocator() {
	return new astro::camera::simulator::SimLocator();
}
