/*
 * Control.h
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _ControlBase_h
#define _ControlBase_h

#include <AstroGuiding.h>

namespace astro {
namespace guiding {

/**
 * \brief Base class for control implementation
 */
class ControlBase {
	CalibrationPtr	_calibration;
public:
	CalibrationPtr	calibration() const { return _calibration; }
	void	calibration(CalibrationPtr c) { _calibration = c; }
private:
	double	_deltat;
public:
	double	deltat() const { return _deltat; }
	void	deltat(double d) { _deltat = d; }
	
	ControlBase(CalibrationPtr cal, double deltat);

	virtual Point	correct(const Point& offset);
};

/**
 * \brief simple control mechanism to change the gain of the control
 */
class GainControl : public ControlBase {
	float	_gain[2];
public:
	void	gain(int index, float value) { _gain[index] = value; }
	float	gain(int index) const { return _gain[index]; }

	GainControl(CalibrationPtr cal, double deltat);

	virtual Point	correct(const Point& offset);
};

/*
 * \brief Optimal control solution for the tracking problem
 */
class OptimalControl : public ControlBase {
public:
	OptimalControl(CalibrationPtr cal, double deltat);
	virtual Point	correct(const Point& offset);
};

} // namespace guiding
} // namespace astro

#endif /* _Control_h */
