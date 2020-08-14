/*
 * SimMount.cpp -- simulated mount implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SimMount.h>

using namespace astro::device;

namespace astro {
namespace camera {
namespace simulator {

/**
 * \brief Trampoline function to start the mount thread
 */
static void	mount_main(SimMount *_mount) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start the mount thread");
	try {
		_mount->move();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "mount thread complete");
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "mount thread failed: %s",
			x.what());
	}
}

/**
 * \brief Construct a simulated mount
 *
 * \param locator	common simulated locator
 */
SimMount::SimMount(/* SimLocator& locator */) 
	 : Mount(DeviceName("mount:simulator/mount")) /*, _locator(locator)*/ {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "constructing simulated mount");
	_when = 0;
	_direction = _target;
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "location: %s",
			location().toString().c_str());
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"ERROR: must define location for simulated mount in %s",
			DEVICEPROPERTIES);
	}
}

/**
 * \brief Destroy the simulator mount
 */
SimMount::~SimMount() {
	// wait for the thread to terminate
	std::unique_lock<std::recursive_mutex>	lock(_sim_mutex);
	if (Mount::GOTO == Mount::state()) {
		Mount::state(Mount::TRACKING);
		_sim_condition.notify_all();
		// wait for the thread to terminate
		if (_sim_thread.joinable()) {
			_sim_thread.join();
		}
	}
}

const double SimMount::_movetime = 10;

/**
 * \brief Compute the direction the mount is currently pointing
 */
RaDec	SimMount::direction() {
	std::unique_lock<std::recursive_mutex>	lock(_sim_mutex);
	return _direction;
}

/**
 * \brief Set the direction
 *
 * \param d	RaDec structure to use as direction
 */
void	SimMount::direction(const RaDec& d) {
	std::unique_lock<std::recursive_mutex>	lock(_sim_mutex);
	_direction = d;
	callback(_direction);
}

/**
 * \brief Get the direction into which the mount is pointing
 */
RaDec	SimMount::getRaDec() {
	return direction();
}

/**
 * \brief Get the azimuth and altitude 
 *
 * This method always throws an exception to indicate that the simulated
 * mount does not know about azimuth and altitude
 */
AzmAlt	SimMount::getAzmAlt() {
	debug(LOG_ERR, DEBUG_LOG, 0, "cannot get AzmAlt");
	throw std::runtime_error("cannot get AzmAlt");
}

/**
 * \brief Move to a new position in right ascension and declination
 *
 * We should make this dynamic so that the simulated mount takes some
 * time to move to the new position
 *
 * \param radec		new coordinates
 */
void	SimMount::Goto(const RaDec& radec) {
	std::unique_lock<std::recursive_mutex>	lock(_sim_mutex);
	//_direction = direction();
	// whatever we find out below, we certainly want to set the
	// target and the arrival time according to the new data
	_when = Timer::gettime() + _movetime;
	_target = radec;
	// first find out whether we alread have a thread. This is the
	// case if the state is GOTO. In this case redirecting was good
	// enough
	if (Mount::GOTO == Mount::state()) {
		return;
	}

	// set the state to GOTO, this ensures that the thread will
	// start running
	Mount::state(Mount::GOTO);
	_sim_condition.notify_all();

	// here we should also start a thread that will periodically send
	// RaDec updates to the callback and reset the state at the
	// end of the move
	_sim_thread = std::thread(mount_main, this);
}

/**
 * \brief Move to a new position in azimuth and altitude
 *
 * This method always throws an exception to indicate that the simulated
 * mount does not understand altitude and azimuth
 */
void	SimMount::Goto(const AzmAlt& /* azmalt */) {
	debug(LOG_ERR, DEBUG_LOG, 0, "cannot get AzmAlt");
	throw std::runtime_error("cannot goto AzmAlt");
}

/**
 * \brief Cancel movement
 *
 * This method is not implemented
 */
void	SimMount::cancel() {
	std::unique_lock<std::recursive_mutex>	lock(_sim_mutex);
	_when = 0;
	_target = _direction;
	// Just reset the stsate to TRACKING. The thread will notice
	// that it is no longer in GOTO mode and will terminate
	Mount::state(Mount::TRACKING);
}

/**
 * \brief Find out whether the mount can provide guide rates
 */
bool	SimMount::hasGuideRates() {
	return true;
}

/**
 * \brief Get the guide rates
 */
RaDec	SimMount::getGuideRates() {
	double	rate = 0.5;
	double	frequency = 1/86400.;
	Angle	guiderate = rate * frequency * 4 * Angle::right_angle;
	return RaDec(guiderate, guiderate);
}

/**
 * \brief Do the actual movement
 */
void	SimMount::move() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Simulator move thread starts %s",
		_target.toString().c_str());
	while (true) {
		Timer::sleep(1);
		std::unique_lock<std::recursive_mutex>	lock(_sim_mutex);
		if (Mount::GOTO == Mount::state()) {
			double	_now = Timer::gettime();
			if (_now > _when) {
				direction(_target);
				Mount::state(Mount::TRACKING);
				return;
			} else {
				double	t = (_when - _now) / _movetime;
				direction(_target * (1 - t) + _direction * t);
			}
		} else {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "thread cancelled");
			return;
		}
	}
}

} // namespace simulator
} // namespace camera
} // namespace astro
