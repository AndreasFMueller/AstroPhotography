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

#define	FIELD_CUTOVER	10

namespace astro {
namespace camera {
namespace sx {

SxCcdM26C::Field::Field(size_t l) : length(l) {
	data = new unsigned short[length];
}

SxCcdM26C::Field::~Field() {
	delete [] data;
	data = NULL;
}

Exposure	SxCcdM26C::m26cExposure() {
	Exposure	m26c;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "compute the exposure parameters for "
		"the M26C's CCD chip");

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

	// copy stuff that is not affected
	m26c.mode = exposure.mode;
	m26c.exposuretime = exposure.exposuretime;
	m26c.gain = 1.;

	// return the modified exposure structure
	return m26c;
}

SxCcdM26C::SxCcdM26C(const CcdInfo& info, SxCamera& camera, int id)
	: SxCcd(info, camera, id) {
}

/**
 * \brief Read the field requested previously
 */
SxCcdM26C::Field	*SxCcdM26C::readField() {
	// allocate a structure for the result
	size_t	l = (m26c.frame.size.width / m26c.mode.getX())
		* (m26c.frame.size.height / m26c.mode.getY());
	Field	*field = new Field(l);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "transfer field of size %u",
		field->length);

	// claim the interface
	try {
		camera.getInterface()->claim();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "interface %d claimed",
			camera.getInterface()->interfaceNumber());
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot claim interface: %s",
			x.what());
		throw x;
	}

	// perform the data transfer
	try {
		BulkTransfer	transfer(camera.getEndpoint(), (int)l * 2,
			(unsigned char *)field->data);
		transfer.setTimeout(exposure.exposuretime * 1.1 + 30);
		camera.getDevicePtr()->submit(&transfer);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "transfer complete");
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "transfer failed: %s", x.what());
		throw x;
	}

	// release the interface again
	camera.getInterface()->release();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "interface %d released",
		camera.getInterface()->interfaceNumber());

	// return the field
	return field;
}

/**
 * \brief Request exposure of a field.
 *
 * This starts a new exposure of the SX camera, so after this command, both
 * fields are cleared. If the second field has to be read too, the requestField
 * should be used, or a new request should be used.
 * \param field	number of the field to expose.
 */
void	SxCcdM26C::exposeField(int field) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request exposure of a field");
	// compute a better request for the M26C camera
	sx_read_pixels_delayed_t	rpd;
	rpd.delay = 1000 * m26c.exposuretime;
	rpd.width = m26c.frame.size.width;
	rpd.height = m26c.frame.size.height;
	rpd.x_offset = m26c.frame.origin.x;
	rpd.y_offset = m26c.frame.origin.y;
	rpd.x_bin = m26c.mode.getX();
	rpd.y_bin = m26c.mode.getY();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request: %hux%hu@(%hu,%hu)/(%d,%d)",
		rpd.width, rpd.height, rpd.x_offset, rpd.y_offset,
		rpd.x_bin, rpd.y_bin);

	// send the request to the camera, this is a request for field 0 
	// field 1 has to be retrieved separately
	try {
		Request<sx_read_pixels_delayed_t>	request(
			RequestBase::vendor_specific_type,
			RequestBase::device_recipient, ccdindex,
			(uint8_t)SX_CMD_READ_PIXELS_DELAYED,
			(uint16_t)(1 << field), &rpd);
		camera.getDevicePtr()->controlRequest(&request);
	} catch (std::exception &x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot request field: %s",
			x.what());
		throw x;
	}
}

/**
 * \brief Request a field, without starting a new exposure.
 *
 * This method requests the already exposed field. It is usually used 
 * after the first field has been exposed and downloaded using the exposeField
 * and readField methods.
 * \param field		field number of the field to download
 */
