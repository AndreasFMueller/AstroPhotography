/*
 * ModuleDescriptor.cpp -- descriptor implementation
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroLoader.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

namespace astro {
namespace module {

static const std::string	module_name("");
static const std::string	module_version(VERSION);

/**
 * \brief Get the name of module
 *
 * The module name will usually be identical to the name used to load
 * the module.  But it is possible that a given module contains
 * code for several functions, like cameras and filterwheels (which are
 * often controlled through a camera).
 */
std::string	ModuleDescriptor::name() const {
	return module_name;
}

/**
 * \brief Get the version of the module
 *
 * The version string should also include version strings for
 * libraries the module depends on.
 */
std::string	ModuleDescriptor::version() const {
	return module_version;
}

/**
 * \brief  Find out whether the module has a DeviceLocator
 */
bool	ModuleDescriptor::hasDeviceLocator() const {
	return false;
}

} // namespace module
} // namespace astro
