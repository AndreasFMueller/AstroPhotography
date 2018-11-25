/**
 * Calibrator.cpp -- compute calibration frames
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

//////////////////////////////////////////////////////////////////////
// TypedCalibrator implementation (used for Calibrator)
//////////////////////////////////////////////////////////////////////
template<typename T>
class TypedCalibrator {
	const ConstImageAdapter<T>&	dark;
	const ConstImageAdapter<T>&	flat;
	T	nan;
public:
	TypedCalibrator(const ConstImageAdapter<T>& _dark,
		const ConstImageAdapter<T>& _flat);
	ImagePtr	operator()(const ImagePtr image) const;
};

template<typename T>
TypedCalibrator<T>::TypedCalibrator(const ConstImageAdapter<T>& _dark,
	const ConstImageAdapter<T>& _flat) 
	: dark(_dark), flat(_flat) {
	nan = std::numeric_limits<T>::quiet_NaN();
}

template<typename T>
ImagePtr	TypedCalibrator<T>::operator()(const ImagePtr image) const {
	ConstPixelValueAdapter<T>	im(image);
	Image<T>	*result = new Image<T>(image->size());
	for (int x = 0; x < image->size().width(); x++) {
		for (int y = 0; y < image->size().height(); y++) {
			T	darkvalue = dark.pixel(x, y);
			// if the pixel is bad give 
			if (darkvalue != darkvalue) {
				result->pixel(x, y) = nan;;
				continue;
			}
			T	v = im.pixel(x, y) - darkvalue;
			if (v < 0) {
				v = 0;
			}
			result->pixel(x, y) = v / flat.pixel(x, y);
		}
	}
	return ImagePtr(result);
}

//////////////////////////////////////////////////////////////////////
// Calibrator implementation
//////////////////////////////////////////////////////////////////////
Calibrator::Calibrator(const ImagePtr _dark, const ImagePtr _flat,
		const ImageRectangle _rectangle)
	: dark(_dark), flat(_flat), rectangle(_rectangle) {
	// We want dark and flat images to be of float or double type
	Image<float>	*fp = dynamic_cast<Image<float> *>(&*dark);
	Image<double>	*dp = dynamic_cast<Image<double> *>(&*dark);
	if ((fp == NULL) && (dp == NULL)) {
		std::string	msg("dark image must be of floating point type");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
}

ImagePtr	Calibrator::operator()(const ImagePtr image) const {
	unsigned int	floatlimit = std::numeric_limits<float>::digits;
	// find the appropriate frame to use for the correction images
	ImageRectangle	frame;
	if (rectangle == ImageRectangle()) {
		frame = ImageRectangle(ImagePoint(), image->size());
	}

	// use pixel size to decide which type to use for the result image
	if (image->bitsPerPixel() <= floatlimit) {
		// create adapters for darks and flats with float values
		ConstPixelValueAdapter<float>	pvdark(dark);
		WindowAdapter<float>		wdark(pvdark, frame);
		ConstPixelValueAdapter<float>	pvflat(flat);
		WindowAdapter<float>		wflat(pvflat, frame);
		TypedCalibrator<float>	calibrator(wdark, wflat);
		return calibrator(image);
	}
	ConstPixelValueAdapter<double>	pvdark(dark);
	WindowAdapter<double>		wdark(pvdark, frame);
	ConstPixelValueAdapter<double>	pvflat(flat);
	WindowAdapter<double>		wflat(pvflat, frame);
	TypedCalibrator<double>	calibrator(wdark, wflat);
	return calibrator(image);
}

} // calibration
} // astro
