/*
 * OthelloGuidePort.cpp -- guider port implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <OthelloGuidePort.h>
#include <AstroDebug.h>
#include <algorithm>
#include <OthelloUtil.h>

using namespace astro::usb;

namespace astro {
namespace camera {
namespace othello {

#if 0
#define	OTHELLO_RAPLUS_BIT	1
#define OTHELLO_DECPLUS_BIT	2
#define OTHELLO_DECMINUS_BIT	4
#define	OTHELLO_RAMINUS_BIT	8
#endif

#define GUIDEPORT_SET		1
#define GUIDEPORT_SET_ALL_TIMES	3
#define GUIDEPORT_GET		4

OthelloGuidePort::OthelloGuidePort(astro::usb::DevicePtr _deviceptr)
	: GuidePort(othellodevname(_deviceptr)),
	  deviceptr(_deviceptr) {
}

OthelloGuidePort::~OthelloGuidePort() {
}

typedef struct othello_set_all_times_s {
	uint16_t	raplus;		// port 0 on GuidePort
	uint16_t	decplus;	// port 1 on GuidePort
	uint16_t	decminus;	// port 2 on GuidePort
	uint16_t	raminus;	// port 3 on GuidePort
} __attribute__((packed)) othello_set_all_times_t;

static uint16_t	othellotime(float t) {
	if (t < 0) { return 0; }
	if (t > 655.35) {
		return 65535;
	}
	uint16_t result = 100 * t;
	return result;
}

void	OthelloGuidePort::activate(float raplus, float raminus,
		float decplus, float decminus) {
	othello_set_all_times_t	payload;
	payload.raplus = othellotime(raplus);
	payload.raminus = othellotime(raminus);
	payload.decplus = othellotime(decplus);
	payload.decminus = othellotime(decminus);
	
	Request<othello_set_all_times_t> r(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient, 0, GUIDEPORT_SET_ALL_TIMES,
		0, &payload);
	try {
		deviceptr->controlRequest(&r);
	} catch (const std::exception& x) {
		std::string 	cause = stringprintf(
			"set all times %hu %hu %hu %hu failed: %s",
			payload.raplus, payload.raminus,
			payload.decplus, payload.decminus, x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
		throw x;
	}
}

typedef	uint8_t	active_t;

uint8_t	OthelloGuidePort::active() {
	Request<active_t>	request(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient, 0xf,
		(uint8_t)GUIDEPORT_GET, 0);
	try {
		deviceptr->controlRequest(&request);
	} catch (const std::exception& x) {
		std::string	cause = stringprintf("can't get active: %s",
			x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
		throw x;
	}
	return *request.data();
}

} // namespace othello
} // namespace camera
} // namespace astro
