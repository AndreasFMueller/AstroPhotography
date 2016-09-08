/*
 * GpioGuideport.h -- interface class for Gpio guideports
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswi
 */
#ifndef _GpioGuideport_h
#define _GpioGuideport_h

#include <AstroDevice.h>
#include <AstroCamera.h>
#include <BasicGuideport.h>

namespace astro {
namespace device {
namespace gpio {

class GpioGuideport : public astro::camera::BasicGuideport {
public:
	GpioGuideport(const std::string& devicename);
	virtual ~GpioGuideport();

	virtual void	do_activate(uint8_t active);
};

} // namepace gpio
} // namespace device
} // namespace astro

#endif /* _GpioGuideport */
