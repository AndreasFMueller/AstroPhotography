/*
 * SxCamera.cpp -- Starlight Express Camera Implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
//#include <AstroDevice.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroExceptions.h>
#include "SxCamera.h"
#include "SxCcd.h"
#include "sx.h"
#include "SxGuidePort.h"
#include "SxUtils.h"
#include "SxCooler.h"
#include <sstream>

using namespace astro::image;

namespace astro {
namespace camera {
namespace sx {

#define	SX_MODEL_M26C	0x005a
#define SX_MODEL_56	0x0021
#define SX_MODEL_46	0x0022
#define SX_PRODUCT_M26C	0x0326

#define	STAR2000_PORT		(1 << 0)
#define COMPRESSED_PIXEL_FORMAT	(1 << 1)
#define EEPROM			(1 << 2)
#define	INTEGRATED_GUIDER	(1 << 3)
#define REGULATED_COOLER	(1 << 4)
#define HAS_SHUTTER		(1 << 5)
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

#define	NUMBER_SX_MODELS	40
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
	{ 0x0326, SX_MODEL_M26C, std::string("SXVR-M26C")     },
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
	{ 0x0000, SX_MODEL_56, std::string("SX-56")         },
	{ 0x0000, SX_MODEL_46, std::string("SX-46")         },
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
 * interface. When the constructor is called, the DevicePtr argument 
 * must refer to an open device.
 *
 * \param _deviceptr	USB device pointer
 */
SxCamera::SxCamera(DevicePtr& _deviceptr)
	: Camera(SxName(_deviceptr).cameraname()), deviceptr(_deviceptr) {
	// the default is to send requests over the data end point
	// XXX here we use the USB control requests
	useControlRequests = true;

	// make sure camera is not busy
	_busy = false;

	// find the product id
	product = deviceptr->descriptor()->idProduct();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "product = %04x", product);

	// now get the data interface ...
	ConfigurationPtr	conf = deviceptr->activeConfig();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", conf->toString().c_str());
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
		debug(LOG_DEBUG, DEBUG_LOG, 0, "IN endpoint:");
		debug(LOG_DEBUG, DEBUG_LOG, 0, inendpoint->toString().c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "OUT endpoint:");
		debug(LOG_DEBUG, DEBUG_LOG, 0, outendpoint->toString().c_str());
	}

	// reset the camera, just for good measure
	EmptyRequest    resetrequest(
                        RequestBase::vendor_specific_type,
                        RequestBase::device_recipient, (uint16_t)0,
                        (uint8_t)SX_CMD_RESET, (uint16_t)0);
	controlRequest(&resetrequest);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "reset the camera");

	// learn the firmware version
	Request<sx_firmware_version_t>	versionrequest(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient, (uint16_t)0,
		(uint8_t)SX_CMD_GET_FIRMWARE_VERSION, (uint16_t)0);
	controlRequest(&versionrequest);
	firmware_version = *versionrequest.data();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "firmware version: %d.%d",
		firmware_version.major_version, firmware_version.minor_version);

	// get the build number
	build_number = 0xffff;
	try {
		Request<sx_build_number_t>	buildnumberrequest(
			RequestBase::vendor_specific_type,
			RequestBase::device_recipient, (uint16_t)0,
			(uint8_t)SX_CMD_GET_BUILD_NUMBER, (uint16_t)0);
		controlRequest(&buildnumberrequest);
		build_number = buildnumberrequest.data()->build_number;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "build_number: %d",
			build_number);
		goto gotbuildnumber;
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "long build number worked");
	}
	try {
		Request<sx_short_build_number_t>	buildnumberrequest(
			RequestBase::vendor_specific_type,
			RequestBase::device_recipient, (uint16_t)0,
			(uint8_t)SX_CMD_GET_BUILD_NUMBER, (uint16_t)0);
		controlRequest(&buildnumberrequest);
		build_number = buildnumberrequest.data()->build_number;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "build_number: %d",
			build_number);
		goto gotbuildnumber;
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "short build number worked");
	}
	build_number = 0;
