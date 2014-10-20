/*
 * GpioGuiderport.h -- interface class for Gpio guiderports
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswi
 */
#ifndef _GpioGuiderport_h
#define _GpioGuiderport_h

#include <AstroDevice.h>
#include <AstroCamera.h>
#include <Serial.h>

namespace astro {
namespace device {
namespace gpio {

class GpioGuiderport : public astro::camera::GuiderPort {
public:
	GpioGuiderport(const std::string& devicename);
	virtual ~GpioGuiderport();

	// accessors
	virtual uint8_t	active();
	virtual void	activate(float raplus, float raminus,
				float declus, float decminus);
};

} // namepace gpio
} // namespace device
} // namespace astro

#endif /* _GpioGuiderport */
