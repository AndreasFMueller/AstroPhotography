/*
 * MtLocator.h -- Microtouch Focuser locator
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _MtLocator_h
#define _MtLocator_h

#include <AstroLocator.h>
#include <AstroCamera.h>

namespace astro {
namespace device {
namespace microtouch {

class MtLocator : public astro::device::DeviceLocator {
public:
	MtLocator();
	virtual ~MtLocator();
	virtual std::string	getName() const;
	virtual std::string	getVersion() const;
	virtual std::vector<std::string>	getDevicelist(
		DeviceLocator::device_type device = DeviceLocator::CAMERA);
protected:
	virtual astro::camera::FocuserPtr	getFocuser0(const std::string& name);
};

} // namespace microtouch
} // namespace device
} // namespace astro

#endif /* _MtLocator_h */
