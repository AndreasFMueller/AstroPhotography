/*
 * Qhy2GuidePort.cpp -- Qhy2 camera guide port
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */

#include <Qhy2GuidePort.h>
#include <Qhy2Utils.h>

namespace astro {
namespace camera {
namespace qhy2 {

/**
 * \brief Construct a guideport object 
 *
 * \param _camera	the camera containing the guide port
 */
Qhy2GuidePort::Qhy2GuidePort(Qhy2Camera& _camera)
	: GuidePort(Qhy2Name(_camera.qhyname()).guideportname()),
	  camera(_camera) {
}

/**
 * \brief Destroy the guide port
 */
Qhy2GuidePort::~Qhy2GuidePort() {
}

/**
 * \brief Find out which guide port pins are on
 *
 * There is apparently no method to query the guide port so we have to
 * keep a record of the current guideport pin state
 */
uint8_t	Qhy2GuidePort::active() {
	uint8_t	result = 0;
	std::chrono::system_clock::time_point	now
		= std::chrono::system_clock::now();
	result |= (now <= until_raplus) ? GuidePort::RAPLUS : 0;
	result |= (now <= until_raminus) ? GuidePort::RAMINUS : 0;
	result |= (now <= until_decplus) ? GuidePort::DECPLUS : 0;
	result |= (now <= until_decminus) ? GuidePort::DECMINUS : 0;
	return result;
}

/**
 * \brief activate a guide port pin for a given time
 *
 * \param until			the time when activity should stop
 * \param duration_time		how long in seconds the activity should last
 * \param direction		the direction code used by the QHYCCD API
 */
void	Qhy2GuidePort::activate(std::chrono::system_clock::time_point& until,
		float duration_time, int direction) {
	std::chrono::system_clock::time_point	now
		= std::chrono::system_clock::now();
	unsigned short	ms = duration_time * 1000;
	until = now + std::chrono::milliseconds(ms);
	int	rc = ControlQHYCCDGuide(camera.handle(), direction, ms);
	if (rc != QHYCCD_SUCCESS) {
		std::string	msg = stringprintf("cannot activate %d "
			"for %hu: %d", 0, ms, rc);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw Qhy2Error(msg, rc);
	}
}

/**
 * \brief Activate all guide port pins
 *
 * \param raplus	activation time for RA+
 * \param raminus	activation time for RA-
 * \param decplus	activation time for DEC+
 * \param decminus	activation time for DEC-
 */
void	Qhy2GuidePort::activate(float raplus, float raminus,
		float decplus, float decminus) {
	activate(until_raplus,   raplus,   0);
	activate(until_raminus,  raminus,  3);
	activate(until_decplus,  decplus,  1);
	activate(until_decminus, decminus, 2);
}

} // namespace qhy2
} // namespace camera
} // namespace astro
