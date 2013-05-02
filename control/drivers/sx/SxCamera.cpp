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

	// learn the model number
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get model number");
	Request<sx_camera_model_t>      modelrequest(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient, (uint16_t)0,
		(uint8_t)SX_CMD_CAMERA_MODEL, (uint16_t)0);
	deviceptr->controlRequest(&modelrequest);
	model = modelrequest.data()->model;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "model = %04x", model);

	// get information about this CCD from the camera
        Request<sx_ccd_params_t>        ccd0request(
                RequestBase::vendor_specific_type,
                RequestBase::device_recipient, (uint16_t)0,
                (uint8_t)SX_CMD_GET_CCD_PARAMS, (uint16_t)0);
        deviceptr->controlRequest(&ccd0request);
	sx_ccd_params_t	params = *ccd0request.data();

	// now create a CcdInfo structure for this device
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create Imaging CCD info");
	CcdInfo	ccd0;
	ccd0.size = ImageSize(params.width, params.height);
	ccd0.name = "Imaging";
	ccd0.binningmodes.push_back(Binning(2,2));
	if (model != SX_MODEL_M26C) {
		ccd0.binningmodes.push_back(Binning(3,3));
		ccd0.binningmodes.push_back(Binning(4,4));
	} else {
		ccd0.size.height *= 2;
	}
	ccdinfo.push_back(ccd0);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Imaging CCD: %s",
		ccd0.toString().c_str());

	// try to get the same information from the second CCD, if there
	// is one
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "create Tracking CCD info");
		// get information about this CCD from the camera
		Request<sx_ccd_params_t>        ccd1request(
			RequestBase::vendor_specific_type,
			RequestBase::device_recipient, (uint16_t)1,
			(uint8_t)SX_CMD_GET_CCD_PARAMS, (uint16_t)0);
		deviceptr->controlRequest(&ccd1request);
		params = *ccd1request.data();

		CcdInfo	ccd1;
		ccd1.size = ImageSize(params.width, params.height);
		ccd1.name = "Tracking";
		ccd1.binningmodes.push_back(Binning(2,2));
		ccdinfo.push_back(ccd1);
	} catch (std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no tracking ccd");
	}

	// now get the data interface and endpoint
	ConfigurationPtr	conf = deviceptr->activeConfig();
	std::cout << *conf;
	interface = (*conf)[0];
	InterfaceDescriptorPtr	ifdesc = (*interface)[0];
	dataendpoint = (*ifdesc)[0];
	std::cout << *dataendpoint;
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
	if (ccdindex >= nCcds()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "ccd id %d out of range",
			ccdindex);
		throw std::range_error("ccd id out of range");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get ccd with index %d", ccdindex);
	if ((model == SX_MODEL_M26C) && (ccdindex == 0)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "create SxCcdM26C for the M26C "
			"imaging CCD");
		return CcdPtr(new SxCcdM26C(ccdinfo[ccdindex], *this,
			ccdindex));
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "create ordinary SX ccd");
	return CcdPtr(new SxCcd(ccdinfo[ccdindex], *this, ccdindex));
}

/**
 * \brief Get the device pointer
 */
DevicePtr	SxCamera::getDevicePtr() {
	return deviceptr;
}

/**
 * \brief Get the data endpoint
 */
EndpointDescriptorPtr	SxCamera::getEndpoint() {
	return dataendpoint;
}

InterfacePtr	SxCamera::getInterface() {
	return interface;
}

} // namespace sx
} // namespace camera
} // namespace astro
