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

/**
 * \brief Construct a Guide Port object for a camera
 *
 * \param camera	Camera of which to get the guide port
 */
AtikGuideport::AtikGuideport(AtikCamera& camera)
	: GuidePort(guideportname(camera)), _camera(camera) {
}

/**
 * \brief Get active pins on the port
 *
 * XXX This method is not implemented
 *
 * \return bitmap of active pins
 */
uint8_t	AtikGuideport::active() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "AtikGuideport::active not implemented");
	return 0;
}

/**
 * \brief Activ
 *
 * XXX This method is not implemented
 *
 * \param raplus	duration to activae RA+
 * \param raminus	duration to activae RA-
 * \param decplus	duration to activae DEC+
 * \param decminus	duration to activae DEC-
 */
void	AtikGuideport::activate(float raplus, float raminus,
		float decplus, float decminus) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"AtikGuiderport::activate(%.2f, %.2f, %.2f, %.2f) "
		"not implemented", raplus, raminus, decplus, decminus);
}

} // namespace atik
} // namespace camera
} // namespace astro
