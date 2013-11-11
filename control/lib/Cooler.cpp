/*
 * Cooler.cpp -- cooler implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>
#include <stdexcept>
#include <AstroFormat.h>

using namespace astro::image;
using namespace astro::device;

namespace astro {
namespace camera {

DeviceName::device_type	Cooler::devicetype = DeviceName::Cooler;

DeviceName	Cooler::defaultname(const DeviceName& parent,
			const std::string& unitname) {
	return DeviceName(parent, DeviceName::Cooler, unitname);
}

Cooler::Cooler(const DeviceName& name) : Device(name) {
	temperature = 25 + 273.1;
}

Cooler::Cooler(const std::string& name) : Device(name) {
	temperature = 25 + 273.1;
}

Cooler::~Cooler() {
}

float	Cooler::getSetTemperature() {
	return temperature;
}

float	Cooler::getActualTemperature() {
	throw std::runtime_error("cannot measure temperature");
}

void	Cooler::setTemperature(float _temperature) {
	if (_temperature < 0) {
		throw std::range_error("negative absolute temperature");
	}
	if (_temperature > 350) {
		throw std::range_error("temperature too large: heater?");
	}
	temperature = _temperature;
}

void	Cooler::setOn(bool onoff) {
}

bool	Cooler::isOn() {
	return true;
}

/**
 * \brief Add temperature metadata to an image
 */
void	Cooler::addTemperatureMetadata(ImageBase& image) {
	if (!isOn()) {
		return;
	}

	// set temperature
	Metavalue	mvsettemp(
		getSetTemperature() - 273.1,
		std::string("CCD temperature setpoint in "
			"degrees C"));
	image.setMetadata(std::string("SET-TEMP"), mvsettemp);
	
	// actual temperature
	try {
		Metavalue	mvtemp(getActualTemperature() - 273.1,
			std::string("actual measured sensor temperature "
				"at the start of exposure in degrees C"));
		image.setMetadata(std::string("CCD-TEMP"), mvtemp);
	} catch (std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "actual temperature unknown: %s",
			x.what());
	}
}

} // namespace camera
} // namespace astro
