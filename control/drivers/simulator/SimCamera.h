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
	virtual bool	hasGuidePort() const { return true; }
	virtual GuidePortPtr	getGuidePort0();
	virtual std::string	userFriendlyName() const;
};

} // namespace simulator
} // namespace camera
} // namespace astro 

#endif /* _SimCamera_h */
