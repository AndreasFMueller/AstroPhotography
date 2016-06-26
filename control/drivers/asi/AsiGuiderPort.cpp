/*
 * AsiGuiderPort.cpp -- ASI guider port implementation
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AsiGuiderPort.h>
#include <AstroDebug.h>
#include <utils.h>

namespace astro {
namespace camera {
namespace asi {

/**
 * \brief trampoline function to start the run function of the class
 */
static void	*asi_main(void *parameter) {
	AsiGuiderPort	*port = (AsiGuiderPort *)parameter;
	std::string	portname = port->name();
	try {
		port->run();
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("guider port %s failed: %s",
			portname.c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		return NULL;
	} catch (...) {
		std::string	msg = stringprintf("guiderport %s thread "
			"failed (unknown exception", portname.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		return NULL;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s thread terminates",
		portname.c_str());
	return parameter;
}

/**
 * \brief Create a new AsiGuiderPort
 *
 * This constructor also starts the thread, which keeps running as long
 * as the _running member variable is true
 */
AsiGuiderPort::AsiGuiderPort(AsiCamera& camera)
	: GuiderPort(asiGuiderportName(camera.index())), _camera(camera) {
	std::unique_lock<std::mutex>	lock(_mutex);
	_ra = 0;
	_dec = 0;
	_running = true;
	_thread = new std::thread(asi_main, this);
}

/**
 * \brief Destroy the guiderport
 *
 * The destructor must make sure the thread is terminated before it goes
 * away, because the thread still references the resources of the object,
 * which would cause a segmentation fault.
 */
AsiGuiderPort::~AsiGuiderPort() {
	{
		std::unique_lock<std::mutex>	lock(_mutex);
		_running = false;
	}
	activate(0, 0, 0, 0);
	_thread->join();
	delete _thread;
}

/**
 * \brief Find out which pins are active
 */
uint8_t	AsiGuiderPort::active() {
	std::unique_lock<std::mutex>	lock(_mutex);
	uint8_t	result = 0;
	if (_ra > 0) {
		result |= GuiderPort::RAPLUS;
	}
	if (_ra < 0) {
		result |= GuiderPort::RAMINUS;
	}
	if (_dec > 0) {
		result |= GuiderPort::DECPLUS;
	}
	if (_dec < 0) {
		result |= GuiderPort::DECMINUS;
	}
	return result;
}

/**
 * \brief Activate the outputs for some amount of time
 */
void	AsiGuiderPort::activate(float raplus, float raminus,
		float decplus, float decminus) {
	if ((fabs(raplus - raminus) > 1000) || (fabs(decplus - decminus))) {
		std::string	portname = name();
		std::string	msg = stringprintf("%s activation time too "
			"long: %f/%f/%f/%f", raplus, raminus, decplus, decminus,
			portname.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	std::unique_lock<std::mutex>	lock(_mutex);
	_ra = (raplus - raminus) * 1000;
	_dec = (decplus - decplus - decminus) * 1000;
	_condition.notify_one();
}

/**
 * \brief Start northward movement
 */
void	AsiGuiderPort::north() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s north movement",
		name().toString().c_str());
	ASIPulseGuideOff(_camera.index(), ASI_GUIDE_SOUTH);
	ASIPulseGuideOn(_camera.index(), ASI_GUIDE_NORTH);
}

/**
 * \brief Start southward movement
 */
void	AsiGuiderPort::south() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s south movement",
		name().toString().c_str());
	ASIPulseGuideOff(_camera.index(), ASI_GUIDE_NORTH);
	ASIPulseGuideOn(_camera.index(), ASI_GUIDE_SOUTH);
}

/**
 * \brief Start eastward movement
 */
void	AsiGuiderPort::east() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s east movement",
		name().toString().c_str());
	ASIPulseGuideOff(_camera.index(), ASI_GUIDE_WEST);
	ASIPulseGuideOn(_camera.index(), ASI_GUIDE_EAST);
}

/**
 * \brief Start westward movement
 */
void	AsiGuiderPort::west() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s west movement",
		name().toString().c_str());
	ASIPulseGuideOff(_camera.index(), ASI_GUIDE_EAST);
	ASIPulseGuideOn(_camera.index(), ASI_GUIDE_WEST);
}

/**
 * \brief Stop RA movement
 */
void	AsiGuiderPort::rastop() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s stop RA movement",
		name().toString().c_str());
	ASIPulseGuideOff(_camera.index(), ASI_GUIDE_EAST);
	ASIPulseGuideOff(_camera.index(), ASI_GUIDE_WEST);
}

/**
 * \brief Stop DEC movement
 */
void	AsiGuiderPort::decstop() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s stop DEC movement",
		name().toString().c_str());
	ASIPulseGuideOff(_camera.index(), ASI_GUIDE_NORTH);
	ASIPulseGuideOff(_camera.index(), ASI_GUIDE_SOUTH);
}

/**
 * \brief The work function
 *
 * This function does the actual work of activating the guide port outputs
 * for a given amount of time.
 */
void	AsiGuiderPort::run() {
	std::unique_lock<std::mutex>	lock(_mutex);
	// we keep running until the _running variable becomes false
	// we are sure not to miss this event, because we carefully lock
	// the private data of the class all the time
	while (_running) {
		// find the time when the next action should finish
		int	duration = 0;

		// first check for any right ascension movement
		if (0 != _ra) {
			if (_ra > 0) {
				west();
				duration = _ra;
			} else {
				east();
				duration = -_ra;
			}
		} else {
			rastop();
		}

		// check whether any declination movement is needed
		if (0 != _dec) {
			if (_dec > 0) {
				north();
				if (_dec < duration) {
					duration = _dec;
				}
			} else {
				south();
				if (-_dec < duration) {
					duration = -_dec;
				}
			}
		} else {
			decstop();
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

} // namespace asi
} // namespace camera
} // namespace astro
