/*
 * HSLbase.cpp -- HSL base class implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroPixel.h>
#include <math.h>

namespace astro {
namespace image {

static inline double	l(double s) {
	if (s < 0) { return 0; }
	if (s > 1) { return 1; }
	return s;
}

static double	mod2(double x) {
	while (x < 0) {
		x += 2;
	}
	while (x > 2) {
		x -= 2;
	}
	return x;
}

HSLBase::HSLBase(double hue, double saturation, double luminance)
	: H(hue), S(saturation), L(luminance) {
	if (saturation == 0) {
		_r = _g = _b = luminance;
		return;
	}
	double	hprime = hue / (M_PI / 3);
	double	chroma = (1 - fabs(2 * luminance - 1)) * saturation;
	double	x = chroma * (1 - fabs(mod2(hprime) - 1));
	if (hue < 1) {
				_r = chroma; _g = x;      _b = 0;
	} else if (hue < 2) {
				_r = x;      _g = chroma; _b = 0;
	} else if (hue < 3) {
				_r = 0;      _g = chroma; _b = x;
	} else if (hue < 4) {
				_r = 0;      _g = x;      _b = chroma;
	} else if (hue < 5) {
				_r = x;      _g = 0;      _b = chroma;
	} else {
				_r = chroma; _g = 0;      _b = x;
	}
	double m = luminance - chroma / 2;
	_r = l(_r + m);
	_g = l(_g + m);
	_b = l(_b + m);
}

template<>
HSL<double>::HSL(double hue, double saturation, double luminance)
        : HSLBase(hue, saturation, luminance) {
}
template<>
double  HSL<double>::R() const { return r(); }
template<>
double  HSL<double>::G() const { return g(); }
template<>
double  HSL<double>::B() const { return b(); }

template<>
HSL<float>::HSL(double hue, double saturation, float luminance)
        : HSLBase(hue, saturation, luminance) {
}
template<>
float   HSL<float>::R() const { return r(); }
template<>
float   HSL<float>::G() const { return g(); }
template<>
float   HSL<float>::B() const { return b(); }


} // namespace image
} // namespace astro
