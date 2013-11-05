/*
 * NetCooler.cpp -- network connected cooler implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <NetCooler.h>
#include <AstroUtils.h>
#include <NetUtils.h>

namespace astro {
namespace camera {
namespace net {

DeviceName	coolername(const std::string& name) {
	DeviceName	devname("net", URL::encode(name));
	devname.type(DeviceName::Cooler);
	return devname;
}

/**
 * \brief Create a new NetCooler
 *
 * The constructor keeps a reference to a remote cooler object
 */
NetCooler::NetCooler(Astro::Cooler_var cooler)
	: Cooler(devname2netname(cooler->getName())),
	  _cooler(cooler) {
	// query the current cooler state from the remote cooler
	Astro::Cooler_Helper::duplicate(_cooler);
}

/**
 * \brief Destroy the NetCooler
 *
 * This method releases the cooler reference
 */
NetCooler::~NetCooler() {
	Astro::Cooler_Helper::release(_cooler);
}

/**
 * \brief Get the actual temperature
 */
float	NetCooler::getActualTemperature() {
	return _cooler->getActualTemperature();
}

/**
 * \brief Set the cooler's set temperature
 */
void	NetCooler::setTemperature(float _temperature) {
	_cooler->setTemperature(_temperature);
}

/**
 * \brief Turn cooler on or off
 */
void	NetCooler::setOn(bool onoff) {
	_cooler->setOn(onoff);
}

/**
 * \brief Find out whether the cooler is on or off
 */
bool	NetCooler::isOn() {
	return _cooler->isOn();
}

} // namespace net
} // namespace camera
} // namespace astro
