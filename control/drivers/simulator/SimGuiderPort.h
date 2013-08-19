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
public:
	SimGuiderPort(SimLocator& locator);
	virtual uint8_t	active();
	virtual void	activate(float raplus, float raminus,
		float decplus, float decminus);
};

} // namespace simulator
} // namespace camera
} // namespace astro


#endif /* _SimGuiderPort_h */
