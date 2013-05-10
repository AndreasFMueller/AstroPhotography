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

#define	STAR2000_PORT		(1 << 0)
#define COMPRESSED_PIXEL_FORMAT	(1 << 1)
#define EEPROM			(1 << 2)
#define	INTEGRATED_GUIDER	(1 << 3)
#define REGULATED_COOLER	(1 << 4)
/*
 * the last constant is not contained in the official documentation,
 * it is a meaning of that by conjectured in an email from Terry Platt, 
 * <tplatt@starlight.win-uk.net>, may 4, 2013
 */

/**
 * \brief Create a new Camera from a USB device pointer
 *
 * The constructor has the side effect of claiming the data interface of
 * the camera. As we are doing multiple Bulk-Transfers during the lifetime
 * of the Camera object, it does not make sense to only claim and
 * release the interface when we need it. However this means that no other
 * instance of the camera object can access the camera (one can also consider
 * this a feature, not a bug). And the destructor absolutely must release
 * interface.
 * \param _deviceptr	USB device pointer
 */
SxCamera::SxCamera(DevicePtr& _deviceptr) : deviceptr(_deviceptr) {
	// the default is to use the 
	useControlRequests = true;

	// make sure the device is open
	deviceptr->open();
	product = deviceptr->descriptor()->idProduct();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "product = %04x", product);

	// now get the data interface ...
	ConfigurationPtr	conf = deviceptr->activeConfig();
	std::cout << *conf;
	interface = (*conf)[0];

	// and also claim it, we will need it all the time
	try {
		interface->claim();
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cannot claim the data interface: %s", x.what());
		throw x;
	}

	// ... and endpoint
	InterfaceDescriptorPtr	ifdesc = (*interface)[0];
	EndpointDescriptorPtr	endpoint0 = (*ifdesc)[0];
	EndpointDescriptorPtr	endpoint1 = (*ifdesc)[1];
	if (endpoint0->isIN()) {
		inendpoint = endpoint0;
		outendpoint = endpoint1;
	} else {
		inendpoint = endpoint1;
		outendpoint = endpoint0;
	}
	if (debuglevel >= LOG_DEBUG) {
		std::cout << "IN endpoint:" << std::endl;
		std::cout << *inendpoint;
		std::cout << "OUT endpoint:" << std::endl;
		std::cout << *outendpoint;
	}

	// learn the firmware version
	Request<sx_firmware_version_t>	versionrequest(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient, (uint16_t)0,
		(uint8_t)SX_CMD_GET_FIRMWARE_VERSION, (uint16_t)0);
	controlRequest(&versionrequest);
	firmware_version = *versionrequest.data();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "firmware version: %d.%d",
		firmware_version.major_version, firmware_version.minor_version);

	// learn the model number
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get model number");
	Request<sx_camera_model_t>      modelrequest(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient, (uint16_t)0,
		(uint8_t)SX_CMD_CAMERA_MODEL, (uint16_t)0);
	controlRequest(&modelrequest);
	model = modelrequest.data()->model;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "model = %04x", model);

	// get information about this CCD from the camera
        Request<sx_ccd_params_t>        ccd0request(
                RequestBase::vendor_specific_type,
                RequestBase::device_recipient, (uint16_t)0,
                (uint8_t)SX_CMD_GET_CCD_PARAMS, (uint16_t)0);
        controlRequest(&ccd0request);
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

	// find out whether this camera has a cooler
	if (ccd0request.data()->extra_capabilities & REGULATED_COOLER) {
		hasCooler = true;
	} else {
		hasCooler = false;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera has cooler: %s",
		(hasCooler) ? "yes" : "no");

	// try to get the same information from the second CCD, if there
	// is one
	if (ccd0request.data()->extra_capabilities & INTEGRATED_GUIDER) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "create Tracking CCD info");
		// get information about this CCD from the camera
		Request<sx_ccd_params_t>        ccd1request(
			RequestBase::vendor_specific_type,
			RequestBase::device_recipient, (uint16_t)1,
			(uint8_t)SX_CMD_GET_CCD_PARAMS, (uint16_t)0);
		controlRequest(&ccd1request);
		params = *ccd1request.data();

		CcdInfo	ccd1;
		ccd1.size = ImageSize(params.width, params.height);
		ccd1.name = "Tracking";
		ccd1.binningmodes.push_back(Binning(2,2));
		ccdinfo.push_back(ccd1);
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no tracking ccd");
	}
}

/**
 * \brief Destructor for the camera class.
 *
 * This method releases the data interface of the camera.
 */
SxCamera::~SxCamera() {
	try {
		interface->release();
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot release: %s", x.what());
	}
}

/**
 * \brief Get a CCD
 *
 * \param ccdindex	Index of the CCD to open.
 * \return A smart pointer to a CCD
 */
CcdPtr	SxCamera::getCcd(size_t ccdindex) {
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
 * \brief Get the data IN endpoint
 *
 * Note that we don't need the OUT endpoint, because that is only needed
 * to send commands. For commands we have the controlRequest method of
 * the camera object, which does everything for us, and does have direct
 * access to the endpoints.
 */
EndpointDescriptorPtr	SxCamera::getEndpoint() {
	return inendpoint;
}

/**
 * \brief Get the data interface of the camera
 */
InterfacePtr	SxCamera::getInterface() {
	return interface;
}

/**
 * \brief Control requests.
 *
 * The Starlight Express documentation says that all commands can be sent
 * to the control interface or the out endpoint. But at least for the
 * M26C camera, this seems not to be true, the READ_PIXELS command seems
 * to hang the camera, and other commands seem not to work correctly. So 
 * We cannot use the controlRequest method of the USBDevice, but must
 * rather reimplement control request handling via the bulk endpoints.
 */
void	SxCamera::controlRequest(RequestBase *request) {
	if (useControlRequests) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "using control interface");
		deviceptr->controlRequest(request);
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "control request %p on data interface",
		request);

	// we first analyse whether this is a control request with a
	// in data phase, because then the packet size to send is just
	// the request header, end there will an additional transfer from
	// the IN endpoint
	size_t	receivelength = (request->bmRequestType() & RequestBase::device_to_host) ? request->wLength() : 0;
	size_t	sendlength = (receivelength) ? sizeof(usb_request_header_t) : request->wLength();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request size send: %d, receive %d",
		sendlength, receivelength);

	// send phase
	debug(LOG_DEBUG, DEBUG_LOG, 0, "preparing OUT transfer: %p",
		request->getPacket());
	BulkTransfer	out(outendpoint, sendlength,
		(unsigned char *)request->getPacket());
	if (0 == receivelength) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "request payload:\n%s",
			request->payloadHex().c_str());
	}
	deviceptr->submit(&out);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "OUT transfer complete");

	// if there is no IN data phase, we are done
	if (0 == receivelength) {
		return;
	}

	// optional receive phase of the control request
	debug(LOG_DEBUG, DEBUG_LOG, 0, "preparing IN transfer");
	BulkTransfer	in(inendpoint, receivelength, request->payload());
	deviceptr->submit(&in);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "IN transfer complete:\n%s",
		request->payloadHex().c_str());
}

/**
 * \brief Get the cooler for this camera, if it exists.
 */
CoolerPtr	SxCamera::getCooler(int ccdindex) {
	if (ccdindex > 0) {
		throw std::runtime_error("only imaging CCD has cooler");
	}
	if (!hasCooler) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "this camera has no cooler");
		throw std::runtime_error("this camera has no cooler");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating cooler object");
	return CoolerPtr(new SxCooler(*this));
}

} // namespace sx
} // namespace camera
} // namespace astro
