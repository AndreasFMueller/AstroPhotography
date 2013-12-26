/*
 * MtLocator.cpp -- microtouch focuser locator
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <MtLocator.h>
#include <AstroDebug.h>
#include <AstroCamera.h>
#include <AstroExceptions.h>
#include <AstroLoader.h>
#include <config.h>

namespace astro {
namespace module {
namespace microtouch {

static std::string	mt_name("microtouch");
static std::string	mt_version(VERSION);

/**
 * \brief Module descriptor for the Microtouch module
 */
class MtDescriptor : public ModuleDescriptor {
public:
	MtDescriptor() { }
	virtual ~MtDescriptor() { }
	virtual std::string	name() const { return mt_name; }
	virtual std::string	version() const { return mt_version; }
	virtual bool	hasDeviceLocator() const { return true; }
};

} // namespace microtouch
} // namespace module
} // namespace astro

extern "C"
astro::module::ModuleDescriptor	*getDescriptor() {
	return new astro::module::microtouch::MtDescriptor();
}

using namespace astro::module::microtouch;
using namespace astro::camera;

namespace astro {
namespace device {
namespace microtouch {

MtLocator::MtLocator() {
}

MtLocator::~MtLocator() {
}

std::string	MtLocator::getName() const {
	return mt_name;
}

std::string	MtLocator::getVersion() const {
	return mt_version;
}

std::vector<std::string>	MtLocator::getDevicelist(
	DeviceName::device_type device) {
	std::vector<std::string>	names;
	if (DeviceName::Focuser != device) {
		return names;
	}
	names.push_back(std::string("focuser:microtouch/focuser"));
	return names;
}

FocuserPtr	MtLocator::getFocuser0(const DeviceName& name) {
	std::string	sname = name;
	if (sname != "focuser:microtouch/focuser") {
		debug(LOG_ERR, DEBUG_LOG, 0, "focuser %s does not exist",
			sname.c_str());
		throw NotFound("no such focuser");
	}
	// create the focuser
	return FocuserPtr();
}

} // namespace microtouch
} // namespace device
} // namespace astro
