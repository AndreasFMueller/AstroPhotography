/*
 * GammaTransformBase.cpp
 *
 * (c) 2019 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroAdapter.h>

namespace astro {
namespace adapter {

GammaTransformBase::GammaTransformBase()
	: _minimum(0.), _maximum(0.), _gamma(1.0) {
}

GammaTransformBase::GammaTransformBase(const GammaTransformBase& other)
	: _minimum(other.minimum()), _maximum(other.maximum()),
	  _gamma(other.gamma()) {
}

double	GammaTransformBase::value(double x) const {
	if (x <= _minimum) {
		return 0.;
	}
	double	 s = _maximum - _minimum;
	s *= pow((x - _minimum) / s, _gamma);
//	debug(LOG_DEBUG, DEBUG_LOG, 0, "%.1f -> %.01f", x, s);
	return s / x;
}

} // namespace adapter
} // namespace astro
