/**
 * FlatFrameFactory.cpp -- compute calibration frames
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

// normalization gridded and ungridded should be separate from bias frame
template<typename T>
static void	normalize(ImageAdapter<T>& image) {
	Max<T, double>	maxfilter;
	T	maxvalue = maxfilter(image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "normalize max value %f to 1", maxvalue);
	ImageSize	size = image.getSize();
	for (int x = 0; x < size.width(); x++) {
		for (int y = 0; y < size.height(); y++) {
			T	v = image.pixel(x, y);
			image.writablepixel(x, y) = v / maxvalue;
		}
	}
}

template<typename T>
static void	mosaic_normalize(ImageAdapter<T>& image) {
	Max<T, double>	maxfilter;
	for (int x = 0; x <= 1; x++) {
		for (int y = 0; y <= 1; y++) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"normalize (%d,%d) subgrid", x, y);
			Subgrid	s(ImagePoint(x, y), ImageSize(2, 2));
			SubgridAdapter<T>	sa(image, s);
			normalize(sa);
		}
	}
}

// interpolation 
template<typename T>
static T	interpolate(ImageAdapter<T>& image, int x, int y) {
	T	sum = 0;
	int	counter = 0;
	for (int xi = -1; xi <= 1; xi++) {
		for (int yi = -1; yi <= 1; yi++) {
			if ((xi == 0) && (yi == 0))
				continue;
			int	X = x + xi;
			int	Y = y + yi;
			T	v = image.pixel(X, Y);
			if (v == v) {
				sum += v;
				counter++;
			}
		}
	}
	if (counter > 0) {
		sum = (1. / counter) * sum;
	}
	return sum;
}

template<typename T>
static void	interpolate(ImageAdapter<T>& image, bool mosaic) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "interpolate in %smosaic image",
		(mosaic) ? "" : "non ");
	if (mosaic) {
		for (int x = 0; x <= 1; x++) {
			for (int y = 0; y <= 1; y++) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"interpolate on (%d,%d) subgrid", x, y);
				Subgrid	s(ImagePoint(x, y), ImageSize(2, 2));
				SubgridAdapter<T>	sa(image, s);
				interpolate(sa, false);
			}
		}
	} else {
		ImageSize	size = image.getSize();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "interpolate %s image",
			size.toString().c_str());
		for (int x = 0; x < size.width(); x++) {
			for (int y = 0; y < size.width(); y++) {
				T	v = interpolate(image, x, y);
				image.writablepixel(x, y) = v;
			}
		}
	}
}

/**
 * \brief Flat image construction function for arbitrary image sequences
 */
template<typename T>
static ImagePtr	flat(const ImageSequence& images, const Image<T>& bias,
			bool mosaic, bool _interpolate) {
	// we first compute the pixelwise mean, but we have to eliminate
	// possible cosmic ray artefacts, so we let the thing compute
	// the variance nevertheless
	debug(LOG_DEBUG, DEBUG_LOG, 0, "compute mean of images");
	ImageMean<T>	im(images, bias, true);

	// extract the image
	ImagePtr	result = im.getImagePtr();
	Image<T>	*image = dynamic_cast<Image<T> *>(&*result);

	// remember bad pixels in the bias frame
	debug(LOG_DEBUG, DEBUG_LOG, 0, "copy bad pixels from bias to flat");
	ImageSize	size = image->size();
	int	bad_bias_pixels = 0;
	for (int x = 0; x < size.width(); x++) {
		for (int y = 0; y < size.height(); y++) {
			T	b = bias.pixel(x, y);
			if (b == b)
				continue;
			image->writablepixel(x, y) = b;
			bad_bias_pixels++;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d bad bias pixels",
		bad_bias_pixels);

	// interpolate bad pixels, if asked to do so
	if (_interpolate) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "bad pixel interpolation");
		interpolate(*image, mosaic);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "flat image interpolated");
	}

	// normalize 
	debug(LOG_DEBUG, DEBUG_LOG, 0, "flat image normalization");
	if (mosaic) {
		mosaic_normalize(*image);
	} else {
		normalize(*image);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image normalized");

	return result;
}

static ImagePtr	flat(const ImageSequence& images, bool interpolate) {
	// we first compute the pixelwise mean, but we have to eliminate
	// possible cosmic ray artefacts, so we let the thing compute
	// the variance nevertheless
	debug(LOG_DEBUG, DEBUG_LOG, 0, "compute mean of images");
	ImageMean<float>	im(images, true);

	// extract the image
	ImagePtr	result = im.getImagePtr();
	Image<float>	*image = dynamic_cast<Image<float> *>(&*result);

	// interpolate any bad pixels
	if (interpolate) {
		debug(LOG_ERR, DEBUG_LOG, 0, "interpolation missing");
	}

	// find the maximum value of the image
	Max<float, double>	maxfilter;
	float	maxvalue = maxfilter(*image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "maximum value: %f", maxvalue);

	// devide the image by that value, so that the new maximum value
	// is 1
	for (int x = 0; x < image->size().width(); x++) {
		for (int y = 0; y < image->size().height(); y++) {
			image->pixel(x, y) /= maxvalue;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image normalized");
	return result;
}

void	FlatFrameFactory::copyMetadata(ImagePtr flat,
	ImagePtr firstimage) const {
	CalibrationFrameFactory::copyMetadata(flat, firstimage);
	flat->setMetadata(FITSKeywords::meta(std::string("PURPOSE"),
                std::string("flat")));
}

/**
 * \brief Flat image construction operator
 *
 * \param images	the images to use to build the flat image
 * \param biasimage	the bias image to use to calibrate the images
 */
ImagePtr	FlatFrameFactory::operator()(const ImageSequence& images,
			const ImagePtr biasimage,
			const bool mosaic,
			const bool interpolate) const {
	ImagePtr	result;

	// make sure we have images
	if (images.size() == 0) {
		throw std::runtime_error("no images supplied for flat");
	}

	// check the type of bias image we have
	if (!biasimage) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "not using a bias image");
		result =  flat(images, interpolate);
		goto metadata;
	}

	// handle double bias image
	{
		Image<double>	*doublebias
				= dynamic_cast<Image<double>*>(&*biasimage);
		if (doublebias) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "bias is Image<double>");
			CountNaNs<double, double>	countnans;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "bias has %f nans",
				countnans(*doublebias));
			result = flat(images, *doublebias, mosaic, interpolate);
			goto metadata;
		}
	}
	{
		Image<float>	*floatbias
				= dynamic_cast<Image<float>*>(&*biasimage);
		if (floatbias) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "bias is Image<float>");
			CountNaNs<float, double>	countnans;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "bias has %f nans",
				countnans(*floatbias));
			result = flat(images, *floatbias, mosaic, interpolate);
			goto metadata;
		}
	}
	throw std::runtime_error("no useful flat image supplied");
metadata:
	copyMetadata(result, *images.begin());
	return result;
}

} // calibration
} // astro
