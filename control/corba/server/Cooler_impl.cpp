/*
 * Cooler_impl.cpp -- CORBA Cooler wrapper implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Cooler_impl.h>

namespace Astro {

char	*Cooler_impl::getName() {
	std::string	name = _cooler->name();
	return CORBA::string_dup(name.c_str());
}

CORBA::Float	Cooler_impl::getSetTemperature() {
	return _cooler->getSetTemperature();
}

CORBA::Float	Cooler_impl::getActualTemperature() {
	return _cooler->getActualTemperature();
}

void	Cooler_impl::setTemperature(CORBA::Float temperature) {
	_cooler->setTemperature(temperature);
}

bool	Cooler_impl::isOn() {
	return _cooler->isOn();
}

void	Cooler_impl::setOn(bool onoff) {
	_cooler->setOn(onoff);
}

} // namespace astro

