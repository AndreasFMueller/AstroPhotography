/*
 * GaussNoiseAdapter.cpp -- implementation of 
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroAdapter.h>
#include <cmath>
#include <limits>
#include <AstroDebug.h>

namespace astro {
namespace adapter {

GaussNoiseAdapter::GaussNoiseAdapter(const ImageSize& size, double mu,
	double sigma, double limit)
	: NoiseAdapter(size), _mu(mu), _sigma(sigma), _limit(limit) {
}

/*
We need to find a solution for the equation

  erf(x) = value

where

  erf(x) = 2/sqrt(pi) * integral_0^x exp(-t^2) dt

we can solve the equation with the help of newton's algorithm

  x_n+1 = x_n - f(x_n) / f'(x_n)

The derivative can be computed from the integral formular for erf(x):

  erf'(x) = 2/sqrt(pi) * exp(-x^2)

*/

static double	ierf(double y) {
	double	x = 0;
	int	counter = 0;
	double	delta = std::numeric_limits<double>::infinity();
	do {
		double	xnew = x - (erf(x) - y) / (sqrt(2 / M_PI) * exp(x*x/2));
		delta = fabs(xnew - x);
		x = xnew;
		counter++;
	} while ((delta > 1e-6) && (counter < 20));
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "erf(%f) = %f, delta = %f", x, y, delta);
	return x;
}

double	GaussNoiseAdapter::pixel(int /* x */, int /* y */) const {
	double	value = 2 * (random() / (double)2147483647) - 1;
	double	x = _mu  + _sigma * ierf(value);
	if (x < 0) {
		return x;
	}
	if (x > _limit) {
		return _limit;
	}
	return x;
}

} // namespace adapter
} // namespace astro
