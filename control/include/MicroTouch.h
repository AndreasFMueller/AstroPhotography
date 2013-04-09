/*
 * MicroTouch.h -- driver for Microtouch focuser motor
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _MicroTouch_h
#define _MicroTouch_h

#include <AstroUSB.h>

namespace astro {
namespace microtouch {

class MicroTouch {
	astro::usb::Device&	device;
public:
	MicroTouch(astro::usb::Device& device);
	uint16_t	position();
	bool	isMoving();
	void	stepUp();
};

} // namespace microtouch
} // namespace astro

#endif /* _MicroTouch_h */
