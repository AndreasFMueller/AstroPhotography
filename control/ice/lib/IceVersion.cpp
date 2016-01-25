/*
 * IceVersion.cpp -- implementation of the ICE astro version
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <config.h>

static const char       *version = VERSION;

extern "C"
const char      *getIceAstroVersion() {
        return version;
}

