/*
 * NiceFocuser.h -- wrapper for focuser in ICE
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _NiceFocuser_h
#define _NiceFocuser_h

#include <AstroCamera.h>
#include <camera.h>
#include <NiceDevice.h>

namespace astro {
namespace camera {
namespace nice {

class NiceFocuser;

class NiceFocuserCallback : public snowstar::FocuserCallback {
	NiceFocuser&	_focuser;
public:
	NiceFocuserCallback(NiceFocuser& focuser) : _focuser(focuser) { }
	virtual void	movement(Ice::Long fromposition, Ice::Long toposition,
				const Ice::Current& current);
	virtual void	info(Ice::Long fromposition, bool on_target,
				const Ice::Current& current);
	virtual void	stop(const Ice::Current& current);
};

class NiceFocuser : public Focuser, public NiceDevice {
	snowstar::FocuserPrx	_focuser;
	Ice::ObjectPtr	_focuser_callback;
	Ice::Identity	_focuser_identity;
public:
	NiceFocuser(snowstar::FocuserPrx focuser, const DeviceName& name);
	virtual ~NiceFocuser();
	virtual long	min();
	virtual long	max();
	virtual long	current();
	virtual long	backlash();
	virtual void	set(Ice::Long value);
};

} // namespace nice
} // namespace camera
} // namespace astro

#endif /* _NiceFocuser_h */
