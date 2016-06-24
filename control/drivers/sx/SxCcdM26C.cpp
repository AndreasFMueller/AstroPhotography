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
#include <AstroDebug.h>
#include <AstroExceptions.h>
#include <AstroFormat.h>
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "compute the exposure parameters for "
		"the M26C's CCD chip");
	Exposure	m26c = exposure;

	// adapt the size suitable for the M26C chip
	unsigned int	height = exposure.width() / 4;
	unsigned int	width = exposure.height() * 2;
	if (m26c.mode().x() > 1) {
		height -=  height % 2;
	}
	ImageSize	m26c_size(width, height);
	//m26c.frame.setSize(m26c_size);

	// the integer arithmetic may have changed some parameters in the
	// m26c object, so we have to recompute the size the client will see
	ImageSize	exposure_size(m26c_size.height() * 4,
					m26c_size.width() / 2);

	// adapt the top left corner
	int	originy = exposure.frame().origin().x() / 4;
	int	originx = exposure.frame().origin().y() * 2;
	if (m26c.mode().y() > 1) {
		originy -= originy % 2;
	}
	ImagePoint	m26c_origin(originx, originy);
	ImageRectangle	m26c_frame(m26c_origin, m26c_size);
	m26c.frame(m26c_frame);

	// fix the origin too
	ImagePoint	exposure_origin(m26c_origin.y() * 4,
					m26c_origin.x() / 2);
	ImageRectangle	exposure_frame(exposure_origin, exposure_size);
	exposure.frame(exposure_frame);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "m26c specific exposure: %s",
		m26c.toString().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "public exposure: %s",
		exposure.toString().c_str());

	// copy stuff that is not affected
	m26c.mode(exposure.mode());
	m26c.exposuretime(exposure.exposuretime());
	m26c.gain(1.);

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
	size_t	l = (m26c.frame().size() / m26c.mode()).getPixels();
	Field	*field = new Field(exposure.frame().size(), l);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "transfer field of size %u", l);

	// perform the data transfer
	try {
		BulkTransfer	transfer(camera.getEndpoint(), (int)l * 2,
			(unsigned char *)field->data);
		int	timeout = exposure.exposuretime() * 1100
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request exposure of a field %d", field);
	// compute a better request for the M26C camera
	sx_read_pixels_delayed_t	rpd;
	rpd.delay = 1000 * m26c.exposuretime();
	rpd.width = m26c.width();
	rpd.height = m26c.height();
	rpd.x_offset = m26c.x();
	rpd.y_offset = m26c.y();
	rpd.x_bin = m26c.mode().x();
	rpd.y_bin = m26c.mode().y();
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"request: %hux%hu@(%hu,%hu)/(%d,%d), t=%ums",
		rpd.width, rpd.height, rpd.x_offset, rpd.y_offset,
		rpd.x_bin, rpd.y_bin, rpd.delay);

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
		std::string	msg = stringprintf("cannot request field: %s",
						x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw DeviceProtocolException(msg);
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "requesting field %d", field);
	// prepare a request for the pixels, without delay, this just
	// downloads the already exposed field
#if 1
	sx_read_pixels_t	rp;
	rp.width = m26c.width();
	rp.height = m26c.height();
	rp.x_offset = m26c.x();
	rp.y_offset = m26c.y();
	rp.x_bin = m26c.mode().x();
	rp.y_bin = m26c.mode().y();
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
		std::string	msg = stringprintf("cannot request field: %s",
					x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw DeviceProtocolException(msg);
	}
