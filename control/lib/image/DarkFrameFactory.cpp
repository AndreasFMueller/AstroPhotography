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
 *
 * Bad pixel detection will be performde if the detect_bad_pixels
 * argument is set to true. A pixel is considered bad if its value
 * differs by more than badpixellimitstddevs standard deviations
 * from mean of the image.
 *
 * TODO: This criterion is probably not good. It should not rely
 *       on the mean of all the other pixels of the image, but only
 *       on the values of the image sequence
 *
 * \param im	the ImageMean object that also contains the dark image
 * \param grid	the part of the grid that is to be worked on
 * \param badpixellimitstddevs	the numer of standard deviations offset
 *				to consider a pixel bad
 * \param detect_bad_pixels	Whether or not to perform bad pixel detection
 */
template<typename T>
static size_t	subdark(const ImageSequence&, ImageMean<T>& im,
			const Subgrid grid,
			unsigned int badpixellimitstddevs,
			bool detect_bad_pixels,
			bool interpolate) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "processing subgrid %s",
		grid.toString().c_str());
	// we also need the mean of the image to decide which pixels are
	// too far off to consider them "sane" pixels
	T	mean = im.mean(grid);
	T	var = im.variance(grid);

	// the subgrid to work on
	SubgridAdapter<T>	sga(*im.image, grid);
	ImageSize	size = sga.getSize();

	// handle the case where not bad pixel detection is necessary
	if (!detect_bad_pixels) {
		for (int x = 0; x < size.width(); x++) {
			for (int y = 0; y < size.height(); y++) {
				T	v = sga.pixel(x, y);
				sga.writablepixel(x, y) = v;
			}
		}
		return 0;
	}

	// now find out which pixels are bad, and mark them using NaNs.
	// we consider pixels bad if the deviate from the mean by more
	// than three standard deviations
	T	stddevk = badpixellimitstddevs * sqrt(var);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found mean: %f, variance: %f, "
		"stddev*%.1f = %f", mean, var, badpixellimitstddevs, stddevk);
	size_t	badpixelcount = 0;
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

	if (interpolate) {
		for (int x = 0; x < size.width(); x++) {
			for (int y = 0; y < size.height(); y++) {
				T	v = sga.pixel(x, y);
				if (v == v)
					continue;
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"interpolating pixel at position "
					"(%d,%d)", x, y);
				double	sum = 0;
				int	counter = 0;
				for (int xi = -1; xi < 1; xi++) {
					for (int yi = -1; yi < 1; yi++) {
						if ((xi == 0) && (yi == 0))
							continue;
						int	X = x + xi;
						int	Y = y + yi;
						if (size.contains(X, Y)) {
							T	v2 = sga.pixel(X, Y);
							if (v2 == v2) {
								sum += v2;
								counter++;
							}
						}
					}
				}
				debug(LOG_DEBUG, DEBUG_LOG, 0, "interpolation: %f/%d", sum, counter);
				if (counter > 0) {
					v = (1. / counter) * sum;
					sga.writablepixel(x, y) = v;
				}
			}
		}
	}

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
 * \param badpixellimitstddevs	number of standard deviations to consider a pixel bad
 */
template<typename T>
static ImagePtr	dark_plain(const ImageSequence& images,
			double badpixellimitstddevs,
			bool detect_bad_pixels,
			bool interpolate) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "plain dark processing");
	ImageMean<T>	im(images, true);
	size_t	badpixels = subdark<T>(images, im, Subgrid(),
		badpixellimitstddevs, detect_bad_pixels, interpolate);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "total bad pixels: %d", badpixels);
	
	// that's it, we now have a dark image
	ImagePtr	darkimg = im.getImagePtr();
	darkimg->setMetadata(FITSKeywords::meta("BADPIXEL", (long)badpixels));
	darkimg->setMetadata(FITSKeywords::meta("BDPXLLIM",
		(double)badpixellimitstddevs));
	return darkimg;
}

template<typename T>
static ImagePtr	dark(const ImageSequence& images,
			double badpixellimitstddevs,
			bool gridded,
			bool detect_bad_pixels,
			bool interpolate) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "gridded: %s", (gridded) ? "YES" : "NO");
	if (!gridded) {
		return dark_plain<T>(images, badpixellimitstddevs,
			detect_bad_pixels, interpolate);
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "gridded dark processing");
	ImageMean<T>	im(images, true);
	// perform the dark computation for each individual subgrid
	size_t	badpixels = 0;
	ImageSize	step(2, 2);
	badpixels += subdark<T>(images, im, Subgrid(ImagePoint(0, 0), step),
			badpixellimitstddevs, detect_bad_pixels, interpolate);
	badpixels += subdark<T>(images, im, Subgrid(ImagePoint(1, 0), step),
			badpixellimitstddevs, detect_bad_pixels, interpolate);
	badpixels += subdark<T>(images, im, Subgrid(ImagePoint(0, 1), step),
			badpixellimitstddevs, detect_bad_pixels, interpolate);
	badpixels += subdark<T>(images, im, Subgrid(ImagePoint(1, 1), step),
			badpixellimitstddevs, detect_bad_pixels, interpolate);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "total bad pixels: %d", badpixels);
	ImagePtr	darkimg = im.getImagePtr();
	darkimg->setMetadata(FITSKeywords::meta("BADPIXEL", (long)badpixels));
	darkimg->setMetadata(FITSKeywords::meta("BDPXLLIM",
		(double)badpixellimitstddevs));
	return darkimg;
}

/**
 * \brief Dark image construction function for arbitrary image sequences
 */
ImagePtr DarkFrameFactory::operator()(const ImageSequence& images,
		bool detect_bad_pixels, bool interpolate) const {
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
		result = dark<float>(images, _badpixellimitstddevs, gridded,
			detect_bad_pixels, interpolate);
	} else {
		result = dark<double>(images, _badpixellimitstddevs, gridded,
			detect_bad_pixels, interpolate);
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
