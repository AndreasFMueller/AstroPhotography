/*
 * CoolerCallbackI.cpp
 *
 * (c) 2020 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */

#include <coolercontrollerwidget.h>

namespace snowgui {

void	CoolerCallbackI::updateCoolerInfo(const snowstar::CoolerInfo& info,
			const Ice::Current& /* current */) {
	_coolercontrollerwidget.statusUpdate();
}

void	CoolerCallbackI::updateSetTemperature(float settemperature,
			const Ice::Current& /* current */) {
	astro::Temperature	temperature(settemperature);
	_coolercontrollerwidget.displaySetTemperature(temperature.celsius());
}

void	CoolerCallbackI::updateDewHeater(float dewheater,
			const Ice::Current& /* current */) {
	_coolercontrollerwidget.setDewHeaterSlider(dewheater);
}

} // namespace snowgui

