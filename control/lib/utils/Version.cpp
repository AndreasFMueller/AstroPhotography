/*
 * version.cpp -- utility function to access the version string
 *
 * (c) 2019 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <string>
#include <AstroFormat.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

namespace astro {

std::string	version() {
	return stringprintf("%s - %s %s", VERSION, __DATE__, __TIME__);
}

}
