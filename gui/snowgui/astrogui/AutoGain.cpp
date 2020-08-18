/*
 * AutoGain.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AutoGain.h>
#include <AstroAdapter.h>
#include <AstroFilter.h>
#include <AstroFilterfunc.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <stdexcept>

using namespace astro::image;

namespace snowgui {

void	AutoGain::setup(double minimum, double maximum) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "minimum = %f, maximum = %f",
		minimum, maximum);
	double	delta = maximum - minimum;
	if (delta <= 0) {
		delta = 1;
	}
	_gain = 255 / delta;
	_brightness = - minimum * _gain;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "min=%f, max=%f, gain=%f, brightness=%f",
		minimum, maximum, _gain, _brightness);
}

AutoGain::AutoGain(const ImagePtr image) {
	// find the minimum and maximum
	double	minimum = 0;
	double	maximum = 1;
	minimum = astro::image::filter::min_luminance(image);
	maximum = astro::image::filter::max_luminance(image);
	setup(minimum, maximum);
}

void	AutoGain::setup(const ConstImageAdapter<double>& doubleadapter,
		const ImageRectangle& rectangle) {
	astro::adapter::WindowAdapter<double>	window(doubleadapter,
							rectangle);
	astro::image::filter::Min<double, double>	minimum;
	astro::image::filter::Max<double, double>	maximum;
	setup(minimum(window), maximum(window));
}

#define	getia(la, image, Pixel)						\
	if (ia == NULL) {						\
		Image<Pixel>	*imgptr					\
			= dynamic_cast<Image<Pixel> *>(&*image);	\
		if (NULL != imgptr) {					\
			la = new astro::adapter::LuminanceAdapter<Pixel, double>(*imgptr);\
		}							\
	}

AutoGain::AutoGain(const ImagePtr image, const ImageRectangle& rectangle) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "autogaining in %s",
		rectangle.toString().c_str());
	// make sure the rectangle is actually contained in the image
	if (!image->size().bounds(rectangle)) {
		std::string	msg = astro::stringprintf(
			"rectangle %s not contained in %s",
			rectangle.toString().c_str(),
			image->size().toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	if (1 == image->planes()) {
		astro::adapter::DoubleAdapter	doubleadapter(image);
		setup(doubleadapter, rectangle);
	} else {
		ConstImageAdapter<double>	*ia = NULL;
		getia(ia, image, RGB<unsigned char>)
		getia(ia, image, RGB<unsigned short>)
		getia(ia, image, RGB<unsigned int>)
		getia(ia, image, RGB<float>)
		getia(ia, image, RGB<double>)
		if (NULL == ia) {
			std::string	msg = astro::stringprintf(
				"cannot convert %s to luminance",
				astro::demangle(image->pixel_type().name()).c_str());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
		setup(*ia, rectangle);
		delete ia;
	}
}

} // namespace snowgui
