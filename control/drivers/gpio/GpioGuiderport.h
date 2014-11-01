/*
 * GpioGuiderport.h -- interface class for Gpio guiderports
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswi
 */
#ifndef _GpioGuiderport_h
#define _GpioGuiderport_h

#include <AstroDevice.h>
#include <AstroCamera.h>
#include <BasicGuiderport.h>

namespace astro {
namespace device {
namespace gpio {

class GpioGuiderport : public astro::camera::BasicGuiderport {
public:
	GpioGuiderport(const std::string& devicename);
	virtual ~GpioGuiderport();

	virtual void	do_activate(uint8_t active);
};

} // namepace gpio
} // namespace device
} // namespace astro

#endif /* _GpioGuiderport */
