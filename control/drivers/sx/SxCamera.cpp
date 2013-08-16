/*
 * SxCamera.cpp -- Starlight Express Camera Implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SxCamera.h>
#include <SxCcd.h>
#include <sx.h>
#include <AstroDebug.h>
#include <SxGuiderPort.h>
#include <AstroFormat.h>

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
typedef struct sx_model_s {
	unsigned short	product;
	unsigned short	model;
	std::string	name;
} sx_model_t;

#define	NUMBER_SX_MODELS	38
sx_model_t	models[NUMBER_SX_MODELS] = {
	{ 0x0105, 0x0045, std::string("SXVF-M5")       },
	{ 0x0305, 0x00c5, std::string("SXVF-M5C")      },
	{ 0x0107, 0x0047, std::string("SXVF-M7")       },
	{ 0x0307, 0x00c7, std::string("SXVF-M7C")      },
	{ 0x0000, 0x0048, std::string("SXVF-M8")       },
	{ 0x0308, 0x00c8, std::string("SXVF-M8C")      },
	{ 0x0109, 0x0049, std::string("MX9")           },
	{ 0x0109, 0x0000, std::string("SXVF-M9")       },
	{ 0x0309, 0x00c9, std::string("MX9C")          },
	{ 0x0509, 0x0009, std::string("Oculus")        },
	{ 0x0325, 0x0059, std::string("SXVR-M25C")     },
	{ 0x0326, 0x005a, std::string("SXVR-M26C")     },
	{ 0x0128, 0x0000, std::string("SXVR-H18")      },
	{ 0x0126, 0x0000, std::string("SXVR-H16")      },
	{ 0x0135, 0x0023, std::string("SXVR-H35")      },
	{ 0x0135, 0x00b3, std::string("SXVR-H35C")     },
	{ 0x0136, 0x0024, std::string("SXVR-H36")      },
	{ 0x0136, 0x00b4, std::string("SXVR-H36C")     },
	{ 0x0100, 0x0009, std::string("SXVR-H9")       },
	{ 0x0119, 0x0009, std::string("SXVR-H9")       },
	{ 0x0319, 0x0089, std::string("SXVR-H9C")      },
	{ 0x0100, 0x0089, std::string("SXVR-H9C")      },
	{ 0x0200, 0x0000, std::string("SXV interface") },
	{ 0x0507, 0x0000, std::string("Lodestar")      },
	{ 0x0507, 0x0000, std::string("Lodestar-C")    },
	{ 0x0517, 0x0000, std::string("CoStar")        },
	{ 0x0000, 0x0009, std::string("HX9")           },
	{ 0x0000, 0x0010, std::string("SXVR-H16")      },
	{ 0x0000, 0x0090, std::string("SXVR-H16C")     },
	{ 0x0000, 0x0012, std::string("SXVR-H18")      },
	{ 0x0000, 0x0092, std::string("SXVR-H18C")     },
	{ 0x0000, 0x0056, std::string("SXVR-H674")     },
	{ 0x0000, 0x00b6, std::string("SXVR-H674C")    },
	{ 0x0000, 0x0057, std::string("SXVR-H694")     },
	{ 0x0000, 0x00b7, std::string("SXVR-H694C")    },
	{ 0x0000, 0x0028, std::string("SXVR-H814")     },
	{ 0x0000, 0x00a8, std::string("SXVR-H814C")    },
	{ 0x0000, 0x0058, std::string("SXVR-H290")     },
};

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
	useControlRequests = false;

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

	// from product id and model number, try to infer the product name
	for (unsigned int m = 0; m < NUMBER_SX_MODELS; m++) {
		if (
		((models[m].product == 0) || (models[m].product == product)) &&
		((models[m].model == 0) || (models[m].model == model))
		) {
			_name = models[m].name;
			break;
		}
	}

	// get information about this CCD from the camera
        Request<sx_ccd_params_t>        ccd0request(
                RequestBase::vendor_specific_type,
                RequestBase::device_recipient, (uint16_t)0,
                (uint8_t)SX_CMD_GET_CCD_PARAMS, (uint16_t)0);
        controlRequest(&ccd0request);
	sx_ccd_params_t	params = *ccd0request.data();

	// now create a CcdInfo structure for this device
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create Imaging CCD info");
	unsigned int	width = params.width;
	unsigned int	height = params.height;
	if (model == SX_MODEL_M26C) {
		height *= 2;
	}
	CcdInfo	ccd0("Imaging", ImageSize(width, height), 0);
	ccd0.addMode(Binning(2,2));
	if (model != SX_MODEL_M26C) {
		ccd0.addMode(Binning(3,3));
		ccd0.addMode(Binning(4,4));
	}
	ccdinfo.push_back(ccd0);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Imaging CCD: %s",
		ccd0.toString().c_str());

	// find out whether this camera has a cooler
	if (ccd0request.data()->extra_capabilities & REGULATED_COOLER) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "has cooler");
		_hasCooler = true;
	} else {
		_hasCooler = false;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera has cooler: %s",
		(_hasCooler) ? "yes" : "no");

	// find out whether this camera has a guider port
	if (ccd0request.data()->extra_capabilities & STAR2000_PORT) {
		hasGuiderPort = true;
	} else {
		hasGuiderPort = false;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera has guider port: %s",
		(hasGuiderPort) ? "yes" : "no");

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

		CcdInfo	ccd1("Tracking",
			ImageSize(params.width, params.height), 1);
		ccd1.addMode(Binning(2,2));
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "control request %p on data interface, "
		"request = %02x, requesttype = %02x,  wValue = %04x, "
		"wIndex = %04x, wLength = %04x",
		request,
		request->bRequest(), request->bmRequestType(),
		request->wValue(), request->wIndex(), request->wLength());

	// we first analyse whether this is a control request with a
	// in data phase, because then the packet size to send is just
	// the request header, end there will an additional transfer from
	// the IN endpoint
	size_t	receivelength = (request->bmRequestType()
		& RequestBase::device_to_host) ? request->wLength() : 0;
	size_t	sendlength = sizeof(usb_request_header_t);
	if (0 == receivelength) {
		sendlength += request->wLength();
	}
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
 * \brief Ask whether this camera has a cooler
 */
bool	SxCamera::hasCooler() {
	return _hasCooler;
}

/**
 * \brief Get the cooler for this camera, if it exists.
 */
CoolerPtr	SxCamera::getCooler(int ccdindex) {
	if (ccdindex > 0) {
		throw std::runtime_error("only imaging CCD has cooler");
	}
	if (!_hasCooler) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "this camera has no cooler");
		throw std::runtime_error("this camera has no cooler");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating cooler object");
	return CoolerPtr(new SxCooler(*this));
}

/**
 * \brief Get the guider port
 */
GuiderPortPtr	SxCamera::getGuiderPort0() throw (not_implemented) {
	if (!hasGuiderPort) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "this camera has no guide port");
		throw std::runtime_error("this camera has no guider port");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating guider port object");
	return GuiderPortPtr(new SxGuiderPort(*this));
}

/**
 * \brief Find out whether this is a color camera
 */
bool	SxCamera::isColor() const {
	return ((product & 0xf00) == 0x300) ? true : false;
}

} // namespace sx
} // namespace camera
} // namespace astro