gotbuildnumber:
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got build number: %04x", build_number);

	// learn the model number
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get model number");
	Request<sx_camera_model_t>      modelrequest(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient, (uint16_t)0,
		(uint8_t)SX_CMD_CAMERA_MODEL, (uint16_t)0);
	controlRequest(&modelrequest);
	model = modelrequest.data()->model;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "model = %04x", model);

	// find out whether this is a model with an interline CCD
	_has_interline_ccd = (0x10 == (0x7f & model));

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
	DeviceName	ccd0name = CcdInfo::defaultname(name(), "Imaging");
	CcdInfo	ccd0(ccd0name, ImageSize(width, height), 0);
	ccd0.addMode(Binning(2,2));
	if (model != SX_MODEL_M26C) {
		ccd0.addMode(Binning(3,3));
		ccd0.addMode(Binning(4,4));
	}

	// set pixel width and height
	debug(LOG_DEBUG, DEBUG_LOG, 0, "params.pixel_uwidth = %hu",
		params.pixel_uwidth);
	ccd0.pixelwidth((float)params.pixel_uwidth / (256. * 1000000));
	ccd0.pixelheight((float)params.pixel_uheight / (256. * 1000000));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "pixel size: %.2fum x %.2fum",
		1000000 * ccd0.pixelwidth(), 1000000 * ccd0.pixelheight());

	// exposure times
	ccd0.minexposuretime(0.001);
	ccd0.maxexposuretime(3600);

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
		_hasGuidePort = true;
	} else {
		_hasGuidePort = false;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera has guider port: %s",
		(_hasGuidePort) ? "yes" : "no");

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

		DeviceName	ccd1name = CcdInfo::defaultname(name(),
					"Tracking");
		CcdInfo	ccd1(ccd1name,
			ImageSize(params.width, params.height), 1);
		ccd1.addMode(Binning(2,2));
		ccdinfo.push_back(ccd1);
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no tracking ccd");
	}

	// find out whether there is a shutter
	if (ccd0request.data()->extra_capabilities & HAS_SHUTTER) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "has shutter");
		ccd0.shutter(true);
	} else {
		ccd0.shutter(false);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera has shutter: %s",
		(ccd0.shutter()) ? "yes" : "no");

	// add the CCDinfo to the ccdinfo array
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Imaging CCD: %s",
		ccd0.toString().c_str());
	ccdinfo.push_back(ccd0);

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
 * \brief get the user friendly name
 */
std::string	SxCamera::userFriendlyName() const {
	// try to match model and product
	for (int i = 0; i < NUMBER_SX_MODELS; i++) {
		if ((product == models[i].product)
			&& (model == models[i].model)) {
			return models[i].name;
		}
	}

	// try to match model alone, at least if model != 0
	if (model != 0) {
		for (int i = 0; i < NUMBER_SX_MODELS; i++) {
			if (model == models[i].model) {
				return models[i].name;
			}
		}
	}

	// try to match product alone
	if (product != 0) {
		for (int i = 0; i < NUMBER_SX_MODELS; i++) {
			if (product == models[i].product) {
				return models[i].name;
			}
		}
	}

	return Device::userFriendlyName();
}

/**
 * \brief Reset the SxCamera
 */
void	SxCamera::reset() {
	EmptyRequest	resetrequest(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient, (uint16_t)0,
		(uint8_t)SX_CMD_RESET, (uint16_t)0);
	controlRequest(&resetrequest);
}

/**
 * \brief Get a CCD
 *
 * \param ccdindex	Index of the CCD to open.
 * \return A smart pointer to a CCD
 */
