/*
 * FocuserI.h -- ICE Focuser class wrapper definition
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _FocuserI_h
#define _FocuserI_h

#include <camera.h>
#include <AstroCamera.h>
#include <DeviceI.h>

namespace snowstar {

class FocuserI : virtual public Focuser, virtual public DeviceI {
	astro::camera::FocuserPtr	_focuser;
public:
	FocuserI(astro::camera::FocuserPtr focuser);
	virtual ~FocuserI();
	virtual int	min(const Ice::Current& current);
	virtual int	max(const Ice::Current& current);
	virtual int	backlash(const Ice::Current& current);
	virtual int	current(const Ice::Current& current);
	virtual void	set(int position, const Ice::Current& current);
};

} // namespace snowstar

#endif /* _FocuserI_h */
