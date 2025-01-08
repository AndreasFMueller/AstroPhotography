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
 * \brief Constructor for the DarkFrameFactory
 *
 * \param badpixellimitstddevs	the numer of standard deviations offset
 *				to consider a pixel bad
 * \param detect_bad_pixels	Whether or not to perform bad pixel detection
 */
DarkFrameFactory::DarkFrameFactory()
	: _badpixellimitstddevs(3), _absolute(0), _interpolate(false),
	  _detect_bad_pixels(false) {
}

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
 * \return	the number of bad pixels in the dark image created
 */
template<typename DarkPixelType>
size_t	DarkFrameFactory::subdark(ImageMean<DarkPixelType>& im,
			const Subgrid grid) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "processing subgrid %s",
		grid.toString().c_str());
	// we also need the mean of the image to decide which pixels are
	// too far off to consider them "sane" pixels
	DarkPixelType	mean = im.mean(grid);
	DarkPixelType	var = im.variance(grid);

	// the subgrid to work on
	SubgridAdapter<DarkPixelType>	sga(*im.image, grid);
	ImageSize	size = sga.getSize();

	// handle the case where bad pixel detection is not requested, i.e.
	// the dark image consists of just the averages
	if (!detect_bad_pixels()) {
		for (int x = 0; x < size.width(); x++) {
			for (int y = 0; y < size.height(); y++) {
				DarkPixelType	v = sga.pixel(x, y);
				sga.writablepixel(x, y) = v;
			}
		}
		return 0;
	}

	// now find out which pixels are bad, and mark them using NaNs.
	// we consider pixels bad if the deviate from the mean by more
	// than three standard deviations
	DarkPixelType	delta = 0;
	if (absolute()) {
		delta = absolute();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "use absolute offset %f",
			delta);
	} else {
		delta = badpixellimitstddevs() * sqrt(var);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found mean: %f, variance: %f, "
			"stddev * %.1f = %f", mean, var, badpixellimitstddevs(),
			delta);
	}

	// compute the bad pixels
	size_t	badpixelcount = 0;
	for (int x = 0; x < size.width(); x++) {
		for (int y = 0; y < size.height(); y++) {
			DarkPixelType	v = sga.pixel(x, y);
			// skip NaNs, they are already marked as bad pixels
			if (!(v == v))
				break;

			// find out whether we should mark pixel as bad
			if (fabs(v - im.image->pixel(x, y)) > delta) {
				sga.writablepixel(x, y)
					= std::numeric_limits<DarkPixelType>::quiet_NaN();
				badpixelcount++;
			}
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %u bad pixels", badpixelcount);

	// perform the inerpolation, if necessary
	if (interpolate()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"bad pixel interpolation requested");
		CalibrationInterpolation	ci;
		size_t	interpolated_pixels = ci.interpolate(sga, sga);
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"number of interpolated pixels: %d",
			interpolated_pixels);
		return 0;
	}

	// return the number of bad pixels
	return badpixelcount;
}

template
size_t	DarkFrameFactory::subdark(ImageMean<float>& im,
			const Subgrid grid) const;
template
size_t	DarkFrameFactory::subdark(ImageMean<double>& im,
			const Subgrid grid) const;

/**
 * \brief Function to compute a dark image from a sequence of images
 *
 * This function first computes pixelwise mean and variance of the
 * image sequence. Then mean and variance over the image are computed.
 * This allows 
 * \param images	sequence of images to use to compute the 
 *			dark image
 * \param badpixellimitstddevs	number of standard deviations to consider
 *				a pixel bad
 * \param absolute		the absolute pixel value difference to
 *				classify a pixel as bad
 * \param detect_bad_pixels	whether or not to detect bad pixels
 * \param _interpolate		whether or not to interpolate bad pixel values
 *				(does nothing if detect_bad_pixels is false)
 */
template<typename DarkPixelType>
ImagePtr	DarkFrameFactory::dark(const ImageSequence& images) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "plain dark processing");
	ImageMean<DarkPixelType>	im(images, true);
	size_t	badpixels = subdark<DarkPixelType>(im, Subgrid());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "total bad pixels: %d", badpixels);
	
	// that's it, we now have a dark image
	ImagePtr	darkimg = im.getImagePtr();

	// set some common metadata about bad pixels
	darkimg->setMetadata(FITSKeywords::meta("BADPIXEL", (long)badpixels));
	darkimg->setMetadata(FITSKeywords::meta("BDPXLLIM",
		(double)badpixellimitstddevs()));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "plain dark image creation completed");

	// return the dark image
	return darkimg;
}

template
ImagePtr	DarkFrameFactory::dark<float>(
			const ImageSequence& images) const;
template
ImagePtr	DarkFrameFactory::dark<double>(
			const ImageSequence& images) const;

/**
 * \brief Construct a dark image from a sequence images
 *
 * This method is capable of handling subgrids
 *
 * \param images		sequence of images from which to construct
 *				the dark image
 * \param badpixellimitstddevs	the limit for bad pixel detection
 * \param absolute		the absolute pixel value difference to
 *				classify a pixel as bad
 * \param gridded		whether or not to use 2x2 subgrids
 * \param detect_bad_pixels	whether or not to detect bad pixels
 * \param _interpolate		whether or not to interpolate bad pixel values
 *				(does nothing if detect_bad_pixels is false)
 */
template<typename DarkPixelType>
ImagePtr	DarkFrameFactory::dark(const ImageSequence& images,
			bool gridded) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "gridded: %s", (gridded) ? "YES" : "NO");
	if (!gridded) {
		return dark<DarkPixelType>(images);
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "gridded dark processing");
	ImageMean<DarkPixelType>	im(images, true);
	// perform the dark computation for each individual subgrid
	size_t	badpixels = 0;
	ImageSize	step(2, 2);
	badpixels += subdark<DarkPixelType>(im,
			Subgrid(ImagePoint(0, 0), step));
	badpixels += subdark<DarkPixelType>(im,
			Subgrid(ImagePoint(1, 0), step));
	badpixels += subdark<DarkPixelType>(im,
			Subgrid(ImagePoint(0, 1), step));
	badpixels += subdark<DarkPixelType>(im,
			Subgrid(ImagePoint(1, 1), step));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "total bad pixels: %d", badpixels);
	ImagePtr	darkimg = im.getImagePtr();
	darkimg->setMetadata(FITSKeywords::meta("BADPIXEL", (long)badpixels));
	darkimg->setMetadata(FITSKeywords::meta("BDPXLLIM",
		(double)badpixellimitstddevs()));
	return darkimg;
}

template
ImagePtr	DarkFrameFactory::dark<float>(const ImageSequence& images,
			bool gridded) const;
template
ImagePtr	DarkFrameFactory::dark<double>(const ImageSequence& images,
			bool gridded) const;

/**
 * \brief Dark image construction function for arbitrary image sequences
 *
 * This method figures out the right type of floating point pixel so that
 * it can hold the all the bits of the integer pixel types. It also detects
 * whether the camera has a bayer mosaic and therefore needs gridded
 * calibration image generation.
 *
 * \param images		sequence of images to construct the dark from
 * \param detect_bad_pixels	whether to detect bad pixels
 * \param _interpolate		whether to interpolate bad pixels
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
		result = dark<float>(images, gridded);
	} else {
		result = dark<double>(images, gridded);
	}

	// copy the metadata
	copyMetadata(result, images, "dark");
	return result;
}

} // calibration
} // astro
