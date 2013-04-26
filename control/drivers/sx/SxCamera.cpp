/*
 * SxCamera.cpp -- Starlight Express Camera Implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SxCamera.h>
#include <SxCcd.h>
#include <sx.h>
#include <debug.h>

using namespace astro::image;

namespace astro {
namespace camera {
namespace sx {

#define	SX_MODEL_M26C	0x005a
#define SX_PRODUCT_M26C	0x0326

/**
 * \brief Create a new Camera from a USB device pointer
 *
 * \param _deviceptr	USB device pointer
 */
SxCamera::SxCamera(DevicePtr& _deviceptr) : deviceptr(_deviceptr) {
	// make sure the device is open
	deviceptr->open();
	product = deviceptr->descriptor()->idProduct();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "product = %04x", product);

	// XXX find out how many ccds this device has
	numberCcds = 1;

	// learn the model number
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get model number");
	Request<sx_camera_model_t>      request(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient, (uint16_t)0,
		(uint8_t)SX_CMD_CAMERA_MODEL, (uint16_t)0);
	deviceptr->controlRequest(&request);
	model = request.data()->model;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "model = %04x", model);

	// 
}

SxCamera::~SxCamera() {
}

/**
 * \brief Get a CCD
 *
 * \param ccdindex	Index of the CCD to open.
 * \return A smart pointer to a CCD
 */
CcdPtr	SxCamera::getCcd(int ccdindex) {
	if (ccdindex >= numberCcds) {
		debug(LOG_ERR, DEBUG_LOG, 0, "ccd id %d out of range",
			ccdindex);
		throw std::range_error("ccd id out of range");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get ccd with index %d", ccdindex);

	// get information about this CCD from the camera
        Request<sx_ccd_params_t>        request(
                RequestBase::vendor_specific_type,
                RequestBase::device_recipient, ccdindex,
                (uint8_t)SX_CMD_GET_CCD_PARAMS, (uint16_t)0);
        deviceptr->controlRequest(&request);
	sx_ccd_params_t	params = *request.data();

	// now use this data to construct the CCD
	ImageSize	size(params.width, params.height);
	if (model == SX_MODEL_M26C) {
		size.height *= 2;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "size: %dx%d", size.width, size.height);
	return CcdPtr(new SxCcd(size, *this, ccdindex));
}

/**
 * \brief Get the device pointer
 */
DevicePtr	SxCamera::getDevicePtr() {
	return deviceptr;
}

} // namespace sx
} // namespace camera
} // namespace astro
