/*
 * ImageCalibration.cpp -- processing class to perform image calibration
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProcess.h>

using namespace astro::adapter;

namespace astro {
namespace process {

ImageCalibration::ImageCalibration(const ConstImageAdapter<double>& image,
	ImagePtr dark, ImagePtr flat) : _dark(dark), _flat(flat),
					_image(image) {
}



} // namespace process
} // namespace astro
