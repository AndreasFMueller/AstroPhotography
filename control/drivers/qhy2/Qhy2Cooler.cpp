/*
 * Qhy2Cooler.cpp -- implementation of the QHY cooler class
 *
 * (c) 2020 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Qhy2Cooler.h>
#include <Qhy2Utils.h>
#include <qhyccd.h>
#include <AstroDebug.h>

namespace astro {
namespace camera {
namespace qhy2 {

/**
 * \brief Start the thread
 *
 * \param cooler	the cooler to control
 */
void	Qhy2Cooler::start_thread(Qhy2Cooler *cooler) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cooler thread launch");
	try {
		cooler->run();
	} catch (const std::exception x) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"exception thrown in cooler thread: %s", x.what());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cooler thread returns");
}

/**
 * \brief Main method of the thread
 */
void	Qhy2Cooler::run() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "the cooler thread for %s starts",
		name().toString().c_str());
	Cooler::setTemperature(Temperature(15, Temperature::CELSIUS));
	setOn(true);

	// get the current temperature
	Temperature	previoustemperature = getActualTemperature();

	// protect common data
	std::unique_lock<std::mutex>	lock(_mutex);
	while (_running) {
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "cooler loop");
		// handle the temperature control call
		if (isOn()) {
			double	celsiustemp = getSetTemperature().celsius();
			int	rc = ControlQHYCCDTemp(camera.handle(),
					celsiustemp);
			if (rc != QHYCCD_SUCCESS) {
				debug(LOG_ERR, DEBUG_LOG, 0, "cannot control "
					"the temperature %.1f (rc=%d)",
					 celsiustemp, rc);
			} else {
				//debug(LOG_DEBUG, DEBUG_LOG, 0,
				//	"ON: temperature: %.1f", celsiustemp);
			}
		} else {
			// control temperature with an extremely high 
			// temperature, which 
			int	rc = ControlQHYCCDTemp(camera.handle(), 30.);
			if (rc != QHYCCD_SUCCESS) {
				debug(LOG_ERR, DEBUG_LOG, 0, "OFF: cannot NOT "
					"control temperature: %d", rc);
			}
			//debug(LOG_DEBUG, DEBUG_LOG, 0, "not controlling");
		}

		// wait for a second or shorter if something happens
		if (std::cv_status::timeout != _cond.wait_for(lock,
			std::chrono::seconds(1))) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "cooler state change: "
				"on=%s, temperature=%%.1fC",
				(isOn()) ? "on" : "off",
				getSetTemperature().celsius());
		}

		// retrieve the actual temperature
		Temperature	a = getActualTemperature();
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "actual: %.1f, previous: %.1f",
		//	a.celsius(), previoustemperature.celsius());

		// if the temperature has changed enough, we send a new
		// coolerinfo to the callback
		if (fabs(a - previoustemperature) > 0.1) {
			previoustemperature = a;
			CoolerInfo	coolerinfo(a, _setTemperature, isOn());
			debug(LOG_DEBUG, DEBUG_LOG, 0, "sending callback(%s)",
				coolerinfo.toString().c_str());
			callback(coolerinfo);
		}
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "Cooler %s thread terminates",
		name().toString().c_str());
}

/**
 * \brief Create the cooler
 *
 * \param _camera	the camera of which we control the cooler
 */
Qhy2Cooler::Qhy2Cooler(Qhy2Camera& _camera)
	: Cooler(Qhy2Name(_camera.qhyname()).coolername()), camera(_camera) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create a QHY cooler");
	std::unique_lock<std::mutex>	lock(_mutex);
	_running = true;
	_thread = std::thread(start_thread, this);
}

/**
 * \brief Destroy the cooler object
 */
Qhy2Cooler::~Qhy2Cooler() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stopping the Qhy2Cooler thread");
	{ // scope may not include setOn because that would block setOn
	  // from completing
		std::unique_lock<std::mutex>	lock(_mutex);
		_running = false;
	}
	setOn(false);
	// wait for the thread to complete
	if (_thread.joinable()) {
		_thread.join();
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cooler destructor complete");
}

/**
 * \brief Get the actual temperature of the cooler
 */
Temperature	Qhy2Cooler::getActualTemperature() {
	double	tmp = GetQHYCCDParam(camera.handle(), CONTROL_CURTEMP);
	return _actualTemperature = Temperature(tmp, Temperature::CELSIUS);
}

/**
 * \brief Turn the cooler on/off
 *
 * \param onnotoff	whether the cooler is to be turned or off
 */
void	Qhy2Cooler::setOn(bool onnotoff) {
	std::unique_lock<std::mutex>	lock(_mutex);
	_on = onnotoff;
	_cond.notify_all();
}

/**
 * \brief Set a new 
 */
void	Qhy2Cooler::setTemperature(const float _temperature) {
	Cooler::setTemperature(_temperature);
	_cond.notify_all();
}

} // namespace qhy2
} // namespace camera
} // namespace astro

