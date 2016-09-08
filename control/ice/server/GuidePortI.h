/*
 * GuidePortI.h -- ICE GuidePort wrapper class definition
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _GuidePortI_h
#define _GuidePortI_h

#include <camera.h>
#include <AstroCamera.h>
#include <DeviceI.h>

namespace snowstar {

class GuidePortI : virtual public GuidePort, virtual public DeviceI {
	astro::camera::GuidePortPtr	_guideport;
public:
	GuidePortI(astro::camera::GuidePortPtr guideport);
	virtual	~GuidePortI();

	virtual Ice::Byte	active(const Ice::Current& current);
	virtual void	activate(float, float, const Ice::Current& current);
static	GuidePortPrx	createProxy(const std::string& name,
		const Ice::Current& current);
};

} // namespace snowstar

#endif /* _GuidePortI_h */
