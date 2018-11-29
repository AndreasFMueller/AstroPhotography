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
 * \brief Flat image construction function for arbitrary image sequences
 */
template<typename T>
ImagePtr	flat(const ImageSequence& images, const Image<T>& bias) {
	// we first compute the pixelwise mean, but we have to eliminate
	// possible cosmic ray artefacts, so we let the thing compute
	// the variance nevertheless
	debug(LOG_DEBUG, DEBUG_LOG, 0, "compute mean of images");
	ImageMean<T>	im(images, bias, true);

	// extract the image
	ImagePtr	result = im.getImagePtr();
	Image<T>	*image = dynamic_cast<Image<T> *>(&*result);

	// find the maximum value of the image
	Max<T, double>	maxfilter;
	T	maxvalue = maxfilter(*image);
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

ImagePtr	flat(const ImageSequence& images) {
	// we first compute the pixelwise mean, but we have to eliminate
	// possible cosmic ray artefacts, so we let the thing compute
	// the variance nevertheless
	debug(LOG_DEBUG, DEBUG_LOG, 0, "compute mean of images");
	ImageMean<float>	im(images, true);

	// extract the image
	ImagePtr	result = im.getImagePtr();
	Image<float>	*image = dynamic_cast<Image<float> *>(&*result);

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
			const ImagePtr biasimage) const {
	ImagePtr	result;

	// make sure we have images
	if (images.size() == 0) {
		throw std::runtime_error("no images supplied for flat");
	}

	// check the type of bias image we have
	if (!biasimage) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "not using a bias image");
		result =  flat(images);
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
			result = flat(images, *doublebias);
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
			result = flat(images, *floatbias);
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
