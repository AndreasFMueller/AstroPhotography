/*
 * SxCcdM26C.cpp -- implementation for the M26C camera
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SxCcd.h>
#include <AstroCamera.h>
#include <AstroImage.h>
#include <sx.h>
#include <debug.h>

namespace astro {
namespace camera {
namespace sx {

Exposure	SxCcdM26C::m26cExposure() {
	Exposure	m26c;

	// adapt the size suitable for 
	m26c.frame.size.width = exposure.frame.size.width / 4;
	m26c.frame.size.height = exposure.frame.size.height * 2;
	if (m26c.mode.getY() > 1) {
		m26c.frame.size.height -= m26c.frame.size.height % 2;
	}
	exposure.frame.size.width = m26c.frame.size.width * 4;
	exposure.frame.size.height = m26c.frame.size.height / 2;

	// adapt the top left corner
	m26c.frame.origin.x = exposure.frame.origin.x / 4;
	m26c.frame.origin.y = exposure.frame.origin.y * 2;
	if (m26c.mode.getY() > 1) {
		m26c.frame.origin.y -= m26c.frame.origin.y % 2;
	}
	exposure.frame.origin.x = m26c.frame.origin.x * 4;
	exposure.frame.origin.y = m26c.frame.origin.y / 2;

	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", m26c.toString().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", exposure.toString().c_str());

	// return the modified exposure structure
	return m26c;
}

SxCcdM26C::SxCcdM26C(const CcdInfo& info, SxCamera& camera, int id)
	: SxCcd(info, camera, id) {
}

#define M26C_WIDTH	3906
#define M26C_HEIGHT	2616

static int	max(const int *v, int l) {
	int	result = v[0];
	for (int i = 0; i < l; i++) {
		if (v[i] < result) {
			result = v[i];
		}
	}
	return result;
}

static int	min(const int *v, int l) {
	int	result = v[0];
	for (int i = 1; i < l; i++) {
		if (v[i] < result) {
			result = v[i];
		}
	}
	return result;
}

Exposure	SxCcdM26C::symmetrize(const Exposure& exp) const {
	Exposure	symexp = exp;
	int	x[4], y[4];
	x[0] = exp.frame.origin.x;
	y[0] = exp.frame.origin.y;
	x[1] = M26C_WIDTH - x[0];
	y[1] = M26C_HEIGHT - y[0];
	x[2] = exp.frame.origin.x + exp.frame.size.width;
	y[2] = exp.frame.origin.y + exp.frame.size.height;
	x[3] = M26C_WIDTH - x[2];
	y[3] = M26C_HEIGHT - y[2];
	symexp.frame.origin.x = min(x, 4);
	symexp.frame.origin.y = min(y, 4);
	symexp.frame.size.width = max(x, 4) - symexp.frame.origin.x;
	if (symexp.frame.size.width > 3900) {
		symexp.frame.size.width = 3900;
	}
	symexp.frame.size.height = max(y, 4) - symexp.frame.origin.y;
	return symexp;
}

void	SxCcdM26C::startExposure(const Exposure& exposure)
		throw (not_implemented) {
	// remember the exposre, we need it for the second field for the
	// case where we do two fields one after the other
	this->exposure = symmetrize(exposure);
	Exposure	m26c = m26cExposure();

	// compute a better request for the M26C camera
	sx_read_pixels_delayed_t	rpd;

	// send the request to the camera
	Request<sx_read_pixels_delayed_t>	request(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient, ccdindex,
		(uint8_t)SX_CMD_READ_PIXELS_DELAYED, (uint16_t)0, &rpd);
	camera.getDevicePtr()->controlRequest(&request);

	// we are now in exposing state
	state = Exposure::exposing;
}

ShortImagePtr	SxCcdM26C::shortImage() throw (not_implemented) {
	return ShortImagePtr();
}

SxCcdM26C::~SxCcdM26C() {
}

} // namespace sx
} // namespace camera
} // namespace astro
