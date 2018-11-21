/*
 * NiceCooler.cpp
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <NiceCooler.h>

namespace astro {
namespace camera {
namespace nice {

NiceCooler::NiceCooler(snowstar::CoolerPrx cooler, const DeviceName& devicename)
	: Cooler(devicename), NiceDevice(devicename), _cooler(cooler)  {
}

NiceCooler::~NiceCooler() {
}

float	NiceCooler::getSetTemperature() {
	return _cooler->getSetTemperature();
}

float	NiceCooler::getActualTemperature() {
	return _cooler->getActualTemperature();
}

void	NiceCooler::setTemperature(float temperature) {
	_cooler->setTemperature(temperature);
}

bool	NiceCooler::isOn() {
	return _cooler->isOn();
}

void	NiceCooler::setOn(bool onoff) {
	_cooler->setOn(onoff);
}

} // namespace nice
} // namespace camera
} // namespace astro
