/*
 * ViewerPipeline.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <ViewerPipeline.h>

namespace astro {
namespace image {

ViewerPipeline::ViewerPipeline(const Image<RGB<float> > *_imagep)
                : ConstImageAdapter<unsigned int>(_imagep->size()),
		  //imagep(_imagep),
		  backgroundsubtract(*_imagep),
                  colorcorrectionadapter(backgroundsubtract),
                  luminanceimage(colorcorrectionadapter),
                  colorimage(colorcorrectionadapter),
                  rangeadapter(luminanceimage),
                  gammaadapter(rangeadapter, 1),
                  upscale(gammaadapter, 256),
                  compose(upscale, colorimage),
                  rgb32(compose) {
}

float  ViewerPipeline::gamma() const {
	return gammaadapter.gamma();
}

void    ViewerPipeline::gamma(float gamma) {
	gammaadapter.gamma(gamma);
}

float	ViewerPipeline::saturation() const {
	return	colorimage.saturation();
}

void	ViewerPipeline::saturation(float saturation) {
	colorimage.saturation(saturation);
}

void	ViewerPipeline::backgroundEnabled(bool backgroundenabled) {
	backgroundsubtract.scalefactor((backgroundenabled) ? 1. : 0.);
}

bool	ViewerPipeline::backgroundEnabled() const {
	return (backgroundsubtract.scalefactor() == 1.) ? true : false;
}

void	ViewerPipeline::gradientEnabled(bool gradientenabled) {
	backgroundsubtract.gradient(gradientenabled);
}

bool	ViewerPipeline::gradientEnabled() const {
	return backgroundsubtract.gradient();
}

void	ViewerPipeline::setRange(double min, double max) {
	rangeadapter.setRange(min, max);
}

float	ViewerPipeline::min() const {
	return rangeadapter.min();
}

float	ViewerPipeline::max() const {
	return rangeadapter.max();
}

RGB<float>	ViewerPipeline::colorcorrection() const {
	return colorcorrectionadapter.rgb();
}

void	ViewerPipeline::colorcorrection(const RGB<float>& _colorcorrection) {
	colorcorrectionadapter.rgb(_colorcorrection);
}

const Background<float>&	ViewerPipeline::background() const {
	return backgroundsubtract.background();
}

void	ViewerPipeline::background(const Background<float>& background) {
	backgroundsubtract.background(background);
}

const	ConstImageAdapter<RGB<float> >&	ViewerPipeline::processedimage() const {
	return compose;
}

} // namespace image
} // namespace astro
