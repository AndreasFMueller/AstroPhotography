/*
 * NetCooler.cpp -- network connected cooler implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <NetCooler.h>

namespace astro {
namespace camera {
namespace net {

NetCooler::NetCooler(Astro::Cooler_var cooler) : _cooler(cooler) {
	// query the current cooler state from the remote cooler
	Astro::Cooler_Helper::duplicate(_cooler);
}

NetCooler::~NetCooler() {
	Astro::Cooler_Helper::release(_cooler);
}

float	NetCooler::getActualTemperature() {
	return _cooler->getActualTemperature();
}

void	NetCooler::setTemperature(float _temperature) {
	_cooler->setTemperature(_temperature);
}

void	NetCooler::setOn(bool onoff) {
	_cooler->setOn(onoff);
}

bool	NetCooler::isOn() {
	return _cooler->isOn();
}

} // namespace net
} // namespace camera
} // namespace astro
