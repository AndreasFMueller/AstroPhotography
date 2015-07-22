/*
 * Version.cpp -- version access
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <config.h>

static const char	*version = VERSION;

extern "C"
const char	*getAstroVersion() {
	return version;
}
