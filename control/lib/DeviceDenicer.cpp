/*
 * DeviceDenicer.cpp -- DeviceDenicer class implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Nice.h>
#include <AstroDebug.h>

namespace astro {
namespace device {
namespace nice {

void	DeviceDenicer::setup(const DeviceName& original) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "denice '%s'",
		original.toString().c_str());
	if (original.modulename() != "nice") {
		throw std::runtime_error("cannot denice device names for "
			"other modules");
	}
	if (original.size() < 2) {
		throw std::runtime_error("bad nice device name: too short");
	}
	std::vector<std::string>	components;
	std::vector<std::string>::const_iterator	i = original.begin();
	i++; // skip the name nice
	_service = *i;
	i++; // skip the service name
	copy(i, original.end(), std::back_inserter(components));
	_devicename = std::shared_ptr<DeviceName>(new DeviceName(original.type(),
		components));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "deniced service name: %s",
		_devicename->toString().c_str());
}

DeviceDenicer::DeviceDenicer(const std::string& device) {
	DeviceName	original(device);
	setup(original);
}

DeviceDenicer::DeviceDenicer(const DeviceName& original) {
	setup(original);
}

std::string	DeviceDenicer::service() const {
	return _service;
}

DeviceName	DeviceDenicer::devicename() const {
	return *_devicename;
}

} // namespace nice
} // namespace device
} // namespace astro
