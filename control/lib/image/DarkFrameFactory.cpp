/**
 * DarkFrameFactory.cpp -- compute calibration frames
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

/**
 * \brief Perform dark computation for a subgrid
 */
template<typename T>
size_t	subdark(const ImageSequence&, ImageMean<T>& im,
	const Subgrid grid, unsigned int badpixellimit = 3) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "processing subgrid %s",
		grid.toString().c_str());
	// we also need the mean of the image to decide which pixels are
	// too far off to consider them "sane" pixels
	T	mean = im.mean(grid);
	T	var = im.variance(grid);

	// now find out which pixels are bad, and mark them using NaNs.
	// we consider pixels bad if the deviate from the mean by more
	// than three standard deviations
	T	stddevk = badpixellimit * sqrt(var);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found mean: %f, variance: %f, "
		"stddev*%.1f = %f", mean, var, badpixellimit, stddevk);
	size_t	badpixelcount = 0;
	SubgridAdapter<T>	sga(*im.image, grid);
	ImageSize	size = sga.getSize();
	for (int x = 0; x < size.width(); x++) {
		for (int y = 0; y < size.height(); y++) {
			T	v = sga.pixel(x, y);
			// skip NaNs
			if (v != v) {
				break;
			}
			if (fabs(v - mean) > stddevk) {
				sga.writablepixel(x, y)
					= std::numeric_limits<T>::quiet_NaN();
				badpixelcount++;
			}
		}
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %u bad pixels", badpixelcount);
	return badpixelcount;
}

/**
 * \brief Function to compute a dark image from a sequence of images
 *
 * This function first computes pixelwise mean and variance of the
 * image sequence. Then mean and variance over the image are computed.
 * This allows 
 * \param images	sequence of images to use to compute the 
 *			dark image
 * \param badpixellimit	number of standard deviations to consider a pixel bad
 */
template<typename T>
ImagePtr	dark_plain(const ImageSequence& images,
			double badpixellimit = 3) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "plain dark processing");
	ImageMean<T>	im(images, true);
	size_t	badpixels = subdark<T>(images, im, Subgrid(), badpixellimit);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "total bad pixels: %d", badpixels);
	
	// that's it, we now have a dark image
	ImagePtr	darkimg = im.getImagePtr();
	darkimg->setMetadata(FITSKeywords::meta("BADPIXEL", (long)badpixels));
	darkimg->setMetadata(FITSKeywords::meta("BDPXLLIM",
		(double)badpixellimit));
	return darkimg;
}

template<typename T>
ImagePtr	dark(const ImageSequence& images, double badpixellimit = 3,
			bool gridded = false) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "gridded: %s", (gridded) ? "YES" : "NO");
	if (!gridded) {
		return dark_plain<T>(images, badpixellimit);
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "gridded dark processing");
	ImageMean<T>	im(images, true);
	// perform the dark computation for each individual subgrid
	size_t	badpixels = 0;
	ImageSize	step(2, 2);
	badpixels += subdark<T>(images, im, Subgrid(ImagePoint(0, 0), step),
			badpixellimit);
	badpixels += subdark<T>(images, im, Subgrid(ImagePoint(1, 0), step),
			badpixellimit);
	badpixels += subdark<T>(images, im, Subgrid(ImagePoint(0, 1), step),
			badpixellimit);
	badpixels += subdark<T>(images, im, Subgrid(ImagePoint(1, 1), step),
			badpixellimit);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "total bad pixels: %d", badpixels);
	ImagePtr	darkimg = im.getImagePtr();
	darkimg->setMetadata(FITSKeywords::meta("BADPIXEL", (long)badpixels));
	darkimg->setMetadata(FITSKeywords::meta("BDPXLLIM",
		(double)badpixellimit));
	return darkimg;
}

/**
 * \brief Dark image construction function for arbitrary image sequences
 */
ImagePtr DarkFrameFactory::operator()(const ImageSequence& images) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "processing %d images into dark frame",
		images.size());
	// make sure we have at least one image
	if (images.size() == 0) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cannot create dark from no images");
		throw std::runtime_error("no images in sequence");
	}

	// find out whether these are Bayer images, by looking at the first
	// image
	ImagePtr	firstimage = *images.begin();
	bool	gridded = firstimage->getMosaicType().isMosaic();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "first image is %sgridded",
		(gridded) ? "" : "not ");
	
	// based on the bit size of the first image, decide whether to work
	// with floats or with doubles
	unsigned int	floatlimit = std::numeric_limits<float>::digits;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "float limit is %u", floatlimit);
	ImagePtr	result;
	if (firstimage->bitsPerPlane() <= floatlimit) {
		result = dark<float>(images, _badpixellimit, gridded);
	} else {
		result = dark<double>(images, _badpixellimit, gridded);
	}

	// copy the metadata
	copyMetadata(result, firstimage);
	return result;
}

void	DarkFrameFactory::copyMetadata(ImagePtr dark,
		ImagePtr firstimage) const {
	CalibrationFrameFactory::copyMetadata(dark, firstimage);

	// make sure this image is a dark image
	dark->setMetadata(FITSKeywords::meta(std::string("PURPOSE"),
		std::string("dark")));
}


} // calibration
} // astro
