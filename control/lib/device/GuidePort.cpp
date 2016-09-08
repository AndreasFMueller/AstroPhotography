/* 
 * GuidePort.pp -- Guider Port implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>

namespace astro {
namespace camera {

DeviceName::device_type	GuidePort::devicetype = DeviceName::Guideport;

DeviceName	GuidePort::defaultname(const DeviceName& parent,
			const std::string& unitname) {
	return DeviceName(parent, DeviceName::Guideport, unitname);
}

GuidePort::GuidePort(const DeviceName& name)
	: Device(name, DeviceName::Guideport) {
}

GuidePort::GuidePort(const std::string& name)
	: Device(name, DeviceName::Guideport) {
}

GuidePort::~GuidePort() {
}

} // namespace camera
} // namespace astro
