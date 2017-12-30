/*
 * LinearLogLuminanceFactor
 *
 * (c) 2017 Prof Dr Andreas Müller Hochschule Rapperswil
 */
#include <AstroTonemapping.h>

namespace astro {
namespace adapter {

static inline double	sqr(double x) { return x * x; }

LinearLogLuminanceFactor::LinearLogLuminanceFactor(double crossover,
	double top, double maximum)
	: _crossover(crossover), _top(top), _maximum(maximum) {
	double	d = maximum - crossover;
	_s = ((top - crossover) - log(maximum / crossover)) / sqr(d);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "s = %f", _s);
}

double	LinearLogLuminanceFactor::operator()(double d) {
	if (d <= _crossover) {
		return 1;
	}
	if (d > _maximum) {
		return _top / d;
	}
	double	m = 1;
	double	b = _crossover;
	double	l = _crossover;
	do {
		l += _crossover;
		m *= 0.5;
		if (d < l) {
			double	v = b + m * (d - l + _crossover);
			return v / d;
		}
		b += m * _crossover;
	} while (1);
	//double	v = _crossover + log(d / _crossover);
	//v += _s * sqr(d - _crossover);
	//double	v = _crossover + (d - _crossover) / (_maximum - _crossover);
	//double	v = _crossover + 0.5 * (d - _crossover);
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "%f -> %f, %f", d, v, v / d);
	//return v / d;
}

} // namespace adapter
} // namespace astro
