/*
 * FWHM2Evaluator.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "FWHM2Evaluator.h"
#include <AstroFocus.h>
#include <AstroFilterfunc.h>
#include <AstroFilter.h>
#include <AstroDebug.h>

using namespace astro::image::filter;
using namespace astro::adapter;

namespace astro {
namespace focusing {

FWHM2Evaluator::FWHM2Evaluator(const ImagePoint& center, double radius)
	: _center(center), _radius(radius) {
}

FWHM2Evaluator::FWHM2Evaluator() {
	_radius = 0;
}

FWHM2Evaluator::FWHM2Evaluator(const ImageRectangle& rectangle)
	: _center(rectangle.center()),
	  _radius(rectangle.size().smallerSide() / 2) {
}

double	FWHM2Evaluator::evaluate(FocusableImage image) {
	double	fwhm = 0;
	ImagePoint	c = _center;
	double	r = _radius;
	if (_radius <= 1) {
		ImagePoint	c = image->center();
		double	r = std::min(image->size().width(),
				image->size().height()) / 2;
		fwhm = focusFWHM2(image, c, r);
	} else {
		fwhm = focusFWHM2(image, _center, _radius);
	}
	FWHMInfo	fwhminfo = focusFWHM2_extended(image, c, r);

	// first build the red channel from the mask
	Image<unsigned char>    *red
		= dynamic_cast<Image<unsigned char> *>(&*fwhminfo.mask);
	if (NULL == red) {
		throw std::logic_error("internal error, mask has not 8bit pixel type");
	}

	// then build the green channel from the original image
	Image<unsigned char>    *green = UnsignedCharImage(image);
	ImagePtr        greenptr(green);

	// create the blue image
	CrosshairAdapter<unsigned char> crosshair(image->size(),
		fwhminfo.maxpoint, 20);
	CircleAdapter<unsigned char>    circle(image->size(), fwhminfo.center,
						fwhminfo.radius);
	MaxAdapter<unsigned char>       blue(crosshair, circle);

	// now use a combination adapter to put all these images together
	// into a single color image
	CombinationAdapter<unsigned char>       combinator(*red, *green, blue);
	Image<RGB<unsigned char> >      *result
		= new Image<RGB<unsigned char> >(combinator);
	_evaluated_image = ImagePtr(result);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "found fwhm = %f", fwhm);
	return fwhm;
}

} // namespace focusing
} // namespace astro
