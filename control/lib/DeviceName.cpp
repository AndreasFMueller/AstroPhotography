/*
 * DeviceName.cpp -- An abstraction for device names
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDevice.h>
#include <AstroFormat.h>
#include <AstroUtils.h>
#include <AstroDebug.h>

namespace astro {

DeviceName::DeviceName(const std::string& name) {
	// parse the device URL
	std::string::size_type	pos = name.find(":");
	_type = string2type(name.substr(0, pos));
	std::string	path = name.substr(pos + 1);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "path: %s", path.c_str());
	split<DeviceName>(path, "/", *this);
}

DeviceName::DeviceName(const std::string& modulename,
	const std::string& unitname) : _type(Camera) {
	push_back(modulename);
	push_back(unitname);
}

DeviceName::DeviceName(const device_type& type,
	const std::vector<std::string>& components)
	: _type(type) {
	std::copy(components.begin(), components.end(), back_inserter(*this));
}

DeviceName::DeviceName(const DeviceName& name, const device_type& type,
	const std::string& unitname) : _type(type) {
	std::copy(name.begin(), name.end(), back_inserter(*this));
	push_back(unitname);
}

const std::string&	DeviceName::modulename() const {
	return this->front();
}

const std::string&	DeviceName::unitname() const {
	return this->back();
}

std::string	DeviceName::name() const {
	Concatenator	c("/");
	std::for_each(++begin(), end(), c);
	return c;
}

/**
 * \brief Type conversion from name to type code
 */
#define	Ntypes	6
static std::string	typenames[Ntypes] = {
	"camera",
	"ccd",
	"cooler",
	"filterwheel",
	"guiderport",
	"focuser"
};
static DeviceName::device_type	typecode[Ntypes] = {
	DeviceName::Camera,
	DeviceName::Ccd,
	DeviceName::Cooler,
	DeviceName::Filterwheel,
	DeviceName::Guiderport,
	DeviceName::Focuser
};

DeviceName::device_type	DeviceName::string2type(const std::string& name) {
	for (int i = 0; i < Ntypes; i++) {
		if (typenames[i] == name) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "type %s mapped to %d",
				name.c_str(), i);
			return typecode[i];
		}
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "type '%s' not found", name.c_str());
	throw std::runtime_error("type not found");
}

/**
 * \brief Type field conversion from type code to string
 */
std::string	DeviceName::type2string(const device_type& type) {
	for (int i = 0; i < Ntypes; i++) {
		if (typecode[i] == type) {
			return std::string(typenames[i]);
		}
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "typecode '%d' not found", type);
	throw std::runtime_error("type code not found");
}

std::string	DeviceName::typestring() const {
	return DeviceName::type2string(type());
}

void	DeviceName::typestring(const std::string& t) {
	type(DeviceName::string2type(t));
}

bool	DeviceName::hasType(const device_type& t) const {
	return _type == t;
}

DeviceName::operator std::string() const {
	return typestring() + ":" + Concatenator::concat(*this, "/");
}

class comparator {
public:
	bool	operator()(const std::string& a, const std::string& b) const;
};

bool	DeviceName::operator==(const DeviceName& other) const {
	if (type() != other.type()) {
		return false;
	}
	if (size() != other.size()) {
		return false;
	}
	return std::equal(begin(), end(), other.begin());
}

bool	DeviceName::operator!=(const DeviceName& other) const {
	return !(*this == other);
}

bool	DeviceName::operator<(const DeviceName& other) const {
	if (type() < other.type()) {
		return true;
	}
	if (type() == other.type()) {
		return lexicographical_compare(begin(), end(),
						other.begin(), other.end());
	}
	return false;
}

std::ostream&	operator<<(std::ostream& out, const DeviceName& name) {
	return out << (std::string)name;
}

} // namespace astro
