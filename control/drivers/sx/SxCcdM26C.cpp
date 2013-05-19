/*
 * SxCcdM26C.cpp -- implementation for the M26C camera
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SxCcd.h>
#include <SxDemux.h>
#include <AstroCamera.h>
#include <AstroImage.h>
#include <sx.h>
#include <debug.h>
#include <SxDemux.h>
#include <fstream>

#define	EXPOSURE_FIELD_CUTOVER		10
#define EXPOSURE_ADCONVERSION_TIME	30000

#define CCD_FLAGS_FIELD_ODD       1   /* Specify odd field for MX cameras */
#define CCD_FLAGS_FIELD_EVEN      2   /* Specify even field for MX cameras */
#define CCD_FLAGS_NOBIN_ACCUM     4   /* Don't accumulate charge if binning */
#define CCD_FLAGS_NOWIPE_FRAME    8   /* Don't apply WIPE when clearing frame */
#define CCD_FLAGS_TDI             32  /* Implement TDI (drift scan) operation */
#define CCD_FLAGS_NOCLEAR_FRAME   64  /* Don't clear frame, even when asked */

namespace astro {
namespace camera {
namespace sx {

/**
 * \brief Compute the exposure parameters for an M26C camera.
 *
 * The M26C camera has a very strange CCD that is actually read column by
 * column, not row by row. Thus we have to recompute the parameters for this
 * CCD.
 */
Exposure	SxCcdM26C::m26cExposure() {
	Exposure	m26c = exposure;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "compute the exposure parameters for "
		"the M26C's CCD chip");

	// adapt the size suitable for 
	m26c.frame.size.height = exposure.frame.size.width / 4;
	m26c.frame.size.width = exposure.frame.size.height * 2;
	if (m26c.mode.getX() > 1) {
		m26c.frame.size.height -= m26c.frame.size.height % 2;
	}
	exposure.frame.size.height = m26c.frame.size.width / 2;
	exposure.frame.size.width = m26c.frame.size.height * 4;

	// adapt the top left corner
	m26c.frame.origin.x = exposure.frame.origin.y * 2;
	m26c.frame.origin.y = exposure.frame.origin.x / 4;
	if (m26c.mode.getY() > 1) {
		m26c.frame.origin.y -= m26c.frame.origin.y % 2;
	}
	exposure.frame.origin.x = m26c.frame.origin.y / 2;
	exposure.frame.origin.y = m26c.frame.origin.x * 4;

	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", m26c.toString().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", exposure.toString().c_str());

	// copy stuff that is not affected
	m26c.mode = exposure.mode;
	m26c.exposuretime = exposure.exposuretime;
	m26c.gain = 1.;

	// return the modified exposure structure
	return m26c;
}

/**
 * \brief Create an SxCcdM26C object.
 */
SxCcdM26C::SxCcdM26C(const CcdInfo& info, SxCamera& camera, int id)
	: SxCcd(info, camera, id) {
}

/**
 * \brief Read the field requested previously
 */
Field	*SxCcdM26C::readField() {
	// allocate a structure for the result
	size_t	l = (m26c.frame.size.width / m26c.mode.getX())
		* (m26c.frame.size.height / m26c.mode.getY());
	Field	*field = new Field(exposure.frame.size, l);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "transfer field of size %u", l);

	// perform the data transfer
	try {
		BulkTransfer	transfer(camera.getEndpoint(), (int)l * 2,
			(unsigned char *)field->data);
		int	timeout = exposure.exposuretime * 1100
				+ EXPOSURE_ADCONVERSION_TIME;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "setting timeout: %d", timeout);
		transfer.setTimeout(timeout);
		camera.getDevicePtr()->submit(&transfer);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "transfer complete");
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "transfer failed: %s", x.what());
		throw x;
	}

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
		camera.controlRequest(&request);
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
#if 0
	sx_read_pixels_t	rp;
	rp.width = m26c.frame.size.width;
	rp.height = m26c.frame.size.height;
	rp.x_offset = m26c.frame.origin.x;
	rp.y_offset = m26c.frame.origin.y;
	rp.x_bin = m26c.mode.getX();
	rp.y_bin = m26c.mode.getY();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request: %hux%hu@(%hu,%hu)/(%d,%d)",
		rp.width, rp.height, rp.x_offset, rp.y_offset,
		rp.x_bin, rp.y_bin);

	// send the request to the camera, this is a request for field 0 
	// field 1 has to be retrieved separately
	try {
		Request<sx_read_pixels_t>	request(
			RequestBase::vendor_specific_type,
			RequestBase::device_recipient, ccdindex,
			(uint8_t)SX_CMD_READ_PIXELS, (uint16_t)(1 << field),
			&rp);
		camera.controlRequest(&request);
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot request field: %s",
			x.what());
		throw x;
	}
