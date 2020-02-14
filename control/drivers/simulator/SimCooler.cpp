/*
 * SimCooler.cpp -- Cooler Simulator
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SimCooler.h>
#include <SimUtil.h>
#include <includes.h>
#include <AstroDebug.h>

using namespace astro::device;

namespace astro {
namespace camera {
namespace simulator {

const static Temperature	ambient_temperature(Temperature::zero + 13.2);

/**
 * \brief Trampoline function to launch the cooler thread
 *
 * \param simcooler 	the cooler to run for
 */
static void	cooler_main(SimCooler *simcooler) {
	try {
		simcooler->run();
	} catch (...) {
	}
}

/**
 *  \brief Construct a new cooler object
 */
SimCooler::SimCooler(SimLocator& locator)
	: Cooler(DeviceName("cooler:simulator/cooler")), _locator(locator) {
	_setTemperature = ambient_temperature;
	_actualTemperature = ambient_temperature;
	lasttemperature = 0.;	// this temperature causes the thread to
				// send an update to the callback in the
				// first loop
	laststatechange = simtime();
	on = false;
	_dewheatervalue = 0.;
	// start the thread to update the temperature at regular intervals
	_terminate = false;
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	_thread = std::thread(cooler_main, this);
}

/**
 * \brief Destroy the simulated cooler
 *
 * The destructor must notify the thread to terminate and waits for
 * the thread to 
 */
SimCooler::~SimCooler() {
	{
		std::unique_lock<std::recursive_mutex>	lock(_mutex);
		_terminate = true;
	}
	_cond.notify_all();
	_thread.join();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cooler thread comleted");
}

/**
 * \brief Run function for the monitoring thread
 */
void	SimCooler::run() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "run() starts");
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	do {
		// make sure the temperature is current
		updateTemperature();

		// wait until something happens
		_cond.wait_for(lock, std::chrono::milliseconds(2000));
	} while (!_terminate);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "run() terminates");
}

/**
 * \brief Send information about the cooler to the callback
 */
void	SimCooler::sendInfo() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sending info update");
	std::unique_lock<std::recursive_mutex>	lock(_mutex);

	// create the callback data
	CoolerInfo	ci(_actualTemperature, getSetTemperature(), on);
	callback::CallbackDataPtr	cid(new CoolerInfoCallbackData(ci));
	callback(ci);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "info update sent");

	// set the last report variables
	lasttemperature = _actualTemperature;
	laststatechange = simtime();
}

/**
 * \brief Update the temperature
 *
 * This method takes the time since the last reported change
 */
void	SimCooler::updateTemperature() {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "update temperature");

	// handle the special case of 0 temperature
	if (lasttemperature == 0) {
		sendInfo();
	}

	// compute time since last info callback and temperature difference
	double	timepast = simtime() - laststatechange;
	Temperature	targettemperature
				= (on) ? _setTemperature : ambient_temperature;
	float	delta = targettemperature - lasttemperature;

	// linearly change the temperature. Because we often update
	// the last temperature, the temperature changes become slower
	// with time
	float	actemp = (timepast / 6) * delta + lasttemperature.temperature();

	// add some random error on the order of about 1 degree
	actemp += ((random() % 200) - 100) / 100.;

	// this is the new temperature that we register i any case
	_actualTemperature = actemp;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "updated temperature %.1fºC",
		_actualTemperature.celsius());

	// now we have to decide whether we should actually send an
	// update to the callbacks. We do this if the temperature
	// difference is large enough or the time difference is more
	// than five seconds
	float	temperaturedifference
			= fabs(_actualTemperature - lasttemperature);
	if ((timepast > 5) || (temperaturedifference > 1))  {
		sendInfo();
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "udpate complete");
}

/**
 * \brief Get the actual temperature
 */
Temperature	SimCooler::getActualTemperature() {
	return _actualTemperature;
}

/**
 * \brief Set the set temperature
 *
 * \param _temperature	absolute temperature for the cooler
 */
void	SimCooler::setTemperature(float _temperature) {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	_setTemperature = _temperature;
	// signal the thread that something has happened
	_cond.notify_all();
}

/**
 * \brief Turn on or off the cooler
 *
 * \param onoff		whether or not to turn the heater on
 */
void	SimCooler::setOn(bool onoff) {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	if (onoff == on) {
		// ignore if there is no actual change
		return;
	}
	on = onoff;
	// notify the thread. the update function called by the thread
	// will pick up the new variables
	_cond.notify_all();
}

/**
 * \brief Find out whether the cooler is currently below ambient temperature
 */
int	SimCooler::belowambient() {
	int	result = (ambient_temperature - getActualTemperature()) / 7.;
	return result;
}

/**
 * \brief Whether or not the dew heater has
 */
bool	SimCooler::hasDewHeater() {
	return true;
}

/**
 * \brief retrieve the current dew heater value
 */
float	SimCooler::dewHeater() {
	return _dewheatervalue;
}

/**
 * \brief retrieve the current dew heater value
 *
 * \param d	dew heater value to set
 */
void	SimCooler::dewHeater(float d) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new dew heater value: %.2f", d);
	_dewheatervalue = d;
	Cooler::dewHeater(d);	// sends callback info
}

/**
 * \brief Retrieve the range of valid dew heater values
 */
std::pair<float, float>	SimCooler::dewHeaterRange() {
	return std::make_pair((float)0., (float)1.);
}

} // namespace simulator
} // namespace camera
} // namespace atro
