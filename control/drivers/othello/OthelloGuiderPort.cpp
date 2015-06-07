/*
 * OthelloGuiderPort.cpp -- guider port implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <OthelloGuiderPort.h>
#include <AstroDebug.h>
#include <algorithm>
#include <OthelloUtil.h>

using namespace astro::usb;

namespace astro {
namespace camera {
namespace othello {

#define	OTHELLO_RAPLUS_BIT	1
#define OTHELLO_DECPLUS_BIT	2
#define OTHELLO_DECMINUS_BIT	4
#define	OTHELLO_RAMINUS_BIT	8

#define OTHELLO_SET		1
#define OTHELLO_SET_ALL_TIMES	3
#define OTHELLO_GET		4

OthelloGuiderPort::OthelloGuiderPort(astro::usb::DevicePtr _deviceptr)
	: GuiderPort(othellodevname(_deviceptr)),
	  deviceptr(_deviceptr) {
}

OthelloGuiderPort::~OthelloGuiderPort() {
}

typedef struct othello_set_all_times_s {
	uint16_t	raplus;
	uint16_t	raminus;
	uint16_t	decplus;
	uint16_t	decminus;
} __attribute__((packed)) othello_set_all_times_t;

static uint16_t	othellotime(float t) {
	if (t < 0) { return 0; }
	if (t > 655.35) {
		return 65535;
	}
	uint16_t result = 100 * t;
	return result;
}

void	OthelloGuiderPort::activate(float raplus, float raminus,
		float decplus, float decminus) {
	othello_set_all_times_t	payload;
	payload.raplus = othellotime(raplus);
	payload.raminus = othellotime(raminus);
	payload.decplus = othellotime(decplus);
	payload.decminus = othellotime(decminus);
	
	Request<othello_set_all_times_t> r(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient, 0, OTHELLO_SET_ALL_TIMES,
		0, &payload);
	deviceptr->controlRequest(&r);
}

typedef	uint8_t	active_t;

uint8_t	OthelloGuiderPort::active() {
	Request<active_t>	request(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient, 0xf,
		(uint8_t)OTHELLO_GET, 0);
	deviceptr->controlRequest(&request);
	return *request.data();
}

} // namespace othello
} // namespace camera
} // namespace astro
