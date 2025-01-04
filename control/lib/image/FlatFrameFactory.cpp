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
ImagePtr	flat(const ImageSequence& images, const Image<T>& bias,
			bool mosaic) {
	// we first compute the pixelwise mean, but we have to eliminate
	// possible cosmic ray artefacts, so we let the thing compute
	// the variance nevertheless
	debug(LOG_DEBUG, DEBUG_LOG, 0, "compute mean of images");
	ImageMean<T>	im(images, bias, true);

	// extract the image
	ImagePtr	result = im.getImagePtr();
	Image<T>	*image = dynamic_cast<Image<T> *>(&*result);

	// maximum values to be used for normalization
	T	maxvalue[4];

	// find the maximum value of the image
	Max<T, double>	maxfilter;
	if (mosaic) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"setting up mosaiced max values");
		{
			Subgrid	s(ImagePoint(0, 0), ImageSize(2, 2));
			ConstSubgridAdapter<T>	sa(*image, s);
			maxvalue[0] = maxfilter(sa);
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"maximum for gridpoint (0,0): %f", maxvalue[0]);
		}
		{
			Subgrid	s(ImagePoint(1, 0), ImageSize(2, 2));
			ConstSubgridAdapter<T>	sa(*image, s);
			maxvalue[1] = maxfilter(sa);
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"maximum for gridpoint (1,0): %f", maxvalue[1]);
		}
		{
			Subgrid	s(ImagePoint(0, 1), ImageSize(2, 2));
			ConstSubgridAdapter<T>	sa(*image, s);
			maxvalue[2] = maxfilter(sa);
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"maximum for gridpoint (0,1): %f", maxvalue[2]);
		}
		{
			Subgrid	s(ImagePoint(1, 1), ImageSize(2, 2));
			ConstSubgridAdapter<T>	sa(*image, s);
			maxvalue[3] = maxfilter(sa);
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"maximum for gridpoint (1,1): %f", maxvalue[3]);
		}
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "nonmosaiced normalization");
		maxvalue[0] = maxfilter(*image);
		maxvalue[1] = maxvalue[0];
		maxvalue[2] = maxvalue[0];
		maxvalue[3] = maxvalue[0];
		debug(LOG_DEBUG, DEBUG_LOG, 0, "maximum value: %f",
			maxvalue[0]);
	}

	// devide the image by that value, so that the new maximum value
	// is 1. note that each subgrid is normalized with its own
	// maximum value, but for non-mosaic-images, they are all the
	// same
	for (int x = 0; x < image->size().width(); x += 2) {
		for (int y = 0; y < image->size().height(); y += 2) {
			image->pixel(x,   y)   /= maxvalue[0];
			if (x+1 < image->size().width())
				image->pixel(x+1, y)   /= maxvalue[1];
			if (y+1 < image->size().height())
				image->pixel(x,   y+1) /= maxvalue[2];
			if ((x+1 < image->size().width())
				&& (y+1 < image->size().height()))
				image->pixel(x+1, y+1) /= maxvalue[3];
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
			const ImagePtr biasimage,
			const bool mosaic) const {
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
			result = flat(images, *doublebias, mosaic);
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
			result = flat(images, *floatbias, mosaic);
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
