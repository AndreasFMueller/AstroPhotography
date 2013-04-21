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

/**
 * \brief 
 */
class MicroTouch {
	astro::usb::Device&	device;
	astro::usb::EndpointDescriptorPtr	outendpoint;
	astro::usb::EndpointDescriptorPtr	inendpoint;
public:
	MicroTouch(astro::usb::Device& device);
	uint16_t	getWord(uint8_t code);
	uint16_t	position();
	void		setPosition(uint16_t position);

	uint8_t		getByte(uint8_t code);
	bool	isMoving();
	bool	isTemperatureCompensating();

	float	getTemperature();

	void	stepUp();
};

} // namespace microtouch
} // namespace astro

#endif /* _MicroTouch_h */
