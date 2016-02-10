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
	
}

/**
 * \brief Destroy the focuser instance
 */
OthelloFocuser::~OthelloFocuser() {
}

unsigned short	OthelloFocuser::min() {
	return 1;
}

unsigned short	OthelloFocuser::max() {
	return 0xffff;
}

typedef struct othello_get_s {
	uint16_t	current;
	uint16_t	target;
	uint16_t	speed;
} __attribute__((packed)) othello_get_t;

/**
 * \brief get the current position of the focuser
 */
unsigned short	OthelloFocuser::current() {
	Request<othello_get_t>	request(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient, 0,
		(uint8_t)FOCUSER_GET, 0);
	deviceptr->controlRequest(&request);
	return request.data()->current;
}

/**
 * \brief Set the position to move to
 */
void	OthelloFocuser::set(unsigned short value) {
	EmptyRequest	request(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient, 0,
		(uint8_t)FOCUSER_SET, value);
	deviceptr->controlRequest(&request);
}

} // namespace astro
} // namespace camera
} // namespace othello
