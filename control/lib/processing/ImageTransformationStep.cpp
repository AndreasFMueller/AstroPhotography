/*
 * ImageTransformationStep.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <AstroFormat.h>
#include <AstroAdapter.h>
#include <sstream>

using namespace astro::adapter;

namespace astro {
namespace process {

ImageTransformationStep::ImageTransformationStep(NodePaths& parent)
	: ImageStep(parent) {
	_vertical_flip = false;
	_horizontal_flip = false;
	_scale = 0;
	_xshift = 0.;
	_yshift = 0.;
}

#define transformadapter(inputimage, Pixel)				\
{									\
	Image<Pixel >	*img = dynamic_cast<Image<Pixel >*>(&*inputimage);	\
	if (NULL != img) {						\
		adapter::FlipAdapter<Pixel >	flap(*img, 		\
			_vertical_flip,	_horizontal_flip);		\
		astro::image::ConstImageAdapter<Pixel >	*adp = &flap;		\
		astro::image::transform::TranslationAdapter<Pixel >	oa(flap, Point(_xshift, _yshift));\
		if ((_xshift != 0) || (_yshift != 0.)) {		\
			debug(LOG_DEBUG, DEBUG_LOG, 0, "xshift=%.2f, yshift=%.2f", _xshift, _yshift); \
			adp = &oa;					\
		}							\
		if (_scale == 0) {					\
			_image = ImagePtr(new Image<Pixel>(*adp));	\
		}							\
		if (_scale > 0) {					\
			UpscaleAdapter<Pixel>	ua(*adp, _scale + 1);	\
			_image = ImagePtr(new Image<Pixel>(ua));	\
		}							\
		if (_scale < 0) {					\
			DownscaleAdapter<Pixel>	ua(*adp, 1 - _scale);	\
			_image = ImagePtr(new Image<Pixel>(ua));	\
		}							\
	}								\
}

ProcessingStep::state	ImageTransformationStep::do_work() {
	ImagePtr	prim = precursorimage();
	// build an adapter to perform the flips and the scaling
	transformadapter(prim, unsigned char);
	transformadapter(prim, unsigned short);
	transformadapter(prim, unsigned int);
	transformadapter(prim, unsigned long);
	transformadapter(prim, float);
	transformadapter(prim, double);
	transformadapter(prim, RGB<unsigned char>);
	transformadapter(prim, RGB<unsigned short>);
	transformadapter(prim, RGB<unsigned int>);
	transformadapter(prim, RGB<unsigned long>);
	transformadapter(prim, RGB<float>);
	transformadapter(prim, RGB<double>);

	return ProcessingStep::complete;
}

std::string	ImageTransformationStep::what() const {
	std::string	scaleinfo;
	if (_scale > 0) {
		scaleinfo = stringprintf("upscale 1->%d", _scale + 1);
	}
	if (_scale < 0) {
		scaleinfo = stringprintf("upscale %d->1", 1 - _scale);
	}
	return stringprintf("transform image hflip=%s vflip=%s scale=%s, xshift=%.1f, yshift=%1.f",
		(_vertical_flip) ? "yes" : "no",
		(_horizontal_flip) ? "yes" : "no",
		scaleinfo.c_str(),
		xshift(), yshift());
}

std::string	ImageTransformationStep::verboseinfo() const {
	std::ostringstream	out;
	out << ImageStep::verboseinfo();
	out << " vertical_flip=" << ((vertical_flip()) ? "yes" : "no");
	out << " horizontal_flip=" << ((horizontal_flip()) ? "yes" : "no");
	out << " scale=" << scale();
	return out.str();
}

} // namespace process
} // namespace astro