#else
	sx_read_pixels_delayed_t	rpd;
	rpd.width = m26c.frame.size.width;
	rpd.height = m26c.frame.size.height;
	rpd.x_offset = m26c.frame.origin.x;
	rpd.y_offset = m26c.frame.origin.y;
	rpd.x_bin = m26c.mode.getX();
	rpd.y_bin = m26c.mode.getY();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request: %hux%hu@(%hu,%hu)/(%d,%d)",
		rpd.width, rpd.height, rpd.x_offset, rpd.y_offset,
		rpd.x_bin, rpd.y_bin);
	rpd.delay = 1;

	// send the request to the camera, this is a request for field 0 
	// field 1 has to be retrieved separately
	try {
		Request<sx_read_pixels_delayed_t>	request(
			RequestBase::vendor_specific_type,
			RequestBase::device_recipient, ccdindex,
			(uint8_t)SX_CMD_READ_PIXELS_DELAYED,
			CCD_FLAGS_NOCLEAR_FRAME | (uint16_t)(1 << field),
			&rpd);
		camera.controlRequest(&request);
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot request field: %s",
			x.what());
		throw x;
	}
#endif
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure %s requested",
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
 * \brief Retrieve an image from the caemra
 *
 * This method completes the exposure on the main ccd and reads the field.
 * Depending on the exposure time, it then either starts a new exposure
 * (for short exposures, because the second field would otherwise be too
 * different), or reads out the already exposed second field (for long
 * exposures). In the latter case, the first field is rescaled to account
 * for the different exposure time.
 */
ImagePtr	SxCcdM26C::getImage() throw (not_implemented) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get an image from the camera");

	// read the right number of pixels from the IN endpoint
	Field	*field0 = readField();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "field 0 transferred");
	if (debuglevel == LOG_DEBUG) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "writing field 0");
		std::ofstream	out("field0.raw", std::ofstream::binary);
		out << *field0;
		out.close();
	}


	// for long exposures, we just read the second field.
	Field	*field1 = NULL;
	if (exposure.exposuretime > EXPOSURE_FIELD_CUTOVER) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "request second field 1");
		requestField(1);
		timer.end();
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "expose second field 1");
		exposeField(1);
	}

	// read the second field
	debug(LOG_DEBUG, DEBUG_LOG, 0, "read field 1");
	field1 = readField();
	if (debuglevel == LOG_DEBUG) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "writing field 1");
		std::ofstream	out("field1.raw", std::ofstream::binary);
		out << *field1;
		out.close();
	}

	// rescale the first field, if we did only one exposure
	if (exposure.exposuretime > EXPOSURE_FIELD_CUTOVER) {
		// rescale the field. We have to multiply the first field
		// with the scaling factor, because of overflows
		field0->rescale(timer.elapsed() / exposure.exposuretime);
	}

	// prepare a new image, this now needs binned pixels
	Image<unsigned short>	*image = new Image<unsigned short>(
		exposure.frame.size.width / exposure.mode.getX(),
		exposure.frame.size.height /exposure.mode.getY());

	// now we have to demultiplex the two fields
	debug(LOG_DEBUG, DEBUG_LOG, 0, "demultiplex the fields");
	if (1 == exposure.mode.getX()) {
		DemuxerUnbinned	demuxer;
		demuxer(*image, *field0, *field1);
	} else {
		DemuxerBinned	demuxer;
		demuxer(*image, *field0, *field1);
	}

	// remove the data
	delete field0;
	delete field1;

	// return the demultiplexed image
	return ImagePtr(image);
}

/**
 * \brief Destroy the CCD object.
 */
SxCcdM26C::~SxCcdM26C() {
}

} // namespace sx
} // namespace camera
} // namespace astro
