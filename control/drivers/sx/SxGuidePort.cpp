/*
 * SxGuidePort.cpp -- guider port implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <algorithm>
#include <AstroDebug.h>
#include "SxGuidePort.h"
#include "SxUtils.h"

namespace astro {
namespace camera {
namespace sx {

#define	SX_RAPLUS_BIT	1
#define SX_DECPLUS_BIT	2
#define SX_DECMINUS_BIT	4
#define	SX_RAMINUS_BIT	8

SxGuidePort::SxGuidePort(SxCamera& _camera)
	: BasicGuideport(GuidePort::defaultname(_camera.name(), "guideport")),
	  camera(_camera) {
}

SxGuidePort::~SxGuidePort() {
}

void	SxGuidePort::do_activate(uint8_t active) {
	// log the state change
	BasicGuideport::do_activate(active);

	// activate in the USB device
	uint8_t	newstate = 0;
	newstate |= (active & RAPLUS) ? SX_RAPLUS_BIT : 0;
	newstate |= (active & RAMINUS) ? SX_RAMINUS_BIT : 0;
	newstate |= (active & DECPLUS) ? SX_DECPLUS_BIT : 0;
	newstate |= (active & DECMINUS) ? SX_DECMINUS_BIT : 0;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new port state: %02x", newstate);

	// now prepare the request
	EmptyRequest	request(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient, 0,
		(uint8_t)SX_CMD_SET_STAR2K, (uint16_t)newstate);

	try {
		// we need to reserve the camera
		if (camera.reserve("guideport", 100)) {
			camera.controlRequest(&request);
			camera.release("guideport");
		} else {
			debug(LOG_WARNING, DEBUG_LOG, 0,
				"cannot reserve the camera, giving up");
			return;
		}
	} catch (USBError& x) {
		camera.release("guideport");
		camera.refresh();
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "do_activate complete");
}

std::string	SxGuidePort::userFriendlyName() const {
	return camera.userFriendlyName();
}

} // namespace sx
} // namespace camera
} // namespace astro
