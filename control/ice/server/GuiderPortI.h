/*
 * GuiderPortI.h -- ICE GuiderPort wrapper class definition
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _GuiderPortI_h
#define _GuiderPortI_h

#include <camera.h>
#include <AstroCamera.h>
#include <DeviceI.h>

namespace snowstar {

class GuiderPortI : virtual public GuiderPort, virtual public DeviceI {
	astro::camera::GuiderPortPtr	_guiderport;
public:
	GuiderPortI(astro::camera::GuiderPortPtr guiderport);
	virtual	~GuiderPortI();

	virtual Ice::Byte	active(const Ice::Current& current);
	virtual void	activate(float, float, const Ice::Current& current);
static	GuiderPortPrx	createProxy(const std::string& name,
		const Ice::Current& current);
};

} // namespace snowstar

#endif /* _GuiderPortI_h */
