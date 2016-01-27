/*
 * CelestronLocator.cpp -- Celestron mount locator
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CelestronLocator.h>
#include <AstroDebug.h>
#include <AstroCamera.h>
#include <AstroExceptions.h>
#include <AstroLoader.h>
#include <config.h>
#include <CelestronMount.h>
#include <AstroFormat.h>

namespace astro {
namespace module {
namespace celestron {

static std::string	celestron_name("celestron");
static std::string	celestron_version(VERSION);

/**
 * \brief Module descriptor for the Microtouch module
 */
class CelestronDescriptor : public ModuleDescriptor {
public:
	CelestronDescriptor() { }
	virtual ~CelestronDescriptor() { }
	virtual std::string	name() const { return celestron_name; }
	virtual std::string	version() const { return celestron_version; }
	virtual bool	hasDeviceLocator() const { return true; }
};

} // namespace celestron
} // namespace module
} // namespace astro

extern "C"
astro::module::ModuleDescriptor	*getDescriptor() {
	return new astro::module::celestron::CelestronDescriptor();
}

using namespace astro::module::celestron;
using namespace astro::device;

namespace astro {
namespace device {
namespace celestron {

CelestronLocator::CelestronLocator() {
}

CelestronLocator::~CelestronLocator() {
}

std::string	CelestronLocator::getName() const {
	return celestron_name;
}

std::string	CelestronLocator::getVersion() const {
	return celestron_version;
}

std::vector<std::string>	CelestronLocator::getDevicelist(
	DeviceName::device_type device) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve device list");
	std::vector<std::string>	names;
	if (DeviceName::Mount != device) {
		return names;
	}
	// no check whether we have properties for celestron devices
	for (int unit = 0; unit < 4; unit++) {
		std::string	devicename
			= stringprintf("mount:celestron/%d", unit);
		Properties	properties(devicename);
		if (properties.hasProperty("device")) {
			names.push_back(devicename);
		}
	}
	return names;
}

MountPtr	CelestronLocator::getMount0(const DeviceName& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get the celestron mount '%s'",
		name.toString().c_str());

	CelestronMount	*mount = new CelestronMount(name);

	return MountPtr(mount);
}

} // namespace celestron
} // namespace device
} // namespace astro

extern "C"
astro::device::DeviceLocator    *getDeviceLocator() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve device locator");
        return new astro::device::celestron::CelestronLocator();
}

