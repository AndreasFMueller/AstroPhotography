/*
 * Mock1Utils.cpp -- some utility functions/classes for the mock1 driver
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Mock1Utils.h>
#include <AstroDebug.h>

namespace astro {
namespace camera {
namespace mock1 {

std::string	cameraname(int id) {
	char	cameraname[20];
	snprintf(cameraname, sizeof(cameraname), "camera:mock1/%d", id);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d -> %s", id, cameraname);
	return std::string(cameraname);
}


} // namespace mock1
} // namespace camera
} // namespace astro