CcdPtr	SxCamera::getCcd0(size_t ccdindex) {
	if (ccdindex >= nCcds()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "ccd id %d out of range",
			ccdindex);
		throw NotFound("ccd id out of range");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get ccd with index %d", ccdindex);
	if ((model == SX_MODEL_M26C) && (ccdindex == 0)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "create SxCcdM26C for the M26C "
			"imaging CCD: %s",
			ccdinfo[ccdindex].toString().c_str());
		return CcdPtr(new SxCcdM26C(ccdinfo[ccdindex], *this,
			ccdindex));
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "create ordinary SX ccd: %s",
		ccdinfo[ccdindex].toString().c_str());
	SxCcd	*ccd = new SxCcd(ccdinfo[ccdindex], *this, ccdindex);

	// return the CCD pointer
	return CcdPtr(ccd);
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
void	SxCamera::controlRequest(RequestBase *request,
		bool asUSBControlRequest) {
	//request->setTimeout(request->getTimeout() + 60000);
	if (request->getTimeout() <= 1000) {
		request->setTimeout(30000);
	}
	if (asUSBControlRequest) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "using control interface, "
			"request with timeout %d", request->getTimeout());
		deviceptr->controlRequest(request);
		return;
	}

	// Performing request over the data OUTPOINT
	sx_command_t	command = (sx_command_t)request->bRequest();
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"control request for command '%s' on data interface, "
		"request = %02x, requesttype = %02x,  wValue = %04x, "
		"wIndex = %04x, wLength = %04x",
		command_name(command).c_str(),
		request->bRequest(), request->bmRequestType(),
		request->wValue(), request->wIndex(), request->wLength());
	request->setTimeout(10000);

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
	out.setTimeout(request->getTimeout());
	if (0 == receivelength) {
		if (request->wLength() > 0) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "request payload:\n%s",
				request->payloadHex().c_str());
		} else {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "no request payload");
		}
	}
	try {
		deviceptr->submit(&out);
	} catch (USBError& x) {
		std::string	msg = stringprintf(
			"SX OUT(%d) transfer error: %s", sendlength, x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw DeviceTimeout(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "OUT transfer complete");

	// if there is no IN data phase, we are done
	if (0 == receivelength) {
		return;
	}

	// optional receive phase of the control request
	debug(LOG_DEBUG, DEBUG_LOG, 0, "preparing IN transfer");
	BulkTransfer	in(inendpoint, receivelength, request->payload());
	in.setTimeout(request->getTimeout());
	try {
		deviceptr->submit(&in);
	} catch (USBError& x) {
		std::string	msg = stringprintf(
			"SX IN(%d) transfer error: %s", receivelength, x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw DeviceTimeout(msg);
	}
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
		throw NotImplemented("only imaging CCD has cooler");
	}
	if (!_hasCooler) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "this camera has no cooler");
		throw NotImplemented("this camera has no cooler");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating cooler object");
	return CoolerPtr(new SxCooler(*this));
}

/**
 * \brief Get the guider port
 */
GuidePortPtr	SxCamera::getGuidePort0() {
	if (!_hasGuidePort) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "this camera has no guide port");
		throw NotImplemented("this camera has no guider port");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating guider port object");
	return GuidePortPtr(new SxGuidePort(*this));
}

/**
 * \brief Find out whether this is a color camera
 */
bool	SxCamera::isColor() const {
	return ((product & 0xf00) == 0x300) ? true : false;
}

/**
 * \brief Find out whether the device is busy
 */
bool	SxCamera::busy() {
	std::unique_lock<std::recursive_mutex>	lock(mutex);
	return _busy;
}

/**
 * \brief Reserve the device
 *
 * Any method that does an USB operation must reserve the device before
 * it initiates the operation, and release it when it completes.
 */
bool	SxCamera::reserve(const std::string& purpose, int timeout) {
	std::unique_lock<std::recursive_mutex>	lock(mutex);
	if (_busy && (purpose == _purpose)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"RESERVE already reserved for '%s'", purpose.c_str());
		return true;
	}
	if (!_busy) {
		_busy = true;
		_purpose = purpose;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "RESERVE camera reserved: '%s'",
			purpose.c_str());
		return true;
	}
	std::cv_status	status
		= condition.wait_for(lock, std::chrono::milliseconds(timeout));
	if (status == std::cv_status::timeout) {
		return false;
	}
	// we now own the lock and can change the busy flag
	if (!_busy) {
		_busy = true;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "RESERVE camera reserved: '%s'",
			purpose.c_str());
		return true;
	}
	
	// we should not get to this point, because that indicates a logic
	// error. After we receive a signal, the device should be free
	return false;
}

/**
 * \brief Release the device
 *
 * This signals all waiting threads that the operation has completed and
 * allows them to continue
 */
void	SxCamera::release(const std::string& purpose) {
	std::unique_lock<std::recursive_mutex>	lock(mutex);
	if (_busy == false) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"RESERVE cannot release '%s', already released",
			purpose.c_str());
	}
	if (purpose != _purpose) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"RESERVE wrong purpose: '%s' != '%s'",
			purpose.c_str(), _purpose.c_str());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "RESERVE camera released: '%s'",
		_purpose.c_str());
	_busy = false;
	_purpose = "";
	condition.notify_all();
}

bool	SxCamera::hasRBIFlood() const {
	return ((model == SX_MODEL_56) && (model == SX_MODEL_56));
}

} // namespace sx
} // namespace camera
} // namespace astro
