/*
 * AtikGuideport.cpp -- implementation of Atik Guideport class
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AtikGuideport.h>
#include <AtikUtils.h>

namespace astro {
namespace camera {
namespace atik {

AtikGuideport::AtikGuideport(AtikCamera& camera)
	: GuidePort(guideportname(camera)), _camera(camera) {
}

uint8_t	AtikGuideport::active() {
	return 0;
}

void	AtikGuideport::activate(float raplus, float raminus,
		float decplus, float decminus) {
}

} // namespace atik
} // namespace camera
} // namespace astro
