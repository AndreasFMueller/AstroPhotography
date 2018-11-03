/*
 * FocusImagePreconditioner.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroFocus.h>
#include <BackgroundAdapter.h>
#include <AstroFilter.h>

namespace astro {
namespace focusing {

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
	image::filter::Mean2<float, float>	mean2;
	float	m2 = mean2(*_imageptr);
	_stddev = sqrt(m2 - _mean * _mean);
	_top = _mean + 3 * _stddev;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "E(x^2)=%f, stddev=%f, top=%f",
		m2, _stddev, _top);
}

float	FocusImagePreconditioner::pixel(int x, int y) const {
	float	v = (_image.pixel(x, y) - _noisefloor) / (_top - _noisefloor);
	if (v < 0.) {
		return 0.;
	}
	if (v > 1.) {
		return 1.;
	}
	return v;
}

} // namespace focus
} // namespace astro
