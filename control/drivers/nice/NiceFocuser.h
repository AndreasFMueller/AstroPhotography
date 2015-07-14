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
	virtual unsigned short	min();
	virtual unsigned short	max();
	virtual unsigned short	current();
	virtual unsigned short	backlash();
	virtual void	set(unsigned short value);
};

} // namespace nice
} // namespace camera
} // namespace astro

#endif /* _NiceFocuser_h */
