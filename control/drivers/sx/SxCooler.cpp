/*
 * SxCooler.cpp -- Starlight Express Cooler API implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUSB.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroExceptions.h>
#include "SxCooler.h"

namespace astro {
namespace camera {
namespace sx {

static DeviceName	sx_coolername(const DeviceName& cameraname) {
	DeviceName	ccdname(cameraname, DeviceName::Ccd, "Imaging");
	DeviceName	coolername(ccdname, DeviceName::Cooler, "cooler");
	return coolername;
}

/**
 * \brief Trampoline function to start the run() method in the cooler
 *
 * \param simcooler	the cooler to run this thread for
 */
static void	cooler_main(SxCooler *simcooler) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start %s thread",
		simcooler->name().toString().c_str());
	try {
		simcooler->run();
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s exception in thread: %s",
			demangle_cstr(x), x.what());
	} catch (...) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "unknown exception");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s thread terminates",
		simcooler->name().toString().c_str());
}

/**
 * \brief Create the cooler
 *
 * \param _camera	The camera this cooler belongs to
 */
SxCooler::SxCooler(SxCamera& _camera)
	: Cooler(sx_coolername(_camera.name())), camera(_camera) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create an SX cooler");
	// call the query function to find the current temperature
	// and cooler state
	query(false);

	// if the cooler is on, we cannot really know the set temperature
	// so we just fake it and assume that the actual temperature
	// is also the set temperature, however, if the cooler is off, 
	// we don't know anything.
	_setTemperature = _actualTemperature;

	// to make sure this is true, we should actually set the temperature
	// as we believe the set temperature is
	cmd();

	// start the thread
	_terminate = false;
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	_thread = std::thread(cooler_main, this);
}

/**
 * \brief Destroy the cooler
 */
SxCooler::~SxCooler() {
	// XXX we should turn the cooler off
	{
		std::unique_lock<std::recursive_mutex>	lock(_mutex);
		_terminate = true;
	}
	_cond.notify_all();
	_thread.join();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cooler thread completed");
}

/**
 * \brief Find a good purpose string for reservation
 */
std::string	SxCooler::purpose() const {
	if (std::this_thread::get_id() == _thread.get_id()) {
		return std::string("cooler-thread");
	}
	return std::string("cooler");
}

/**
 * \brief Execute the COOLER command
 */
void	SxCooler::cmd() {
	uint16_t	temp = getSetTemperature().temperature() * 10;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cooler command T = %.1fºC, on = %s",
		getSetTemperature().celsius(), (_on) ? "yes" : "no");
	Request<sx_cooler_temperature_t>	request(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient,
		(uint16_t)((_on) ? 1 : 0),
		(uint8_t)SX_CMD_COOLER, temp);
	try {
		if (camera.reserve(purpose(), 100)) {
			camera.controlRequest(&request);
		} else {
			debug(LOG_WARNING, DEBUG_LOG, 0,
				"Warning: cannot set cooler, camera reserved");
			return;
		}
	} catch (USBError& x) {
		camera.release(purpose());
		std::string	msg = stringprintf("%s usb error: %s",
					name().toString().c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw DeviceTimeout(msg);
	}
	camera.release(purpose());
	Temperature	actual = Temperature(request.data()->temperature / 10.);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "actual temperature = %.1fºC",
		_actualTemperature.celsius());
	bool		on = (request.data()->status) ? true : false;
	if ((on != _on) || (actual != _actualTemperature)) {
		_actualTemperature = actual;
		_on = on;
		callback(CoolerInfo(_actualTemperature, _setTemperature, _on));
	}
}

/**
 * \brief Query the state of the cooler, using the COOLER_TEMPERATURE command
 */
void	SxCooler::query(bool sendcallback) {
	uint16_t	temp = 0;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cooler query");
	Request<sx_cooler_temperature_t>	request(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient,
		(uint16_t)(0),
		(uint8_t)SX_CMD_COOLER_TEMPERATURE, temp);
	try {
		if (camera.reserve(purpose(), 100)) {
			camera.controlRequest(&request);
		} else {
			debug(LOG_WARNING, DEBUG_LOG, 0, "Warning: cannot "
				"query cooler, camera reserved");
			return;
		}
	} catch (USBError& x) {
		camera.release(purpose());
		std::string	msg = stringprintf("%s usb error: %s",
					name().toString().c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw DeviceTimeout(msg);
	}
	camera.release(purpose());

	// interpret the data received
	Temperature	actual = Temperature(request.data()->temperature / 10.);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "actual temperature = %.1fºC",
		actual.celsius());
	bool		on = (request.data()->status) ? true : false;

	// find out whether anything has changed, in which case we
	// should update the callback
	if ((on != _on) || (actual != _actualTemperature)) {
		_actualTemperature = actual;
		_on = on;
		if (sendcallback) {
			callback(CoolerInfo(_actualTemperature,
				_setTemperature, _on));
		}
	}
}

/**
 *  \brief Get the temperature
 */
Temperature	SxCooler::getActualTemperature() {
	query(true);
	return Cooler::getActualTemperature();
}

/**
 * \brief Set the temperature
 *
 * \param temperature	the temperature to set
 */
void	SxCooler::setTemperature(float temperature) {
	Cooler::setTemperature(temperature);
	cmd();
}

/**
 *  \brief Query whether the cooler is on
 */
bool	SxCooler::isOn() {
	query(true);
	return Cooler::isOn();
}

/**
 * \brief Turn the cooler on
 * 
 *  \param onoff	whether or not to turn the cooler on or off
 */
void	SxCooler::setOn(bool onoff) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "turning cooler %s",
		(onoff) ? "on" : "off");
	Cooler::setOn(onoff);
	cmd();
}

/**
 * \brief Main thread for 
 */
void	SxCooler::run() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "run() starts");
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	do {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "new repeat");
		// query temperature
		try {
			query(true);
		} catch (DeviceTimeout& x) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "query failed: %s",
				x.what());
			camera.refresh();
		}
		
		debug(LOG_DEBUG, DEBUG_LOG, 0, "query complete");

		// wait until something happens or at most 3 seconds
		switch (_cond.wait_for(lock, std::chrono::milliseconds(3000))) {
		case std::cv_status::timeout:
			debug(LOG_DEBUG, DEBUG_LOG, 0, "cond timeout");
			break;
		case std::cv_status::no_timeout:
			debug(LOG_DEBUG, DEBUG_LOG, 0, "no cond timeout");
			break;
		}

	} while (!_terminate);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "run() terminates");
}

} // namespace sx
} // namespace camera
} // namespace astro
