/*
 * ExposureTimer.cpp -- implementation of the exposure timer
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroLoop.h>
#include <AstroFilterfunc.h>

using namespace astro::image::filter;

namespace astro {
namespace task {

/**
 * \brief Update the exposure time
 */
void	ExposureTimer::update(ImagePtr image) {
	double	actualvalue = 1.;
	switch (_method) {
	case NONE:
		actualvalue = _targetvalue;
		break;
	case MEAN:
		actualvalue = mean(image);
		break;
	case MEDIAN:
		actualvalue = median(image);
		break;
	}
	double	x = _targetvalue / actualvalue;
	double  y = x - 1;
        double  f =  y * (1 - _relaxation * exp(-y * y)) + 1;
        debug(LOG_DEBUG, DEBUG_LOG, 0, "x = %f, scalefactor = %f", x, f);
	_exposuretime *= f;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "actual = %f, target = %f, new exp = %f",
		actualvalue, _targetvalue, _exposuretime);
	if (_exposuretime > _limit) {
		_exposuretime = _limit;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "limit exposure time to %f",
			_exposuretime);
	}
	if (_exposuretime < _minimum) {
		_exposuretime = _minimum;
	}
}

} // namespace task
} // namespace astro
