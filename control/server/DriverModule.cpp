/*
 * DriverModule.cpp -- Driver module implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "DriverModule.h"

namespace Astro {

char	*DriverModule_impl::getName() {
	return (char *)_module->modulename().c_str();
}

} // namespace astro
