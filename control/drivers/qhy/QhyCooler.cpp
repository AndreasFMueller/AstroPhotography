/*
 * QhyCooler.cpp -- implementation of the QHY cooler class
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <QhyCooler.h>
#include <QhyUtils.h>
#include <qhylib.h>
#include <AstroDebug.h>

namespace astro {
namespace camera {
namespace qhy {

/**
 * \brief Compute the name of the cooler
 */
static DeviceName	qhy_coolername(const DeviceName& cameraname) {
	QhyName	qhyname(cameraname);
	return qhyname.coolername();
}

/**
 * \brief Create the cooler
 */
QhyCooler::QhyCooler(QhyCamera& _camera, ::qhy::DevicePtr devptr)
	: Cooler(qhy_coolername(_camera.name())), deviceptr(devptr) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create a QHY cooler");
}

/**
 * \brief Destroy the cooler object
 */
QhyCooler::~QhyCooler() {
	setOn(false);
}

/**
 * \brief Get the actual temperature of the cooler
 */
float	QhyCooler::getActualTemperature() {
	return deviceptr->dc201().temperature();
}

/**
 * \brief Set the temperature of the cooler
 *
 * \param temperature	temperature set for the cooler
 */
void	QhyCooler::setTemperature(float temperature) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting the temperature to %.1f",
		temperature);
	deviceptr->dc201().settemperature(temperature);
}

/**
 * \brief Test whether the cooler is on
 */
bool	QhyCooler::isOn() {
	return deviceptr->dc201().cooler();
}

/**
 * \brief Turn the cooler on/off
 *
 * \param onnotoff	whether the cooler is to be turned or off
 */
void	QhyCooler::setOn(bool onnotoff) {
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
}

} // namespace qhy
} // namespace camera
} // namespace astro

