/* 
 * GuidderPort.pp -- Guider Port implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>

namespace astro {
namespace camera {

DeviceName	GuiderPort::defaultname(const DeviceName& parent, const std::string& unitname) {
	return DeviceName(parent, DeviceName::Guiderport, unitname);
}

GuiderPort::GuiderPort(const DeviceName& name) : Device(name) {
}

GuiderPort::GuiderPort(const std::string& name) : Device(name) {
}

GuiderPort::~GuiderPort() {
}

} // namespace camera
} // namespace astro
