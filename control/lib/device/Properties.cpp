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
#include <includes.h>

namespace astro {

/**
 * \brief Properties Constructor
 *
 * \param devicename	The device name to use while scanning the properties
 */
Properties::Properties(const std::string& devicename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create properties for device '%s'",
		devicename.c_str());
	// try system file
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "trying system file: %s",
			DEVICEPROPERTIES);
		setup(devicename, DEVICEPROPERTIES);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "read %s", DEVICEPROPERTIES);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "system file %s not usable: %s",
			DEVICEPROPERTIES, x.what());
	}
	// system directory
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "trying system directory: %s",
			DEVICEPROPERTYDIR);
		setupDir(devicename, DEVICEPROPERTYDIR);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "read directory %s",
			DEVICEPROPERTYDIR);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "error in directory %s: %s",
			DEVICEPROPERTYDIR, x.what());
	}
	// device.properties in the present directory (what for?)
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"trying local file: device.properties");
		setup(devicename, "device.properties");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "read device.properties");
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "system file %s not usable: %s",
			"device.properties", x.what());
	}
	// file from environment
	try {
		char	*filename = getenv("DEVICEPROPERTIES");
		if (NULL != filename) {
			setup(devicename, filename);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "read %s", filename);
		}
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "system file %s not usable: %s",
			getenv("DEVICEPROPERTIES"), x.what());
	}
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
		std::string	buffer = standardize(std::string(b));
		if (buffer.size() == 0) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"skip empty line: %s", b);
			continue;
		}
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
 * \brief Set up properties from a directory
 *
 * \param name		device name to look for
 * \param dirname	directory path to scan
 */
void	Properties::setupDir(const std::string& name,
		const std::string& dirname) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "scanning directory %s",
		dirname.c_str());
	// open the directory
	DIR     *dir = opendir(dirname.c_str());
	if (NULL == dir) {
		std::string	msg = stringprintf("cannot open property file "
			"directory %s: %s", dirname.c_str(), strerror(errno));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// scan the directory
	struct dirent	*direntp = NULL;
	struct dirent	direntry;
	std::list<std::string>	files;
	do {
		int	rc = readdir_r(dir, &direntry, &direntp);
		if (rc) {
			std::string	msg = stringprintf("cannt read "
				"property directory %s: %s", dirname.c_str(),
				strerror(errno));
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			closedir(dir);
			throw std::runtime_error(msg);
		}
		if (NULL == direntp)
			continue;
		int     namelen = strlen(direntp->d_name);
		if (namelen < 12)
			continue;
		if (0 == strcmp(".properties", direntp->d_name + namelen - 11)) {
			std::string     filename = dirname + "/"
						+ std::string(direntp->d_name);
			files.push_back(filename);
		}
	} while (direntp != NULL);
	closedir(dir);

	// now read all files
	for (auto i = files.begin(); i != files.end(); i++) {
		try {
			setup(name, *i);
		} catch (const std::exception& x) {
			debug(LOG_ERR, DEBUG_LOG, 0, "error in file %s: %s",
				i->c_str(), x.what());
		}
	}
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
