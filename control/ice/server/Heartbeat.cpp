/*
 * Heartbeat.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <Heartbeat.h>
#include <AstroFormat.h>
#include <AstroDebug.h>

namespace snowstar {

/**
 * \brief Specialization of the callback_adapter for HeartbeatMonitorPrx
 */
template<>
void	callback_adapter<HeartbeatMonitorPrx>(HeartbeatMonitorPrx& p,
		const astro::callback::CallbackDataPtr data) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adapter");
	astro::callback::IntegerCallbackData	*icd
		= dynamic_cast<astro::callback::IntegerCallbackData *>(&*data);
	if (icd != NULL) {
		int	sequence_number = icd->value();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "sequence number to send: %d",
			sequence_number);
		try {
			p->beat(sequence_number);
			return;
		} catch (const std::exception& x) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"exception during beat: %s", x.what());
		}
	}

	astro::callback::FloatCallbackData	*fcd
		= dynamic_cast<astro::callback::FloatCallbackData*>(&*data);
	if (fcd != NULL) {
		float	interval = fcd->value();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "new interval length: %f",
			interval);
		try {
			p->interval(interval);
			return;
		} catch (const std::exception& x) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"exception during interval: %s", x.what());
		}
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "don't know how to handle %s data",
		astro::demangle(typeid(*data).name()).c_str());
}

/**
 * \brief The heartbeat trampoline function to start the run method
 *
 * \param hb	the heartbeat class of which the run() method should be called
 */
static void	heartbeat_run(Heartbeat *hb) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "startung heartbeat run method");
	try {
		hb->run();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "heartbeat run method returns");
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "heartbeat run method throws: %s",
			x.what());
	}
}

/**
 * \brief Construct a heartbeat server
 *
 * The constructor also starts the thread
 *
 * \param interval	the heartbeat interval 
 */
Heartbeat::Heartbeat(float interval) : _sequence_number(0), _interval(interval),
	_terminate(false), _thread(heartbeat_run, this) {
	_paused = false;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "heartbeat initialize");
}

/**
 * \brief Destroy the heartbeat server
 */
Heartbeat::~Heartbeat() {
	try {
		terminate(true);
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "termination: %s", x.what());
	}
	if (_thread.joinable()) {
		// wait until thread has terminated
		_thread.join();
	}
}

/**
 * \brief Send the interval to the clients
 */
void	Heartbeat::send_interval() {
	astro::callback::FloatCallbackData	*fcd
		= new astro::callback::FloatCallbackData(_interval);
	astro::callback::CallbackDataPtr	data(fcd);
	try {
		callbacks(data);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "heartbeat failed: %s", x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "heartbeat failed for unknown "
			"reason");
	}
}

/**
 * \brief Change the interval
 *
 * This method also signals the thread that the heartbeat interval has 
 * changed. 
 *
 * \param i	the new interval to use
 */
void	Heartbeat::interval(float f) {
	if (f < 0) {
		std::string	msg = astro::stringprintf("negative interval "
			"%f not allowed", f);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	{
		std::unique_lock<std::mutex>	lock(_mutex);
		_interval = f;
		_cond.notify_all();
	}
	// signal the new interval length
	send_interval();
}

/**
 * \brief Change the termination status
 *
 * Note that the heartbeat can currently not be restarted, if you set
 * terminate to true, future invocations of this method will throw an
 * exception
 *
 * \param t	whether or not to terminate
 */
void	Heartbeat::terminate(bool t) {
	std::unique_lock<std::mutex>	lock(_mutex);
	if (_terminate) {
		std::string	msg;
		if (t) {
			msg = "heartbeat already terminated";
		} else {
			msg = "cannot restart heartbeat";
		}
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	_terminate = t;
	_cond.notify_all();
}

/**
 * \brief The heartbeat run method
 *
 * This method sends a heartbeat every <interval> seconds. If <interval> is
 * 0, then it does not do anything. 
 */
void	Heartbeat::run() {
	std::unique_lock<std::mutex>	lock(_mutex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting the run method");
	do {
		// wait for interval seconds or for a signal
		long	milliseconds = 1000 * _interval;
		if (milliseconds > 0) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"waiting for %.3f seconds", _interval);
			std::chrono::milliseconds	ivl(milliseconds);
			if (_cond.wait_for(lock, ivl)
				== std::cv_status::timeout) {
				// if the timer expires, send the heartbeat
				send();
			} else {
				debug(LOG_DEBUG, DEBUG_LOG, 0, "state change");
			}
		} else {
			_cond.wait(lock);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "state change");
		}
	} while (!_terminate);
	// send the stop signal
	try {
		callbacks.stop();
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "failed to send stop signal: %s",
			x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot send the stop signal");
	}
}

/**
 * \brief Send heartbeat calls to all the clients
 */
void	Heartbeat::send() {
	_sequence_number++;
	if (_paused) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "paused, not sending %d",
			_sequence_number);
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sending heartbeat %d",
		sequence_number());
	// do the actual sending
	astro::callback::IntegerCallbackData	*icd
		= new astro::callback::IntegerCallbackData(_sequence_number);
	astro::callback::CallbackDataPtr	data(icd);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "callback data: %p", icd);
	try {
		callbacks(data);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "heartbeat failed: %s", x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "heartbeat failed for unknown "
			"reason");
	}
}

/**
 * \brief Register the heartbeat monitor callback
 *
 * \param heartbeatmonitor      the identity of the monitor to register
 * \param current		the current call context
 */
void    Heartbeat::doregister(
		const Ice::Identity& heartbeatmonitor,
                const Ice::Current& current) {
        // do the registering
	try {
		callbacks.registerCallback(heartbeatmonitor, current);
		send_interval();
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot register callback: %s %s",
			astro::demangle(typeid(x).name()).c_str(), x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cannot register callback for unknown reason");
	}
}

/**
 * \brief Unregister a heartbeat monitor callback
 *
 * \param heartbeatmonitor      the identity of the monitor to unregister
 * \param current		the current call context
 */
void    Heartbeat::unregister(
		const Ice::Identity& heartbeatmonitor,
		const Ice::Current& current) {
        // do the actual unregistering
	try {
		callbacks.unregisterCallback(heartbeatmonitor, current);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot unregister callback: %s %s",
			astro::demangle(typeid(x).name()).c_str(), x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cannot unregister callback for unknown reason");
	}
}

void	Heartbeat::pause() {
	_paused = true;
}

void	Heartbeat::resume() {
	_paused = false;
}

} // namespace snowstar
