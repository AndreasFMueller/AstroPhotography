/*
 * darkcmd.cpp -- implementation of the dark command
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "guide.h"
#include <cstdlib>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <iostream>
#include <IceConversions.h>
#include "display.h"

namespace snowstar {
namespace app {
namespace snowguide {

/**
 * \brief Implementation of the dark command
 */
int	Guide::dark_command(GuiderPrx guider) {
	try {
		guider->startDarkAcquire(exposure.exposuretime, imagecount,
			badpixellimit);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot start dark: %s", x.what());
		return EXIT_FAILURE;
	}

	// we are done
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the flat command
 */
int	Guide::flat_command(GuiderPrx guider) {
	try {
		guider->startFlatAcquire(exposure.exposuretime, imagecount,
			usedark);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot start flat: %s", x.what());
		return EXIT_FAILURE;
	}

	// we are done
	return EXIT_SUCCESS;
}

} // namespace snowguide
} // namespace app
} // namespace snowstar
