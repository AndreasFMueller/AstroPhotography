/*
 * AtikUtils.h
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _AtikUtils_h
#define _AtikUtils_h

#include <atikccdusb.h>
#include <AtikCamera.h>

namespace astro {
namespace camera {
namespace atik {

DeviceName	cameraname(AtikCamera& camera);
DeviceName	cameraname(::AtikCamera *camera);

DeviceName	ccdname(AtikCamera& camera, const std::string& name);
DeviceName	ccdname(::AtikCamera *camera, const std::string& name);

DeviceName	filterwheelname(AtikCamera& camera);
DeviceName	filterwheelname(::AtikCamera *camera);

DeviceName	guideportname(AtikCamera& camera);
DeviceName	guideportname(::AtikCamera *camera);

DeviceName	coolername(AtikCamera& camera);
DeviceName	coolername(::AtikCamera *camera);

} // namespace atik
} // namespace camera
} // namespace astro

#endif /* _AtikUtils_h */
