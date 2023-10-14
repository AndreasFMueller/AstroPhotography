/*
 * HSVbase.cpp -- HSV base class implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroPixel.h>
#include <AstroDebug.h>
#include <math.h>

namespace astro {
namespace image {

static inline double	limit(double s) {
	if (s < 0) { return 0; }
	return s;
}

static inline double	mod2(double x) {
	while (x < 0) {
		x += 2;
	}
	while (x > 2) {
		x -= 2;
	}
	return x;
}

/**
 * \brief convert HSV color to RGB
 *
 * This conversion routine es based on the Wikipedia article
 * https://en.wikipedia.org/wiki/HSL_and_HSV
 *
 * \param hue		color angle between 0 and 2\pi
 * \param saturation	color saturation between 0 and 1
 * \param value		value, any positive real number
 */
HSVBase::HSVBase(double hue, double saturation, double value)
	: _h(hue), _s(saturation), _v(value) {
	if (saturation == 0) {
		_r = _g = _b = value;
		return;
	}
	const double	twopi = 2 * M_PI;
	while (_h < 0) {
		_h += twopi;
	}
	while (_h > twopi) {
		_h -= twopi;
	}
	double	hprime = _h / (twopi / 6);
	double	C = saturation * value;
	double	X = C * (1 - fabs(mod2(hprime) - 1));
	if (hprime < 1) {
					_r = C; _g = X; _b = 0;
	} else if (hprime < 2) {
					_r = X; _g = C; _b = 0;
	} else if (hprime < 3) {
					_r = 0; _g = C; _b = X;
	} else if (hprime < 4) {
					_r = 0; _g = X; _b = C;
	} else if (hprime < 5) {
					_r = X; _g = 0; _b = C;
	} else {
					_r = C; _g = 0; _b = X;
	}
	double m = value - C;
//debug(LOG_DEBUG, DEBUG_LOG, 0, "hprime = %.2f, C = %.2f, X = %.2f, m = %.2f, (%.2f,%.2f,%.2f)\n", hprime, C, X, m, _r, _g, _b);
	_r = limit(_r + m);
	_g = limit(_g + m);
	_b = limit(_b + m);
}

template<>
HSV<double>::HSV(double hue, double saturation, double luminance)
        : HSVBase(hue, saturation, luminance) {
}
template<>
double  HSV<double>::R() const { return r(); }
template<>
double  HSV<double>::G() const { return g(); }
template<>
double  HSV<double>::B() const { return b(); }

template<>
HSV<float>::HSV(double hue, double saturation, float luminance)
        : HSVBase(hue, saturation, luminance) {
}
template<>
float   HSV<float>::R() const { return r(); }
template<>
float   HSV<float>::G() const { return g(); }
template<>
float   HSV<float>::B() const { return b(); }


} // namespace image
} // namespace astro
