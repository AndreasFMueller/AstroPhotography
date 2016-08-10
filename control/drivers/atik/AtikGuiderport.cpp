/*
 * AtikGuiderport.cpp -- implementation of Atik Guiderport class
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AtikGuiderport.h>
#include <AtikUtils.h>

namespace astro {
namespace camera {
namespace atik {

AtikGuiderport::AtikGuiderport(::AtikCamera *camera)
	: GuiderPort(guiderportname(camera)), _camera(camera) {
}

} // namespace atik
} // namespace camera
} // namespace astro
