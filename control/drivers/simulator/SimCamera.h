/*
 * SimCamera.h -- Simulator Camera
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SimCamera_h
#define _SimCamera_h

#include <SimLocator.h>

namespace astro {
namespace camera {
namespace simulator {

class SimCamera : public Camera {
	SimLocator&	_locator;
public:
	SimCamera(SimLocator& locator);
	virtual CcdPtr	getCcd0(size_t ccdid);
	virtual bool	hasFilterWheel() const { return true; }
	virtual FilterWheelPtr	getFilterWheel0();
	virtual bool	hasGuiderPort() const { return true; }
	virtual GuiderPortPtr	getGuiderPort0();
};

} // namespace simulator
} // namespace camera
} // namespace astro 

#endif /* _SimCamera_h */
