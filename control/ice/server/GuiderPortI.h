/*
 * GuiderPortI.h -- ICE GuiderPort wrapper class definition
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _GuiderPortI_h
#define _GuiderPortI_h

#include <camera.h>
#include <AstroCamera.h>

namespace snowstar {

class GuiderPortI : public GuiderPort {
	astro::camera::GuiderPortPtr	_guiderport;
public:
	GuiderPortI(astro::camera::GuiderPortPtr guiderport)
		: _guiderport(guiderport) { }
	virtual	~GuiderPortI() { }

	virtual std::string	getName(const Ice::Current& current);
	virtual Ice::Byte	active(const Ice::Current& current);
	virtual void	activate(float, float, const Ice::Current& current);
};

} // namespace snowstar

#endif /* _GuiderPortI_h */
