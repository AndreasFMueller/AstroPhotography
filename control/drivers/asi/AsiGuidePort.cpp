/*
 * AsiGuidePort.cpp -- ASI guider port implementation
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AsiGuidePort.h>
#include <AstroDebug.h>
#include <utils.h>

namespace astro {
namespace camera {
namespace asi {

/**
 * \brief trampoline function to start the run function of the class
 */
void	AsiGuidePort::main(AsiGuidePort *port) noexcept {
	std::string	portname = port->name();
	try {
		port->run();
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("guider port %s failed: %s",
			portname.c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	} catch (...) {
		std::string	msg = stringprintf("guideport %s thread "
			"failed (unknown exception", portname.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s thread terminates",
		portname.c_str());
}

/**
 * \brief Create a new AsiGuidePort
 *
 * This constructor also starts the thread, which keeps running as long
 * as the _running member variable is true
 */
AsiGuidePort::AsiGuidePort(AsiCamera& camera)
	: GuidePort(asiGuideportName(camera.index())), _camera(camera) {
	std::unique_lock<std::mutex>	lock(_mutex);
	_ra = 0;
	_dec = 0;
	_running = true;
	_thread = std::thread(main, this);
}

/**
 * \brief Destroy the guideport
 *
 * The destructor must make sure the thread is terminated before it goes
 * away, because the thread still references the resources of the object,
 * which would cause a segmentation fault.
 */
AsiGuidePort::~AsiGuidePort() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destructor of the guideport");
	stop();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "guideport completed");
}

/**
 * \brief Find out which pins are active
 */
uint8_t	AsiGuidePort::active() {
	std::unique_lock<std::mutex>	lock(_mutex);
	uint8_t	result = 0;
	if (_ra > 0) {
		result |= GuidePort::RAPLUS;
	}
	if (_ra < 0) {
		result |= GuidePort::RAMINUS;
	}
	if (_dec > 0) {
		result |= GuidePort::DECPLUS;
	}
	if (_dec < 0) {
		result |= GuidePort::DECMINUS;
	}
	return result;
}

/**
 * \brief Activate the outputs for some amount of time
 */
void	AsiGuidePort::activate(float raplus, float raminus,
		float decplus, float decminus) {
	int	activationlimit = 1000000;
	if ((fabs(raplus - raminus) > activationlimit)
		|| (fabs(decplus - decminus) > activationlimit)) {
		std::string	portname = name();
		std::string	msg = stringprintf("%s activation time too "
			"long: %f/%f/%f/%f", raplus, raminus, decplus, decminus,
			portname.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	std::unique_lock<std::mutex>	lock(_mutex);
	_ra = (raplus - raminus) * 1000;
	_dec = (decplus - decminus) * 1000;
	_condition.notify_one();
}

/**
 * \brief Start northward movement
 */
void	AsiGuidePort::north() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s north movement",
		name().toString().c_str());
	_camera.pulseGuideOff(AsiCamera::asi_guide_south);
	_camera.pulseGuideOn(AsiCamera::asi_guide_north);
}

/**
 * \brief Start southward movement
 */
void	AsiGuidePort::south() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s south movement",
		name().toString().c_str());
	_camera.pulseGuideOff(AsiCamera::asi_guide_north);
	_camera.pulseGuideOn(AsiCamera::asi_guide_south);
}

/**
 * \brief Start eastward movement
 */
void	AsiGuidePort::east() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s east movement",
		name().toString().c_str());
	_camera.pulseGuideOff(AsiCamera::asi_guide_west);
	_camera.pulseGuideOn(AsiCamera::asi_guide_east);
}

/**
 * \brief Start westward movement
 */
void	AsiGuidePort::west() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s west movement",
		name().toString().c_str());
	_camera.pulseGuideOff(AsiCamera::asi_guide_east);
	_camera.pulseGuideOn(AsiCamera::asi_guide_west);
}

/**
 * \brief Stop RA movement
 */
void	AsiGuidePort::rastop() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s stop RA movement",
		name().toString().c_str());
	_camera.pulseGuideOff(AsiCamera::asi_guide_east);
	_camera.pulseGuideOff(AsiCamera::asi_guide_west);
}

/**
 * \brief Stop DEC movement
 */
void	AsiGuidePort::decstop() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s stop DEC movement",
		name().toString().c_str());
	_camera.pulseGuideOff(AsiCamera::asi_guide_north);
	_camera.pulseGuideOff(AsiCamera::asi_guide_south);
}

/**
 * \brief The work function
 *
 * This function does the actual work of activating the guide port outputs
 * for a given amount of time.
 */
void	AsiGuidePort::run() {
	std::unique_lock<std::mutex>	lock(_mutex);
	// we keep running until the _running variable becomes false
	// we are sure not to miss this event, because we carefully lock
	// the private data of the class all the time
	while (_running) {
		// find the time (in milliseconds) when the next action
		// should finish
		int	raduration = 0;
		int	decduration = 0;

		// first check for any right ascension movement
		if (0 != _ra) {
			if (_ra > 0) {
				west();
				raduration = _ra;
			} else {
				east();
				raduration = -_ra;
			}
		} else {
			rastop();
		}

		// check whether any declination movement is needed
		if (0 != _dec) {
			if (_dec > 0) {
				north();
				decduration = _dec;
			} else {
				south();
				decduration = -_dec;
			}
		} else {
			decstop();
		}

		// compute the time to the next event
		int	duration = 0;
		if (raduration > 0) {
			if (decduration > 0) {
				duration = std::min(raduration, decduration);
			} else {
				duration = raduration;
			}
		} else {
			duration = decduration;
		}

		// so we are done, and wait for new information
		if (0 == duration) {
			// just wait for new information
			_condition.wait(lock);
		} else {
			// wait for the condition variable for the duration
			// in milliseconds.
			std::cv_status	status = _condition.wait_for(lock,
				std::chrono::milliseconds(duration));
			// if this times out, then we have waited for the
			// duration, so we change the _ra und _dec variables
			// accordingly
			if (status == std::cv_status::timeout) {
				if (_ra > 0) {
					_ra -= duration;
				}
				if (_ra < 0) {
					_ra += duration;
				}
				if (_dec > 0) {
					_dec -= duration;
				}
				if (_dec < 0) {
					_dec += duration;
				}
			}
			// in other cases, i.e. if it does not time out,
			// then new settings for _ra and _dec must have become
			// available, so we just loop around, and pick up the
			// new values
		}
	}
}

/**
 * \brief Stop the guideport thread
 */
void	AsiGuidePort::stop() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stopping the guideport");
	{
		std::unique_lock<std::mutex>	lock(_mutex);
		_running = false;
	}
	_condition.notify_all();
	if (_thread.joinable()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "join the guideport thread");
		_thread.join();
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stop complete");
}

} // namespace asi
} // namespace camera
} // namespace astro
