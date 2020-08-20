/*
 * OthelloFocuser.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <OthelloFocuser.h>
#include <OthelloUtil.h>

using namespace astro::usb;

namespace astro {
namespace camera {
namespace othello {

#define FOCUSER_RESET	0
#define FOCUSER_GET	1
#define FOCUSER_SET	2
#define FOCUSER_LOCK	3
#define FOCUSER_RCVR	4
#define FOCUSER_STOP	5
#define FOCUSER_SAVED	6

/**
 * \brief Construct a new Focuser instance
 *
 * \param _deviceptr	the device to operate on
 */
OthelloFocuser::OthelloFocuser(DevicePtr _deviceptr)
	: Focuser(othellodevname(_deviceptr)), deviceptr(_deviceptr) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create a new focuser");
	_running = true;
	start();
}

/**
 * \brief Destroy the focuser instance
 */
OthelloFocuser::~OthelloFocuser() {
	stop();
}

/**
 * \brief The main trampoline function
 *
 * \param focuser	run the run() method of this focuser
 */
void	OthelloFocuser::main(OthelloFocuser *focuser) noexcept {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start focuser monitoring thread");
	try {
		focuser->run();
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "focuser %s run() failed: %s",
			focuser->name().toString().c_str(), x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "focuser %s run() crashed",
			focuser->name().toString().c_str());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "focuser monitoring thread terminates");
}

/**
 * \brief The run method of the monitoring thread
 */
void	OthelloFocuser::run() {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	long	_previous = 0;
	while (_running) {
		long	_current = current();
		if (_previous != _current) {
			callback(_current, (_current == _targetposition));
		}
		_previous = _current;
		_condition.wait_for(lock, std::chrono::milliseconds(1000));
	}
}

/**
 * \brief Start the focuser monitoring thread
 */
void	OthelloFocuser::start() {
	if (_thread.joinable()) {
		return;
	}
	_running = true;
	_thread = std::thread(OthelloFocuser::main, this);
}

/**
 * \brief Stop the monitoring thread, wait until it completes
 */
void	OthelloFocuser::stop() {
	{
		std::unique_lock<std::recursive_mutex>	lock(_mutex);
		_running = false;
	}
	_condition.notify_all();
	if (_thread.joinable()) {
		_thread.join();
	}
}

/**
 * \brief Get the minimum value for our own focuser
 */
long	OthelloFocuser::min() {
	return 1;
}

/**
 * \brief Get the maximum value for our own focuser
 */
long	OthelloFocuser::max() {
	return 16777214;
}

typedef struct othello_get_s {
	int32_t	current;
	int32_t	target;
	int32_t	speed;
} __attribute__((packed)) othello_get_t;

typedef struct othello_set_s {
	int32_t		set;
} __attribute__((packed)) othello_set_t;

/**
 * \brief get the current position of the focuser
 *
 * \return	current position of the focuser
 */
long	OthelloFocuser::current() {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	Request<othello_get_t>	request(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient, 0,
		(uint8_t)FOCUSER_GET, 0);
	try {
		deviceptr->controlRequest(&request);
		_current = request.data()->current;
		return _current;
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("cannot get current: %s",
			x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	}
	// if current retrieval fails, just return the previous value
	return _current;
}

/**
 * \brief Set the position to move to
 *
 * \param value		the value the focuser should move to
 */
void	OthelloFocuser::set(long value) {
	// the parent class set method also triggers the callback
	Focuser::set(value);

	// prepare the structure to send
	othello_set_t	setdata;
	setdata.set = value;
	Request<othello_set_t>	request(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient, 1 /* fast move */,
		(uint8_t)FOCUSER_SET, 0, &setdata);

	// don't wait for more than one second for a response
	request.setTimeout(1);

	// now try to send the request. repeat 3 times if we fail
	{
		std::unique_lock<std::recursive_mutex>	lock(_mutex);
		int	retrycounter = 3;
		while (retrycounter-- > 0) {
			try {
				deviceptr->controlRequest(&request);
				// notify others of the change
				_condition.notify_all();
				return;
			} catch (const std::exception& x) {
				std::string	msg = stringprintf("control "
					"request failed: %s", x.what());
				debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			}
		}
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "focuser update to %ld failed", value);
}

} // namespace astro
} // namespace camera
} // namespace othello
