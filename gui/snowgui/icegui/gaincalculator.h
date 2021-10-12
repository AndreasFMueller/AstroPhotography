/*
 * gaincalculator.h
 *
 * (c) 2021 Prof Dr Andreas Müller, OST Ostschweizer Fachhochschule
 */
#ifndef _gaincalculator_h
#define _gaincalculator_h

#include <device.h>

namespace snowgui {

class GainCalculator {
	float	_min;
	float	_max;
	float	_slope;
	void	setup();
public:
	float	min() const { return _min; }
	void	min(float m);
	float	max() const { return _max; }
	void	max(float m);
	float	slope() const { return _slope; }
	std::pair<float, float>	interval() const {
		return std::make_pair(_min, _max);
	}
	void	interval(const std::pair<float, float>& i);
	void	interval(const snowstar::Interval& i);

	GainCalculator();
	GainCalculator(float min, float max);
	GainCalculator(const std::pair<float, float>& i);
	GainCalculator(const snowstar::Interval& i);

	int	gainToSlider(float gain);
	float	sliderToGain(int slider);
};

} // namespace snowgui

#endif /* _gaincalculator_h */
