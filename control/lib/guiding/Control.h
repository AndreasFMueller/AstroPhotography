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
	double	_deltat;
public:
	double	deltat() const { return _deltat; }
	void	deltat(double d) { _deltat = d; }
protected:
	double	_parameters[2];
	virtual void	parameter(int index, double v) {
		_parameters[index] = v;
	}
	virtual double	parameter(int index) const {
		return _parameters[index];
	}
public:
	ControlBase(double deltat);
	virtual ~ControlBase();

	virtual Point	correct(const Point& offset);
};

/**
 * \brief simple control mechanism to change the gain of the control
 */
class GainControl : public ControlBase {
public:
	void	gain(int index, float value) { parameter(index, value); }
	float	gain(int index) const { return parameter(index); }

	GainControl(double deltat);
	virtual ~GainControl();

	virtual Point	correct(const Point& offset);
};

// forward declaration of the KalmanFilter class
class KalmanFilter;

/*
 * \brief Optimal control solution for the tracking problem
 *
 * Optimal control works by correcting not the currently measured offset
 * but the Kalman filtered offset. This is the gist of the separation 
 * principle (see Donald E. Caitlin, Estimation, Control, and the discrete
 * Kalman Filter, Theorem 8.3.3, p. 186)
 */
class OptimalControl : public ControlBase {
	KalmanFilter	*_kalmanfilter;
	OptimalControl(const OptimalControl& other) = delete;
	OptimalControl&	operator=(const OptimalControl& other) = delete;
	void	update(const Point& z);
public:
	virtual void	parameter(int index, double v);
	void	measurementerror(double m);
	double	measurementerror() const;
	void	systemerror(double s);
	double	systemerror() const;
	OptimalControl(double deltat);
	virtual ~OptimalControl();
	virtual Point	correct(const Point& offset);
	Point	offset() const;
};

} // namespace guiding
} // namespace astro

#endif /* _Control_h */
