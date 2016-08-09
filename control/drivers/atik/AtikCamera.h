/*
 * AtikCamera.h -- Atik camera class
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _AtikCamera_h
#define _AtikCamera_h

#include <atikccdusb.h>
#include <AstroCamera.h>

namespace astro {
namespace camera {
namespace atik {

class AtikCamera : public Camera {
	::AtikCamera	*_camera;
public:
	AtikCamera(::AtikCamera *camera);
};

} // namespace atik
} // namespace camera
} // namespace astro

#endif /* _AtikCamera_h */
