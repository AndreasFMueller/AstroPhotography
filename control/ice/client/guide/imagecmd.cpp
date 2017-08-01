/*
 * imagecmd.cpp -- implementation of the image command
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "guide.h"
#include <cstdlib>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroIO.h>
#include <iostream>
#include <IceConversions.h>
#include "display.h"

namespace snowstar {
namespace app {
namespace snowguide {

/**
 * \brief Implementation of the dark command
 */
int	Guide::image_command(GuiderPrx guider, const std::string& filename) {
	try {
		guider->startImaging(exposure);
		do {
			usleep(100000);
		} while (GuiderIMAGING == guider->getState());
		ImagePrx	image = guider->getImage();
		astro::image::ImagePtr	imageptr = convert(image);
		astro::io::FITSout	out(filename);
		out.write(imageptr);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot : %s", x.what());
		return EXIT_FAILURE;
	}

	// we are done
	return EXIT_SUCCESS;
}

} // namespace snowguide
} // namespace app
} // namespace snowstar
