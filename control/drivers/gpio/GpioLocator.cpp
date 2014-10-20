/*
 * GpioLocator.cpp -- Gpio Guiderport locator
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <GpioLocator.h>
#include <AstroDebug.h>
#include <AstroCamera.h>
#include <AstroExceptions.h>
#include <AstroLoader.h>
#include <config.h>
#include <GpioGuiderport.h>

namespace astro {
namespace module {
namespace gpio {

static std::string	gpio_name("gpio");
static std::string	gpio_version(VERSION);

/**
 * \brief Module descriptor for the Microtouch module
 */
class GpioDescriptor : public ModuleDescriptor {
public:
	GpioDescriptor() { }
	virtual ~GpioDescriptor() { }
	virtual std::string	name() const { return gpio_name; }
	virtual std::string	version() const { return gpio_version; }
	virtual bool	hasDeviceLocator() const { return true; }
};

} // namespace gpio
} // namespace module
} // namespace astro

extern "C"
astro::module::ModuleDescriptor	*getDescriptor() {
	return new astro::module::gpio::GpioDescriptor();
}

using namespace astro::module::gpio;
using namespace astro::device;
using namespace astro::camera;

namespace astro {
namespace device {
namespace gpio {

GpioLocator::GpioLocator() {
}

GpioLocator::~GpioLocator() {
}

std::string	GpioLocator::getName() const {
	return gpio_name;
}

std::string	GpioLocator::getVersion() const {
	return gpio_version;
}

std::vector<std::string>	GpioLocator::getDevicelist(
	DeviceName::device_type device) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve device list");
	std::vector<std::string>	names;
	if (DeviceName::Guiderport != device) {
		return names;
	}
	names.push_back(std::string("guiderport:gpio/0"));
	return names;
}

GuiderPortPtr	GpioLocator::getGuiderPort0(const DeviceName& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get the gpio guiderport '%s'",
		name.toString().c_str());

	GpioGuiderport	*guiderport = new GpioGuiderport(name);

	return GuiderPortPtr(guiderport);
}

} // namespace gpio
} // namespace device
} // namespace astro

extern "C"
astro::device::DeviceLocator    *getDeviceLocator() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve device locator");
        return new astro::device::gpio::GpioLocator();
}