#else
	sx_read_pixels_delayed_t	rpd;
	rpd.width = m26c.frame.size.width();
	rpd.height = m26c.frame.size.height();
	rpd.x_offset = m26c.frame.origin.x();
	rpd.y_offset = m26c.frame.origin.y();
	rpd.x_bin = m26c.mode.x();
	rpd.y_bin = m26c.mode.y();
	rpd.delay = 1;
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"request: %hux%hu@(%hu,%hu)/(%d,%d), delay = %ul",
		rpd.width, rpd.height, rpd.x_offset, rpd.y_offset,
		rpd.x_bin, rpd.y_bin, rpd.delay);

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
		std::string	msg = stringprintf("cannot request field: %s",
					x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw DeviceProtocolException(msg);
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
	x[0] = exp.x();
	y[0] = exp.y();
	x[1] = M26C_WIDTH - x[0];
	y[1] = M26C_HEIGHT - y[0];
	x[2] = exp.x() + exp.width();
	y[2] = exp.y() + exp.height();
	x[3] = M26C_WIDTH - x[2];
	y[3] = M26C_HEIGHT - y[2];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "x[] = %d %d %d %d",
		x[0], x[1], x[2], x[3]);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "y[] = %d %d %d %d",
		y[0], y[1], y[2], y[3]);

	// symmetrized origin
	ImagePoint	origin(min(x, 4), min(y, 4));
	
	// symmetrize size
	unsigned int	sizex = max(x, 4) - symexp.x();
	if (sizex > 3900) {
		sizex = 3900;
	}
	unsigned int	sizey = max(y, 4) - symexp.y();
	ImageSize	size(sizex, sizey);
	ImageRectangle	frame(origin, size);
	symexp.frame(frame);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "symmetrized exposure: %s",
		symexp.toString().c_str());
	return symexp;
}

/**
 * \brief Start an exposure on the M26C camera
 *
 * \param exposure	exposure structure for the exposure to perform
 */
void	SxCcdM26C::startExposure0(const Exposure& exposure) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure %s requested",
		exposure.toString().c_str());
	// remember the exposure, we need it for the second field for the
	// case where we do two fields one after the other
	this->exposure = symmetrize(exposure);

	// compute a better request for the M26C camera
	m26c = m26cExposure();

	// start the exposure
	exposeField(0);
	timer.start();

	// we are now in exposing state
	state = CcdState::exposing;
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
void	SxCcdM26C::getImage0() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get an image from the camera");
	// start the exposure
	state = CcdState::exposing;
	this->startExposure0(exposure);

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
	if (exposure.exposuretime() > EXPOSURE_FIELD_CUTOVER) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "request second field 1");
		timer.end();
		requestField(1);
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
	if (exposure.exposuretime() > EXPOSURE_FIELD_CUTOVER) {
		//double	deadtime = 3.37; // exposuretime = 11
		//double	deadtime = 3.99; // exposuretime = 20
		double	deadtime = 1.2;
		double	scalefactor
			= (timer.elapsed() - deadtime) / exposure.exposuretime();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "scalefactor = %f", scalefactor);
		// rescale the field. We have to multiply the first field
		// with the scaling factor, because of overflows
		if (scalefactor > 0) {
			field0->rescale(scalefactor);
		} else {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "no rescaling");
		}
		//field0->rescale(1.1283);
	}

	// prepare a new image, this now needs binned pixels
	Image<unsigned short>	*_image = new Image<unsigned short>(
		exposure.frame().size() / exposure.mode());
	_image->setOrigin(exposure.frame().origin());
	_image->setMosaicType(MosaicType::BAYER_RGGB);

	// now we have to demultiplex the two fields
	debug(LOG_DEBUG, DEBUG_LOG, 0, "demultiplex the fields");
	if (1 == exposure.mode().x()) {
		DemuxerUnbinned	demuxer;
		demuxer(*_image, *field0, *field1);
	} else {
		DemuxerBinned	demuxer;
		demuxer(*_image, *field0, *field1);
	}

	// remove the data
	delete field0;
	delete field1;

	// return the demultiplexed image
	image = ImagePtr(_image);
	state = CcdState::exposed;
}

/**
 * \brief Destroy the CCD object.
 */
SxCcdM26C::~SxCcdM26C() {
}

} // namespace sx
} // namespace camera
} // namespace astro
