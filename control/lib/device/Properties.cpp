/*
 * Properties.cpp -- properties interface implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDevice.h>
#include <stdlib.h>
#include <fstream>
#include <AstroDebug.h>
#include <stdexcept>
#include <AstroUtils.h>

namespace astro {

/**
 * \brief Properties Constructor
 */
Properties::Properties(const std::string& devicename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create properties for device '%s'",
		devicename.c_str());
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "trying system file: %s",
			DEVICEPROPERTIES);
		setup(name, DEVICEPROPERTIES);
	} catch (...) { }
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"trying local file: device.properties");
		setup(name, "device.properties");
	} catch (...) { }
	try {
		char	*filename = getenv("DEVICEPROPERTIES");
		if (NULL != filename) {
			setup(name, filename);
		}
	} catch (...) { }
}

Properties::~Properties() {
}

static std::string	removecomments(const std::string& s) {
	size_t	comment = s.find('#');
	if (comment == std::string::npos) {
		return s;
	}
	return s.substr(0, comment);
}

static std::string	standardize(const std::string& s) {
	return trim(removecomments(s));
}

class property_triple {
public:
	property_triple(const std::string& b);
	std::string	devicename;
	std::string	property;
	std::string	value;
	std::string	toString() const {
		return devicename + "." + property + "=" + value;
	}
};

class	badproperty {
public:
};

/**
 * \brief parse a buffer into a property triple
 */
property_triple::property_triple(const std::string& buffer) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating property from '%s'",
		buffer.c_str());
	std::string	b = standardize(buffer);
	size_t	equalsign = b.rfind('=');
	if (std::string::npos == equalsign) {
		throw badproperty();
	}
	value = trim(b.substr(equalsign + 1));
	std::string	key = trim(b.substr(0, equalsign));
	size_t	position = key.find_last_of(" \t");
	if (std::string::npos == position) {
		throw badproperty();
	}
	devicename = standardize(key.substr(0, position));
	property = standardize(key.substr(position + 1));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found triple: device = '%s', "
		"property = '%s', value = '%s'",
		devicename.c_str(), property.c_str(), value.c_str());
}

/**
 * \brief initialize the properties
 */
void	Properties::setup(const std::string& name, const std::string& filename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "reading properties from file '%s'",
		filename.c_str());
	std::ifstream	in(filename.c_str());
	if (!in) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot open file '%s'",
			filename.c_str());
		return;
	}
	static const size_t	buffer_size = 10240;
	char	b[buffer_size];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "file open, start reading, size %d",
		sizeof(b));
	while (!in.eof()) {
		in.getline(b, buffer_size);
		std::string	buffer(b);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "got buffer: '%s'",
			buffer.c_str());
		try {
			property_triple	t(buffer);
			if (name == t.devicename) {
				setProperty(t.property, t.value);
			} else {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"%s != %s", name.c_str(),
					t.devicename.c_str());
			}
		} catch (const std::exception& x) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "exception during "
				"property creation: %s", x.what());
		}
	}
	in.close();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "properties read");
}

/**
 * \brief Check whether the property is set
 */
bool	Properties::hasProperty(const std::string& name) const {
	return (properties.end() != properties.find(name));
}

/**
 * \brief Get property value throwing an exception if not set
 */
std::string	Properties::getProperty(const std::string& name) const {
	propertymap_type::const_iterator	pi = properties.find(name);
	if (pi == properties.end()) {
		throw std::runtime_error("property not available");
	}
	return pi->second;
}

/**
 * \brief Get the property value with a default
 */
std::string	Properties::getProperty(const std::string& name,
				const std::string& defaultvalue) const {
	propertymap_type::const_iterator	pi = properties.find(name);
	if (pi == properties.end()) {
		return defaultvalue;
	}
	return pi->second;
}

/**
 * \brief set a property
 */
void	Properties::setProperty(const std::string& name,
		const std::string& value) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding %s -> %s",
		name.c_str(), value.c_str());
	properties.insert(std::make_pair(name, value));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "has value: %s",
		hasProperty(name) ? "YES" : "NO");
}

} // namespace astro
