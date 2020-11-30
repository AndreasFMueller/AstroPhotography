/*
 * Qhy2Cooler.cpp -- implementation of the QHY cooler class
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Qhy2Cooler.h>
#include <Qhy2Utils.h>
#include <qhyccd.h>
#include <AstroDebug.h>

namespace astro {
namespace camera {
namespace qhy2 {

/**
 * \brief Compute the name of the cooler
 */
static DeviceName	qhy_coolername(const DeviceName& cameraname) {
	Qhy2Name	qhyname(cameraname);
	return qhyname.coolername();
}

/**
 * \brief Create the cooler
 */
Qhy2Cooler::Qhy2Cooler(Qhy2Camera& _camera)
	: Cooler(qhy_coolername(_camera.name())) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create a QHY cooler");
}

/**
 * \brief Destroy the cooler object
 */
Qhy2Cooler::~Qhy2Cooler() {
	setOn(false);
}

/**
 * \brief Get the actual temperature of the cooler
 */
Temperature	Qhy2Cooler::getActualTemperature() {
#if 0
	return Temperature(deviceptr->dc201().temperature());
#endif
}

/**
 * \brief Set the temperature of the cooler
 *
 * \param temperature	temperature set for the cooler
 */
void	Qhy2Cooler::setTemperature(float temperature) {
#if 0
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting the temperature to %.1f",
		temperature);
	deviceptr->dc201().settemperature(temperature);
#endif
}

/**
 * \brief Test whether the cooler is on
 */
bool	Qhy2Cooler::isOn() {
#if 0
	return deviceptr->dc201().cooler();
#endif
}

/**
 * \brief Turn the cooler on/off
 *
 * \param onnotoff	whether the cooler is to be turned or off
 */
void	Qhy2Cooler::setOn(bool onnotoff) {
#if 0
	debug(LOG_DEBUG, DEBUG_LOG, 0, "turing the cooler %s",
		(onnotoff) ? "on" : "off");
	if (onnotoff) {
		if (isOn()) {
			return;
		}
		deviceptr->dc201().startCooler();
	} else {
		if (!isOn()) {
			return;
		}
		deviceptr->dc201().stopCooler();
	}
#endif
}

} // namespace qhy2
} // namespace camera
} // namespace astro

