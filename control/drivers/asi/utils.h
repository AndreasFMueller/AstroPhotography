/*
 * utils.h -- utilities for the asi driver implementation
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _utils_h
#define _utils_h

#include <AstroCamera.h>

namespace astro {
namespace camera {
namespace asi {

DeviceName	asiCameraName(int index);
DeviceName	asiCcdName(int index, const std::string& imgtype);
DeviceName	asiCoolerName(int index, const std::string& imgtype);
DeviceName	asiGuideportName(int index);

} // namespace asi
} // namespace camera
} // namespace astro

#endif /* _utils_h */
