/*
 * AtikCamera.cpp -- ATIK camera implementation
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AtikCamera.h>
#include <AtikUtils.h>

namespace astro {
namespace camera {
namespace atik {

AtikCamera::AtikCamera(::AtikCamera *camera)
	: Camera(cameraname(camera)), _camera(camera) {
}

} // namespace atik
} // namespace camera
} // namespace astro
