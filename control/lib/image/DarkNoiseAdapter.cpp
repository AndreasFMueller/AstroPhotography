/*
 * DarkNoiseAdapter.cpp -- poisson distributed noise 
 *
 * (c) 2015 Prof Dr Andreas Mueller
 */
#include <AstroAdapter.h>
#include <AstroDebug.h>
#include <cmath>

namespace astro {
namespace adapter {

DarkNoiseAdapter::DarkNoiseAdapter(const ImageSize& size, double temperature,
	double darkcurrent, int electrons_per_pixel)
	: NoiseAdapter(size), _electrons_per_pixel(electrons_per_pixel) {
	_lambda = darkcurrent * pow(2, (temperature - 273.13) / 7.);
	setup();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "lambda = %f, nlevels = %d", _lambda,
		nlevels);
}

DarkNoiseAdapter::DarkNoiseAdapter(const ImageSize& size, double lambda,
	int electrons_per_pixel)
	: NoiseAdapter(size), _electrons_per_pixel(electrons_per_pixel),
	  _lambda(lambda) {
	setup();
}

DarkNoiseAdapter::~DarkNoiseAdapter() {
	if (levels) {
		delete levels;
		levels = NULL;
	}
}

void	DarkNoiseAdapter::setup() {
	nlevels = _lambda * 2 + 20;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "nlevels = %f", nlevels);
	levels = new double[nlevels];
	double	norm = exp(-_lambda);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "norm = %f", norm);
	double	p = 1.;
	double	s = 0.;
	int	k = 0;
	while (k < nlevels) {
		s += p;
		levels[k] = norm * s;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "levels[%d] = %.16f",
			k, levels[k]);
		if (k > 1) {
			if (levels[k] >= 1.0) {
				nlevels = k + 1;
				return;
			}
		}
		k++;
		p *= _lambda / k;
	}
}

int	DarkNoiseAdapter::poisson2() const {
	if (NULL == levels) {
		return poisson();
	}
	double	randomvalue = random() / 2147483647.;
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "randomvalue = %f", randomvalue);
	if (randomvalue > levels[nlevels - 1]) {
		return nlevels;
	}
	if (randomvalue < levels[0]) {
		return 0;
	}
	int	min = 0, max = nlevels - 1;
	while ((max - min) > 1) {
		int	m = (max + min) / 2;
		if (m == max) {
			m = max - 1;
		}
		if (m == min) {
			m = min + 1;
		}
		if (levels[m] <= randomvalue) {
			min = m;
		}
		if (randomvalue < levels[m]) {
			max = m;
		}
	//	debug(LOG_DEBUG, DEBUG_LOG, 0, "[%d, %d] = [%f, %f]",
	//		min, max, levels[min], levels[max]);
	}
	return max;
}

int	DarkNoiseAdapter::poisson() const {
	double	randomvalue = exp(_lambda) * random() / 2147483647.;
	double	p = 1.;
	double	s = 0.;
	int	k = 0;
	while ((randomvalue > s) && (k < 10000)) {
		s += p;
		if (randomvalue <= s) {
			return k;
		}
		k++;
		p *= _lambda / k;
	}
	return k;
}

double	DarkNoiseAdapter::pixel(int /* x */, int /* y */) const {
	return poisson2() / (double)_electrons_per_pixel;
}

} // namespace adapter
} // namespace astro
