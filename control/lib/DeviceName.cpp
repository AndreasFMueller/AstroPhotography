/*
 * DeviceName.cpp -- An abstraction for device names
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTypes.h>
#include <AstroFormat.h>

namespace astro {

DeviceName::DeviceName(const std::string& name) {
	// split name
	size_t	offset = name.find(':');
	if (std::string::npos == offset) {
		throw std::range_error("not ':' character in device name");
	}
	_modulename = name.substr(0, offset);
	_unitname = name.substr(offset + 1);
}

DeviceName::operator std::string() const {
	return stringprintf("%s:%s", modulename().c_str(), unitname().c_str());
}

bool	DeviceName::operator==(const DeviceName& other) const {
	return (modulename() == other.modulename())
		&& (unitname() == other.unitname());
}

bool	DeviceName::operator!=(const DeviceName& other) const {
	return !(*this == other);
}

bool	DeviceName::operator<(const DeviceName& other) const {
	if (modulename() < other.modulename()) {
		return true;
	}
	if (modulename() > other.modulename()) {
		return false;
	}
	return unitname() < other.unitname();
}

} // namespace astro
