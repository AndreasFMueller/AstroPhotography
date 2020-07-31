/*
 * CoolerCallbackI.cpp
 *
 * (c) 2020 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */

#include <coolercontrollerwidget.h>

namespace snowgui {

/**
 * \brief Construct a new cooler callback
 */
CoolerCallbackI::CoolerCallbackI(coolercontrollerwidget& c)
	: _coolercontrollerwidget(c) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "coolercallback constructed");
}

/**
 * \brief Destroy the cooler
 */
CoolerCallbackI::~CoolerCallbackI() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "coolercallback being destroyed");
}

/**
 * \brief the callback method for cooler info
 *
 * \param info		the updated info
 * \param current	the current call context
 */
void	CoolerCallbackI::updateCoolerInfo(const snowstar::CoolerInfo& /* info */,
			const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cooler info update received");
	_coolercontrollerwidget.statusUpdate();
}

/**
 * \brief the callback method for the set temperature
 *
 * \param settemperature	the updated set temperature
 * \param current		the current call context
 */
void	CoolerCallbackI::updateSetTemperature(float settemperature,
			const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cooler set temperature update received");
	astro::Temperature	temperature(settemperature);
	_coolercontrollerwidget.displaySetTemperature(temperature.celsius());
}

/**
 * \brief the callback method for deawheater state
 *
 * \param dewheater	dewheating value
 * \param current	the current call context
 */
void	CoolerCallbackI::updateDewHeater(float dewheater,
			const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cooler dewheater update received");
	_coolercontrollerwidget.setDewHeaterSlider(dewheater);
}

} // namespace snowgui

