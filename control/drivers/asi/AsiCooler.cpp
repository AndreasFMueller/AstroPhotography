/*
 * AsiCooler.cpp -- implementation of a cooler class for ASI cameras
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AsiCooler.h>
#include <AstroDebug.h>

namespace astro {
namespace camera {
namespace asi {

/**
 * \brief Trampoline function to start the cooler thread
 *
 * \param cooler	the cooler to run the thread for
 */
static void	asi_cooler_main(AsiCooler *cooler) {
	std::string	name = cooler->name().toString();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s thread starts", name.c_str());
	try {
		cooler->run();
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "%s failed: %s", name.c_str(),
			x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "%s failed: (unknown)",
			name.c_str());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s thread terminates", name.c_str());
}

/**
 * \brief auxiliary function to compute name of a cooler
 *
 * \param camera	the camera for which the cooler name should be found
 */
static DeviceName	asiCoolerName(AsiCamera& camera) {
	DeviceName	coolername = camera.name();
	coolername.type(DeviceName::Cooler);
	return coolername;
}

/**
 * \brief Construct a new cooler
 *
 * \param camera	the camera to use to talk to the cooler device
 */
AsiCooler::AsiCooler(AsiCamera& camera)
	: Cooler(asiCoolerName(camera)), _camera(camera) {
	float	t = getActualTemperature().temperature();
	_camera.settemperature(t);
	_running = true;
	_thread = std::thread(asi_cooler_main, this);
}

/**
 * \brief Destroy a cooler
 *
 * The destructor must ensure that the cooler is turned off
 */
AsiCooler::~AsiCooler() {
	try {
		setOn(false);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot turn off: %s", x.what());
	}
	stop();
}

/**
 * \brief Get the set temperature
 */
Temperature	AsiCooler::getSetTemperature() {
	return Temperature(_camera.settemperature());
}

/**
 * \brief Get the current temperature
 */
Temperature	AsiCooler::getActualTemperature() {
	return Temperature(_camera.getControlValue(AsiTemperature).value / 10.);
}

/**
 * \brief Set the temperature in the camera
 */
void	AsiCooler::setCoolerTemperature() {
	AsiControlValue	value;
	value.type = AsiTargetTemp;
	// must not be multiplied by 10!
	value.value = _camera.settemperature() - Temperature::zero;
	value.isauto = false;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting temperature to %.1f -> %d",
		_camera.settemperature(), value.value);
	_camera.setControlValue(value);
}

/**
 * \brief Set the target temperature of the cooler
 *
 * \param temperature	the target temperature to set
 */
void	AsiCooler::setTemperature(float temperature) {
	// tell the parent class what the new set temperature is (this also
	// triggers the set temperature callback
	Cooler::setTemperature(temperature);

	// set the temperature on the device
	_camera.settemperature(temperature);
	setCoolerTemperature();
}

/**
 * \brief Find out whether the cooler is on or off
 */
bool	AsiCooler::isOn() {
	return (_camera.getControlValue(AsiCoolerOn).value) ? true : false;
}

/**
 * \brief Turn cooler on/off
 *
 * Turning the cooler on also sets the temperature anew, because apparently
 * the camera forgets the set temperature...
 *
 * \param onoff	state of the cooler after this operation
 */
void	AsiCooler::setOn(bool onoff) {
	// turn the heater on
	AsiControlValue	value;
	value.type = AsiCoolerOn;
	value.value = (onoff) ? ASI_TRUE : ASI_FALSE;
	value.isauto = false;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "turning cooler %s",
		onoff ? "on" : "off");
	_camera.setControlValue(value);
	// turn the fan on 
	value.type = AsiFanOn;
	_camera.setControlValue(value);
	// turn anti dew heater on
	value.type = AsiAntiDewHeater;
	_camera.setControlValue(value);
	// must send the set temperature again
	setCoolerTemperature();
}

/**
 * \brief Find out whether there also is a dew heater
 */
bool	AsiCooler::hasDewHeater() {
	try {
		_camera.controlIndex("AntiDewHeater");
		return true;
	} catch (const std::exception& x) {
	}
	return false;
}

/**
 * \brief Get the dew heater control value
 */
float	AsiCooler::dewHeater() {
	return (float)_camera.getControlValue(AsiAntiDewHeater).value;
}

/**
 * \brief Set the dew heater control value
 *
 * \param dewheatervalue	the new dewheatervalue
 */
void	AsiCooler::dewHeater(float dewheatervalue) {
	// set the dew heater control value in the camera
	AsiControlValue controlvalue;
	controlvalue.type = AsiAntiDewHeater;
	controlvalue.value = dewheatervalue;
	controlvalue.isauto = false;
	_camera.setControlValue(controlvalue);

	// trigger the callback informing clients of dew heater state changes
	callback(DewHeater(dewheatervalue));
}

/**
 * \brief Retrieve the range of acceptable dew heater control values
 */
std::pair<float, float>	AsiCooler::dewHeaterRange() {
	if (!hasDewHeater()) {
		throw std::runtime_error("device has no dew heater");
	}
	int	control_index = _camera.controlIndex("AntiDewHeater");
	float   minDewHeater = (float)_camera.controlMin(control_index);
	float   maxDewHeater = (float)_camera.controlMax(control_index);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "dew heater interval: [%.2f, %.2f]",
		minDewHeater, maxDewHeater);
	return std::make_pair(minDewHeater, maxDewHeater);
}

/**
 * \brief The run method for the cooler thread
 *
 * The cooler thread just monitors the cooler of the camera and reports 
 * any observed temperature changes
 */
void	AsiCooler::run() {
	std::unique_lock<std::mutex>	lock(_mutex);
	auto	interval = std::chrono::milliseconds(3000);
	Temperature	_previous = getActualTemperature();
	while (_running) { 
		std::cv_status	status = _condition.wait_for(lock, interval);
		if (status == std::cv_status::timeout) {
			// timeout: get the actual temperature
			Temperature _new = getActualTemperature();
			if (_previous != _new) {
				callback(CoolerInfo(*this));
			}
			_previous = _new;
		}
		// all other cases, just check whether the thread should stop
	}
}

/**
 * \brief Stop the cooler thread
 */
void	AsiCooler::stop() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stop the ccd thread");
	{
		std::unique_lock<std::mutex>	lock(_mutex);
		_running = false;
	}
	_condition.notify_all();
	if (_thread.joinable()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "join the ccd thread");
		_thread.join();
	}
}

} // namespace asi
} // namespace camera
} // namespade astro