void	SxCcdM26C::requestField(int field) {
	// prepare a request for the pixels, without delay, this just
	// downloads the already exposed field
	sx_read_pixels_t	rpd;
	rpd.width = m26c.frame.size.width;
	rpd.height = m26c.frame.size.height;
	rpd.x_offset = m26c.frame.origin.x;
	rpd.y_offset = m26c.frame.origin.y;
	rpd.x_bin = m26c.mode.getX();
	rpd.y_bin = m26c.mode.getY();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request: %hux%hu@(%hu,%hu)/(%d,%d)",
		rpd.width, rpd.height, rpd.x_offset, rpd.y_offset,
		rpd.x_bin, rpd.y_bin);

	// send the request to the camera, this is a request for field 0 
	// field 1 has to be retrieved separately
	try {
		Request<sx_read_pixels_t>	request(
			RequestBase::vendor_specific_type,
			RequestBase::device_recipient, ccdindex,
			(uint8_t)SX_CMD_READ_PIXELS, (uint16_t)(1 << field),
			&rpd);
		camera.getDevicePtr()->controlRequest(&request);
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot request field: %s",
			x.what());
		throw x;
	}
}

#define M26C_WIDTH	3906
#define M26C_HEIGHT	2616

static int	max(const int *v, int l) {
	int	result = v[0];
	for (int i = 0; i < l; i++) {
		if (v[i] > result) {
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

/**
 * \brief Symmetrize the exposure
 *
 * The M26C has an interlaced CCD which reads the different fields and
 * colors from different sides of the chip. This only works for symmetric
 * exposures, symmetric with respect to the center of the CCD chip. This
 * method computes a symmetrized exposure object.
 */
Exposure	SxCcdM26C::symmetrize(const Exposure& exp) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "symmetrizing exposure %s",
		exp.toString().c_str());
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "x[] = %d %d %d %d",
		x[0], x[1], x[2], x[3]);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "y[] = %d %d %d %d",
		y[0], y[1], y[2], y[3]);
	symexp.frame.origin.x = min(x, 4);
	symexp.frame.origin.y = min(y, 4);
	symexp.frame.size.width = max(x, 4) - symexp.frame.origin.x;
	if (symexp.frame.size.width > 3900) {
		symexp.frame.size.width = 3900;
	}
	symexp.frame.size.height = max(y, 4) - symexp.frame.origin.y;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "symmetrized exposure: %s",
		symexp.toString().c_str());
	return symexp;
}

/**
 * \brief Start an exposure on the M26C camera
 *
 * \param exposure	exposure structure for the exposure to perform
 */
void	SxCcdM26C::startExposure(const Exposure& exposure)
		throw (not_implemented) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "expousre %s requested",
		exposure.toString().c_str());
	// remember the exposre, we need it for the second field for the
	// case where we do two fields one after the other
	this->exposure = symmetrize(exposure);
	m26c = m26cExposure();

	// compute a better request for the M26C camera
	exposeField(0);
	timer.start();

	// we are now in exposing state
	state = Exposure::exposing;
}

/**
 * \brief Rescale a field
 *
 * This method scales the pixels of the field with the factor. The factor
 * must be >1 because otherwise saturated pixels become unsaturated by
 * the scaling operation, leading to wrong colors.
 * \param field		field object to rescale
 * \param scale
 */
void	SxCcdM26C::Field::rescale(double scale) {
	for (size_t i = 0; i < length; i++) {
		unsigned long	rescaled = data[i] * scale;
		if (rescaled > 0xffff) {
			data[i] = 0xffff;
		} else {
			data[i] = rescaled;
		}
	}
}

ShortImagePtr	SxCcdM26C::shortImage() throw (not_implemented) {

	// read the right number of pixels from the IN endpoint
	Field	*field0 = readField();
	Field	*field1 = NULL;

	// for long exposures, we just read the second field.
	if (exposure.exposuretime > FIELD_CUTOVER) {
		requestField(1);
		timer.end();
		field1 = readField();

		// rescale the field. We have to multiply the first field
		// with the scaling factor, because of overflows
		field0->rescale(timer.elapsed() / exposure.exposuretime);
	} else {
		exposeField(1);
		field1 = readField();
	}

	// now we have to demultiplex the two fields

	// remove the data
	delete field0;
	delete field1;

	// return the demultiplexed image
	return ShortImagePtr();
}

SxCcdM26C::~SxCcdM26C() {
}

} // namespace sx
} // namespace camera
} // namespace astro
