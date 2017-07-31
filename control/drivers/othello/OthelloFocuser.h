/*
 * OthelloFocuser.h -- Othello focuser hardware definitions
 *
 * (c) 2016 prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _OthelloFocuser_h
#define _OthelloFocuser_h

#include <AstroCamera.h>
#include <AstroUSB.h>

namespace astro {
namespace camera {
namespace othello {

class OthelloFocuser : public astro::camera::Focuser {
	astro::usb::DevicePtr	deviceptr;
	OthelloFocuser(const OthelloFocuser& other);
	OthelloFocuser&	operator=(const OthelloFocuser& other);
public:
	OthelloFocuser(astro::usb::DevicePtr _deviceptr);
	~OthelloFocuser();
	virtual long	min();
	virtual long	max();
	virtual long	current();
	virtual void	set(long value);
};

} // namespace othello
} // namespace camera
} // namespace astro

#endif /* _OthelloFocuser_h */
