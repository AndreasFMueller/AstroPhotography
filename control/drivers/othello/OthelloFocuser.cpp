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
 */
OthelloFocuser::OthelloFocuser(DevicePtr _deviceptr)
	: Focuser(othellodevname(_deviceptr)), deviceptr(_deviceptr) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create a new focuser");
}

/**
 * \brief Destroy the focuser instance
 */
OthelloFocuser::~OthelloFocuser() {
}

long	OthelloFocuser::min() {
	return 1;
}

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
	// bring the value into the interval
	if (value < min()) {
		value = min();
	}
	if (value > max()) {
		value = max();
	}
	// prepare the structure to send
	othello_set_t	setdata;
	setdata.set = value;
	Request<othello_set_t>	request(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient, 1 /* fast move */,
		(uint8_t)FOCUSER_SET, 0, &setdata);
	int	retrycounter = 3;
	while (retrycounter-- > 0) {
		try {
			deviceptr->controlRequest(&request);
			return;
		} catch (const std::exception& x) {
			std::string	msg = stringprintf("control request "
				"failed: %s", x.what());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		}
	}
	// lets hope the client realizes that the focuser is not moving
}

} // namespace astro
} // namespace camera
} // namespace othello
