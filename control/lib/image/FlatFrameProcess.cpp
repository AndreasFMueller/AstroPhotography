/**
 * FlatFrameProcess.cpp -- compute calibration frames
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroCalibration.h>
#include <AstroFilter.h>
#include <PixelValue.h>
#include <limits>
#include <AstroDebug.h>
#include <stdexcept>
#include <vector>
#include <AstroFormat.h>
#include <AstroIO.h>
#include "ImageMean.h"

using namespace astro::image;
using namespace astro::image::filter;
using namespace astro::camera;
using namespace astro::adapter;
using namespace astro::io;

namespace astro {
namespace calibration {

ImagePtr	FlatFrameProcess::get() {
	prepare();

	// start exposure
	exposure.shutter(Shutter::OPEN);
	ccd->startExposure(exposure);

	// get a sequence of images
	ImageSequence	images = ccd->getImageSequence(_nimages);

	// convert the images into a flat frame
	FlatFrameFactory	ff;
	ImagePtr	flat = ff(images, dark);

	// turn of cooler
	cleanup();

	// return the dark image
	return flat;
}

} // calibration
} // astro
