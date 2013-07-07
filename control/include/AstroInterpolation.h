/*
 * AstroInterpolation.h -- interpolate missing pixels in calibrated images
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroInterpolation_h
#define _AstroInterpolation_h

#include <AstroImage.h>

namespace astro {
namespace interpolation {

/**
 * \brief Using a dark image, interpolate bad pixelsin an image
 */
class Interpolator {
	const astro::image::ImagePtr& dark;
	astro::image::Image<float>	*floatdark;
	astro::image::Image<double>	*doubledark;
public:
	Interpolator(const astro::image::ImagePtr& dark);
	void	interpolate(astro::image::ImagePtr& image);
	astro::image::ImagePtr	operator()(const astro::image::ImagePtr& image);
};

} // namespace interpolation
} // namespace astro

#endif /* _AstroInterpolation_h */
