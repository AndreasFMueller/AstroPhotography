/*
 * Cooler.cpp -- cooler implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>
#include <stdexcept>
#include <AstroFormat.h>
#include <unistd.h>
#include <AstroDebug.h>
#include <AstroIO.h>
#include <AstroConfig.h>

using namespace astro::image;
using namespace astro::io;
using namespace astro::device;

namespace astro {
namespace camera {

config::ConfigurationKey	_cooler_stable_key(
	"device", "cooler", "stable");
config::ConfigurationRegister	_cooler_stable_registration(
	_cooler_stable_key,
	"tolerance in degrees K for the temperature to consider the cooler stable");

DeviceName::device_type	Cooler::devicetype = DeviceName::Cooler;

/**
 * \brief auxiliary function to produce a default 
 */
DeviceName	Cooler::defaultname(const DeviceName& parent,
			const std::string& unitname) {
	return DeviceName(parent, DeviceName::Cooler, unitname);
}

/**
 * \brief Create a cooler from the name
 */
Cooler::Cooler(const DeviceName& name) : Device(name, DeviceName::Cooler),
	_actualTemperature(25, Temperature::CELSIUS),
	_setTemperature(25, Temperature::CELSIUS), _on(false) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create cooler named %s",
		Device::name().name().c_str());
}

/**
 * \brief Create a cooler from the unit name
 */
Cooler::Cooler(const std::string& name) : Device(name, DeviceName::Cooler),
	_actualTemperature(25, Temperature::CELSIUS),
	_setTemperature(25, Temperature::CELSIUS), _on(false) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create cooler named %s",
		Device::name().name().c_str());
}

/**
 * \brief Destroy the cooler
 */
Cooler::~Cooler() {
}

/**
 * \brief Get the current set temperature
 */
Temperature	Cooler::getSetTemperature() {
	return _setTemperature;
}

/**
 * \brief Retrieve the actual temperature
 *
 * Not all coolers can report the actual temperature. This method must
 * be overridden by concrete Cooler implementations.
 */
Temperature	Cooler::getActualTemperature() {
	return _actualTemperature;
}

/**
 * \brief Set the set temperature
 *
 * Temperature must be absolute, so temperatures below 0 are rejected as
 * well as temperatures above 350K, as they correspond to heaters rather
 * than coolers.
 *
 * \param _temperature	new set temperature
 */
void	Cooler::setTemperature(float _temperature) {
	if (_temperature < 0) {
		throw std::range_error("negative absolute temperature");
	}
	if (_temperature > 350) {
		throw std::range_error("temperature too large: heater?");
	}
	_setTemperature = _temperature;
}

/**
 * \brief Set the temperature
 *
 * This is the public interface which ensures that the status update
 * callback is called
 */
void	Cooler::setTemperature(const Temperature& temperature) {
	// send the new temperature to the callback
	callback(temperature);
	setTemperature(temperature.temperature());
}

/**
 * \brief Turn the cooler on/off
 *
 * This is an empty implementation that must be overridden by driver classes.
 * Driver classes still need to call this method at the end of their
 * implementation to ensure that the callback is sent.
 *
 *  \param onoff	Turn the cooler on/off
 */
void	Cooler::setOn(bool onoff) {
	CoolerInfo	ci(getActualTemperature(), getSetTemperature(), onoff);
	_on = onoff;
	callback(ci);
}

/**
 * \brief Whether or not the cooler is in
 *
 * This is a default implementation, must be overridden by driver classes.
 */
bool	Cooler::isOn() {
	return _on;
}

/**
 * \brief Add temperature metadata to an image
 */
void	Cooler::addTemperatureMetadata(ImageBase& image) {
	// if the cooler is not on, then there is nothing to add to the image
	if (isOn()) {
		// add set temperature information to the image (SET-TEMP)
		image.setMetadata(
			FITSKeywords::meta(std::string("SET-TEMP"),
				this->getSetTemperature().celsius()));
	}
	
	// add actual temperature info to the image (CCD-TEMP)
	try {
		image.setMetadata(
			FITSKeywords::meta(std::string("CCD-TEMP"),
				getActualTemperature().celsius()));
	} catch (std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "actual temperature unknown: %s",
			x.what());
	}
}

/**
 * \brief Find out whether the cooler has cooled to a stable temperature
 */
bool	Cooler::stable() {
	if (!isOn()) {
		return true;
	}
	config::ConfigurationPtr	config = config::Configuration::get();
	float	stablelimit = 3.;
	if (config->has(_cooler_stable_key)) {
		stablelimit = std::stof(config->get(_cooler_stable_key));
		if (stablelimit <= 0) {
			stablelimit = 3.;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "stable limit config: %.1f",
			stablelimit);
	}
	float	actualtemperature = this->getActualTemperature().temperature();
	float	delta = fabs(actualtemperature - _setTemperature.temperature());
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"T_act = %.1f, T_set = %.1f, delta = %.1f, limit = %.1f",
		actualtemperature, _setTemperature.temperature(),
		delta, stablelimit);
	return (delta < stablelimit);
}

/**
 * \brief Wait for the cooler to cool down
 */
bool	Cooler::wait(float timeout) {
	while ((timeout > 0) && (!stable())) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "waiting for cooler");
		usleep(1000000);
		timeout -= 1;
	}
	return (timeout < 0) ? false : true;
}

/**
 * \brief Does this cooler have a dew heater?
 */
bool	Cooler::hasDewHeater() {
	return false;
}

/**
 * \brief Retrieve the range for the dew heater
 */
std::pair<float, float>	Cooler::dewHeaterRange() {
	return std::make_pair((float)0., (float)1.);
}

/**
 * \brief Retreive the current dew heater value
 */
float	Cooler::dewHeater() {
	return 0.;
}

/**
 * \brief Set the dew heater value
 *
 * Just ignores the input
 *
 * \param d	the dew heater value
 */
void	Cooler::dewHeater(float d) {
	callback(DewHeater(d));
}

/**
 * \brief CoolerInfo callback
 *
 * \param info	the cooler info
 */
void	Cooler::callback(const CoolerInfo& info) {
	callback::CallbackDataPtr	cb(new CoolerInfoCallbackData(info));
	_callback(cb);
}

/**
 * \brief Dew heater callback
 *
 * \param dewheater	the dewheater data to report
 */
void	Cooler::callback(const DewHeater& dewheater) {
	callback::CallbackDataPtr	cb(new DewHeaterCallbackData(
						dewheater));
	_callback(cb);
}

/**
 * \brief Set temperature change callback
 *
 * param newSetTemperature	the new set temperature
 */
void	Cooler::callback(const Temperature& newSetTemperature) {
	callback::CallbackDataPtr	cb(new SetTemperatureCallbackData(
						newSetTemperature));
	_callback(cb);
}

/**
 * \brief Register a new callback
 *
 * \param callback	the callback to install
 */
void	Cooler::addCallback(callback::CallbackPtr callback) {
	_callback.insert(callback);
}

/**
 * \brief Remove a new callback
 *
 * \param callback	the callback to install
 */
void	Cooler::removeCallback(callback::CallbackPtr callback) {
	auto	i = _callback.find(callback);
	if (i != _callback.end()) {
		_callback.erase(i);
	}
}


} // namespace camera
} // namespace astro
