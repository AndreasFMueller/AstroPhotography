/*
 * SimGuiderPort.h -- GuiderPort definition for the simulator camera
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SimGuiderPort_h
#define _SimGuiderPort_h

#include "SimLocator.h"
#include <AstroTypes.h>

namespace astro {
namespace camera {
namespace simulator {

/**
 * \brief Simulated guider port
 */
class SimGuiderPort : public GuiderPort {
	SimLocator&	_locator;
	double	starttime;
	Point	_drift;
	Point	_offset;
	double	_omega;
	double	lastactivation;
	double	ra, dec;
	double	pixelspeed;
	Point	_ravector, _decvector;
	SimGuiderPort(const SimGuiderPort& other);
	SimGuiderPort&	operator=(const SimGuiderPort& other);
public:
	void	update();
	SimGuiderPort(SimLocator& locator);
	virtual uint8_t	active();
	virtual void	activate(float raplus, float raminus,
		float decplus, float decminus);

	// parameters for the simulation
	const Point&	drift() const { return _drift; }
	void	drift(const Point& drift) { _drift = drift; }

	double	omega() const { return _omega; }
	void	omega(double omega) { _omega = omega; }

	const Point&	ravector() const { return _ravector; }
	void	ravector(const Point& ravector) { _ravector = ravector; }

	const Point&	decvector() const { return _decvector; }
	void	decvector(const Point& decvector) { _decvector = decvector; }

	// retrive offset and rotation
	Point	offset();
	double	alpha();
};

} // namespace simulator
} // namespace camera
} // namespace astro


#endif /* _SimGuiderPort_h */
