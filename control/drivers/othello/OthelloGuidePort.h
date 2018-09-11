/*
 * OthelloGuidePort.h -- Othello guider port
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _OthelloGuidePort_h
#define _OthelloGuidePort_h

#include <AstroCamera.h>
#include <BasicGuideport.h>
#include <AstroUSB.h>

namespace astro {
namespace camera {
namespace othello {

/**
 * \brief Othello Guider Port interface
 *
 * This class encapsulates a thread that handles the timing of the guider
 * port output signals.
 */
class OthelloGuidePort : public GuidePort {
	astro::usb::DevicePtr	deviceptr;
public:
	OthelloGuidePort(astro::usb::DevicePtr _deviceptr);
	virtual ~OthelloGuidePort();
	virtual void	activate(float raplus, float raminus, float decplus,
				float decminus);
	virtual uint8_t	active();
};

} // namespace othello
} // namespace camera
} // namespace astro

#endif /* _OthelloGuidePort_h */
