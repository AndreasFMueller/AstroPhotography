/*
 * DeviceName.cpp -- An abstraction for device names
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDevice.h>
#include <AstroFormat.h>
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <Nice.h>
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include <AstroDiscovery.h>

namespace astro {

DeviceName::DeviceName(const std::string& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "parsing name '%s'", name.c_str());
	// parse the device URL
	std::string::size_type	pos = name.find(":");
	if (pos == std::string::npos) {
		std::string	msg = stringprintf("device name '%s' lacks ':'",
			name.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	_type = string2type(name.substr(0, pos));
	std::string	path = name.substr(pos + 1);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "path: %s", path.c_str());
	split<DeviceName>(path, "/", *this);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "have %d components", size());
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

DeviceName::DeviceName(const device_type& type,
	const std::string& modulename, const std::string& unitname)
	: _type(type) {
	push_back(modulename);
	push_back(unitname);
}

DeviceName::DeviceName(const DeviceName& name, const device_type& type,
	const std::string& unitname) : _type(type) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "copy %d device name components",
		name.size());
	std::copy(name.begin(), name.end(), back_inserter(*this));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "unit name = %s", unitname.c_str());
	push_back(unitname);
}

DeviceName::DeviceName(const DeviceName& other)
	: std::vector<std::string>(other), _type(other.type()) {
}

DeviceName&	DeviceName::operator=(const DeviceName& other) {
	std::copy(other.begin(), other.end(), begin());
	_type = other._type;
	return *this;
}

const std::string&	DeviceName::unitname() const {
	return this->back();
}

void	DeviceName::unitname(const std::string& u) {
	if (size() == 0) {
		throw std::runtime_error("name empty, can't replace unit name");
	}
	pop_back();
	push_back(u);
}

const std::string&	DeviceName::enclosurename() const {
	size_t	offset = 1;
	if (isNetworkDevice()) {
		offset += 1;
	}
	if (size() <= offset) {
		throw std::runtime_error("no enclosure name");
	}
	return this->operator[](offset);
}

void	DeviceName::enclosurename(const std::string& n) {
	size_t	offset = 1;
	if (isNetworkDevice()) {
		offset += 1;
	}
	if (n.size() <= offset) {
		throw std::runtime_error("no enclosure name");
	}
	this->operator[](offset) = n;
}

const std::string&	DeviceName::modulename() const {
	if (size() == 0) {
		throw std::runtime_error("empty name");
	}
	return this->front();
}

void	DeviceName::modulename(const std::string& m) {
	if (size() == 0) {
		throw std::runtime_error("empty name");
	}
	this->operator[](0) = m;
}

const std::string&	DeviceName::hostname() const {
	if (!isNetworkDevice()) {
		throw std::runtime_error("not a network device");
	}
	if (size() < 2) {
		throw std::runtime_error("no hostname present");
	}
	return this->operator[](1);
}

void	DeviceName::hostname(const std::string& h) {
	if (!isNetworkDevice()) {
		throw std::runtime_error("not a network device");
	}
	if (size() < 2) {
		throw std::runtime_error("no hostname present");
	}
	this->operator[](1) = 1;
}

std::string	DeviceName::name() const {
	Concatenator	c("/");
	std::for_each(++begin(), end(), c);
	return c;
}

std::string	DeviceName::toString() const {
	std::ostringstream	out;
	out << *this;
	return out.str();
}

/**
 * \brief Type conversion from name to type code
 */
#define	Ntypes	9
static std::string	typenames[Ntypes] = {
	"adaptiveoptics",
	"camera",
	"ccd",
	"cooler",
	"filterwheel",
	"focuser",
	"guideport",
	"module",
	"mount"
};
static DeviceName::device_type	typecode[Ntypes] = {
	DeviceName::AdaptiveOptics,
	DeviceName::Camera,
	DeviceName::Ccd,
	DeviceName::Cooler,
	DeviceName::Filterwheel,
	DeviceName::Focuser,
	DeviceName::Guideport,
	DeviceName::Module,
	DeviceName::Mount
};

DeviceName::device_type	DeviceName::string2type(const std::string& name) {
	for (int i = 0; i < Ntypes; i++) {
		if (typenames[i] == name) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "type %s mapped to %d",
				name.c_str(), i);
			return typecode[i];
		}
	}
	std::string	msg = stringprintf("type '%s' not found", name.c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "converting name");
	std::string	result = typestring();
	result.append(":");
	result.append(Concatenator::concat(*this, std::string("/")));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "converted name: %s", result.c_str());
	return result;
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
	std::string	n = name;
	return out << n;
}

DeviceName	DeviceName::parent(const DeviceName::device_type& devicetype) const {
	DeviceName	result;
	const_iterator	i = begin();
	size_type	j = 1;
	while (j++ < size()) {
		result.push_back(*i);
		i++;
	}
	result.type(devicetype);
	return result;
}

DeviceName	DeviceName::child(const DeviceName::device_type& devicetype,
			const std::string& unitname) const {
	return DeviceName(*this, devicetype, unitname);
}

bool	DeviceName::isNetworkDevice() const {
	return (modulename() == "nice");
}

bool	DeviceName::isLocalDevice() const {
	return !isNetworkDevice();
}

bool	DeviceName::isServedByUs() const {
	if (isLocalDevice()) {
		return false;
	}
	std::string	service = servicename();
	return astro::discover::ServicePublisher::ispublished(service);
}

DeviceName	DeviceName::localdevice() const {
	if (isLocalDevice()) {
		return *this;
	}
	astro::device::nice::DeviceDenicer	d(*this);
	return d.devicename();
}

DeviceName	DeviceName::netdevice(const std::string& service) const {
	if (isNetworkDevice()) {
		return *this;
	}
	astro::device::nice::DeviceNicer	n(service);
	return n(*this);
}

const std::string&	DeviceName::servicename() const {
	if (isLocalDevice()) {
		throw std::logic_error("not a network device");
	}
	return operator[](1);
}

bool	DeviceName::isServedBy(const std::string& service) const {
	if (isLocalDevice()) {
		return false;
	}
	return servicename() == service;
}

} // namespace astro
