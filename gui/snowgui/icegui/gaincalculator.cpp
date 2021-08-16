/*
 * gaincalculator.cpp
 *
 * (c) 2021 Prof Dr Andreas Müller, OST Ostschweizer Fachhochschule
 */
#include "gaincalculator.h"
#include <AstroDebug.h>

namespace snowgui {

GainCalculator::GainCalculator() : _min(0), _max(1) {
	setup();
}

GainCalculator::GainCalculator(float min, float max) : _min(min), _max(max) {
	setup();
}

GainCalculator::GainCalculator(const std::pair<float, float>& i)
	: _min(i.first), _max(i.second) {
	setup();
}

GainCalculator::GainCalculator(const snowstar::Interval& i)
	: _min(i.min), _max(i.max) {
	setup();
}

void	GainCalculator::interval(const snowstar::Interval& i) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "gain interval: [%.2f,%.2f]",
		i.min, i.max);
	_min = i.min;
	_max = i.max;
	setup();
}

void	GainCalculator::interval(const std::pair<float,float>& i) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "gain interval: [%.2f,%.2f]",
		i.first, i.second);
	_min = i.first;
	_max = i.second;
	setup();
}

void	GainCalculator::min(float m) {
	_min = m;
	setup();
}

void	GainCalculator::max(float m) {
	_max = m;
	setup();
}

void	GainCalculator::setup() {
	_slope = (_max - _min) / 100.;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "slope = %.3f", _slope);
}

int	GainCalculator::gainToSlider(float gain) {
	return (gain - _min) / _slope;
}

float	GainCalculator::sliderToGain(int slider) {
	return _min + slider * _slope;
}

} // namespace snowgui
