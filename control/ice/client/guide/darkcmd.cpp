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
		guider->startDarkAcquire(exposure.exposuretime, imagecount);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot start dark: %s", x.what());
		return EXIT_FAILURE;
	}

	// we are done
	return EXIT_SUCCESS;
}


} // namespace snowguide
} // namespace app
} // namespace snowstar
