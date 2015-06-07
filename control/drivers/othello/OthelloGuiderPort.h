/*
 * OthelloGuiderPort.h -- Starlight Express guider port
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _OthelloGuiderPort_h
#define _OthelloGuiderPort_h

#include <AstroCamera.h>
#include <BasicGuiderport.h>
#include <AstroUSB.h>

namespace astro {
namespace camera {
namespace othello {

/**
 * \brief Starlight Express Guider Port interface
 *
 * This class encapsulates a thread that handles the timing of the guider
 * port output signals.
 */
class OthelloGuiderPort : public GuiderPort {
	astro::usb::DevicePtr	deviceptr;
public:
	OthelloGuiderPort(astro::usb::DevicePtr _deviceptr);
	virtual ~OthelloGuiderPort();
	virtual void	activate(float raplus, float raminus, float decplus,
				float decminus);
	virtual uint8_t	active();
};

} // namespace othello
} // namespace camera
} // namespace astro

#endif /* _OthelloGuiderPort_h */
