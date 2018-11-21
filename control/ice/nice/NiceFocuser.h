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

class NiceFocuser : public Focuser, public NiceDevice {
	snowstar::FocuserPrx	_focuser;
public:
	NiceFocuser(snowstar::FocuserPrx focuser, const DeviceName& name);
	virtual ~NiceFocuser();
	virtual long	min();
	virtual long	max();
	virtual long	current();
	virtual long	backlash();
	virtual void	set(long value);
};

} // namespace nice
} // namespace camera
} // namespace astro

#endif /* _NiceFocuser_h */
