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

/**
 * \brief Constructor for the FlatFrameFactory
 *
 * \param mosaic	whether to perform the construction on a grid
 * \param interpolate	whether to interpolate pixels indicated as bad by
 *			the bias image
 */
FlatFrameFactory::FlatFrameFactory(bool mosaic, bool interpolate)
	: _mosaic(mosaic), _interpolate(interpolate) {
}

/**
 * \brief Normalize an image
 *
 * This method computes the maximum of an image and then devides all
 * pixels by this value. The image can be a SubgridAdapter, which measn
 * that only the pixels of the subgrid are normalized. This is used
 * by the mosaic_normalize template to perform subgridded normalization.
 *
 * \param image 	the image to normalize
 */
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image normalized");
}

/**
 * \brief Normalize a mosaic image
 *
 * \param image		normalize in a mosaiced way
 */
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

/**
 * \brief Flat image construction function for arbitrary image sequences
 *
 * Construct a flat image from a sequence of images
 *
 * \param images	the sequence of images to construct the flat from
 * \param bias		the bias image to subtract first
 */
template<typename FlatPixelType>
ImagePtr	FlatFrameFactory::flat(const ImageSequence& images,
			const Image<FlatPixelType>& bias) const {
	// first we report how many NaNs there are in the bias image
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "bias is type %s", bias.info().c_str());
	CountNaNs<FlatPixelType, size_t>	countnans;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "bias has %d nans",
		countnans.filter(bias));

	// we first compute the pixelwise mean, but we have to eliminate
	// possible cosmic ray artefacts, so we let the thing compute
	// the variance nevertheless
	debug(LOG_DEBUG, DEBUG_LOG, 0, "compute mean of images");
	ImageMean<FlatPixelType>	im(images, bias, true);

	// extract the image
	ImagePtr	result = im.getImagePtr();
	Image<FlatPixelType>	*image
		= dynamic_cast<Image<FlatPixelType> *>(&*result);

	// remember bad pixels in the bias frame
	debug(LOG_DEBUG, DEBUG_LOG, 0, "copy bad pixels from bias to flat");
	ImageSize	size = image->size();
	int	bad_bias_pixels = 0;
	for (int x = 0; x < size.width(); x++) {
		for (int y = 0; y < size.height(); y++) {
			FlatPixelType	b = bias.pixel(x, y);
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
		CalibrationInterpolation	ci(_mosaic);
		ci.interpolate(*(ImageAdapter<FlatPixelType>*)image,
			*(ConstImageAdapter<FlatPixelType>*)&bias);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "flat image interpolated");
	}

	// normalize 
	debug(LOG_DEBUG, DEBUG_LOG, 0, "flat image normalization");
	if (_mosaic) {
		mosaic_normalize(*image);
	} else {
		normalize(*image);
	}

	return result;
}

template
ImagePtr	FlatFrameFactory::flat(
			const astro::image::ImageSequence& images,
			const Image<float>& bias) const;
template
ImagePtr	FlatFrameFactory::flat(
			const astro::image::ImageSequence& images,
			const Image<double>& bias) const;

/**
 * \brief Construct a flat image 
 *
 * This is a very basic version that does not understand bias frames
 * and interpolation
 *
 * \param images	the images used to construct the flat
 */
ImagePtr	FlatFrameFactory::flat(const ImageSequence& images) const {
	// we first compute the pixelwise mean, but we have to eliminate
	// possible cosmic ray artefacts, so we let the thing compute
	// the variance nevertheless
	debug(LOG_DEBUG, DEBUG_LOG, 0, "compute mean of images");
	ImageMean<float>	im(images, true);

	// extract the image, which consists of mean values for each pixel
	ImagePtr	result = im.getImagePtr();
	Image<float>	*image = dynamic_cast<Image<float> *>(&*result);

	// normaliize the flat image just computed
	normalize(*image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image normalized");

	return result;
}

/**
 * \brief Flat image construction operator
 *
 * \param images	the images to use to build the flat image
 * \param biasimage	the bias image to use to calibrate the images
 */
ImagePtr	FlatFrameFactory::operator()(const ImageSequence& images,
			const ImagePtr biasimage) const {
	ImagePtr	result;

	// Make sure we have images
	if (images.size() == 0) {
		throw std::runtime_error("no images supplied for flat");
	}

	// Check whether we have a bias image. If we don't then we
	// cannot do interpolation either and we can simply call
	// the simple flat creation function without a bias frame.
	if (!biasimage) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "not using a bias image");
		result =  flat(images);
		goto metadata;
	}

	// If we do have a bias frame, then we first have to find out
	// what the pixel type of the bias frame is. We then use the
	// flat function with a bias argument to create a flat image.
	// Note, however, that this only works for floating point pixel
	// types.
	{
		// handle bias image with double type pixels
		Image<double>	*doublebias
				= dynamic_cast<Image<double>*>(&*biasimage);
		if (doublebias) {
			result = flat(images, *doublebias);
			goto metadata;
		}
	}
	{
		// handle bias image with float type pixels
		Image<float>	*floatbias
				= dynamic_cast<Image<float>*>(&*biasimage);
		if (floatbias) {
			result = flat<float>(images, *floatbias);
			goto metadata;
		}
	}

	// we cannot handle other types of bias images
	throw std::runtime_error("no useful bias image supplied");
metadata:
	// copy the meta data information from the first image of the
	// image sequence 
	copyMetadata(result, images, "flat");
	return result;
}

} // calibration
} // astro
