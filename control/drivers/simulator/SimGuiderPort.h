/*
 * SimGuiderPort.h -- GuiderPort definition for the simulator camera
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SimGuiderPort_h
#define _SimGuiderPort_h

#include "SimLocator.h"

namespace astro {
namespace camera {
namespace simulator {

class SimGuiderPort : public GuiderPort {
	SimLocator&	_locator;
	double	starttime;
	double	_driftx;
	double	_drifty;
	double	_omega;
public:
	SimGuiderPort(SimLocator& locator);
	virtual uint8_t	active();
	virtual void	activate(float raplus, float raminus,
		float decplus, float decminus);

	// parameters for the simulation
	double	driftx() const { return _driftx; }
	void	driftx(double driftx) { _driftx = driftx; }
	double	drifty() const { return _drifty; }
	void	drifty(double drifty) { _drifty = drifty; }
	double	omega() const { return _omega; }
	void	omega(double omega) { _omega = omega; }
	std::pair<double, double>	offset();
};

} // namespace simulator
} // namespace camera
} // namespace astro


#endif /* _SimGuiderPort_h */
