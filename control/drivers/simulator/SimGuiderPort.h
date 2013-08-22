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

class SimGuiderPort : public GuiderPort {
	SimLocator&	_locator;
	double	starttime;
	Point	_drift;
	double	_omega;
public:
	SimGuiderPort(SimLocator& locator);
	virtual uint8_t	active();
	virtual void	activate(float raplus, float raminus,
		float decplus, float decminus);

	// parameters for the simulation
	const Point&	drift() const { return _drift; }
	void	drift(const Point& drift) { _drift = drift; }
	double	omega() const { return _omega; }
	void	omega(double omega) { _omega = omega; }

	// retrive offset and rotation
	Point	offset();
	double	alpha();
};

} // namespace simulator
} // namespace camera
} // namespace astro


#endif /* _SimGuiderPort_h */
