/*
 * Qhy2Locator.h
 *
 * (c) 2020 Prof Dr Andreas Mueller, QSI camera locator
 */
#ifndef _Qhy2Locator_h
#define _Qhy2Locator_h

#include <AstroLocator.h>
#include <AstroCamera.h>
#include <qhyccd.h>

using namespace astro::device;

namespace astro {
namespace camera {
namespace qhy2 {

/**
 * \brief The Locator class for QSI devices
 *
 * This is essentially a wrapper about the QSI repository functions
 */
class Qhy2CameraLocator : public DeviceLocator {
	static std::map<std::string,qhyccd_handle*>	_camera_handles;
public:
	Qhy2CameraLocator();
	virtual ~Qhy2CameraLocator();
	virtual std::string	getName() const;
	virtual std::string	getVersion() const;
	virtual std::vector<std::string>	getDevicelist(DeviceName::device_type device = DeviceName::Camera);
	static qhyccd_handle	*handleForName(const std::string& qhyname);
	static qhyccd_handle	*handleForName(const DeviceName& devicename);
protected:
	virtual CameraPtr	getCamera0(const DeviceName& name);
	virtual CoolerPtr	getCooler0(const DeviceName& name);
	virtual CcdPtr	getCcd0(const DeviceName& name);
	virtual GuidePortPtr	getGuidePort0(const DeviceName& name);
};

} // namespace qhy2
} // namespace camera
} // namespace astro

#endif /* _Qhy2Locator_h */
