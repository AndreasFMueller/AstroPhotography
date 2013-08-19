/* 
 * GuidderPort.pp -- Guider Port implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>

namespace astro {
namespace camera {

GuiderPort::GuiderPort(const std::string& name) : Device(name) {
}

GuiderPort::~GuiderPort() {
}

} // namespace camera
} // namespace astro
