/*
 * FocusImagePreconditioner.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroFocus.h>
#include "BackgroundAdapter.h"
#include "TopAdapter.h"
#include <AstroFilter.h>

namespace astro {
namespace focusing {

/**
 * \brief Construct a preconditioner
 *
 * \param imageptr	a focusable image for focus evaluation
 */
FocusImagePreconditioner::FocusImagePreconditioner(FocusableImage imageptr)
	: ConstImageAdapter<float>(imageptr->getSize()), _image(*imageptr),
	  _imageptr(imageptr) {
	// find maximum value
	image::filter::Max<float, float>	max;
	_max = max.filter(*_imageptr);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "maximum: %f", _max);

	// find noise background
	image::filter::Mean<float, float>	mean;
	_mean = mean(*_imageptr);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mean=%f", _mean);
	float	limit = _mean;
	BackgroundAdapter	ba(*_imageptr, limit);
	for (int i = 0; i < 3; i++) {
		image::filter::Mean<float, float>	mean;
		limit = mean(ba);
		ba.limit(limit);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "noisefloor: %f", limit);
	_noisefloor = limit;

	// compute a reasonable top value
	TopAdapter	topadapter(imageptr, _max);
	_top = _noisefloor + 3 * (_mean - _noisefloor);
	
	debug(LOG_DEBUG, DEBUG_LOG, 0, "top=%f", _top);
}

/**
 * \brief Pixel access
 *
 * \param x	x-coordinate of pixel
 * \param y	y-coordinate of pixel
 */
float	FocusImagePreconditioner::pixel(int x, int y) const {
	// set nan-Pixels to 0
	float	v = _image.pixel(x, y);
	if (v != v) {
		return 0;
	}
	// rescale pixels
	v = (v - _noisefloor) / (_top - _noisefloor);
	if (v <= 0.) {
		return 0.;
	}
	if (v >= 1.) {
		return 1.;
	}
	return v;
}

} // namespace focus
} // namespace astro
