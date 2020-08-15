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

#define	STAR2000_PORT		(1 << 0)
#define COMPRESSED_PIXEL_FORMAT	(1 << 1)
#define EEPROM			(1 << 2)
#define	INTEGRATED_GUIDER	(1 << 3)
#define REGULATED_COOLER	(1 << 4)
#define HAS_SHUTTER		(1 << 5)

/**
 * \brief Get the model number from a device ptr
 *
 * \param deviceptr	device to query
 */
unsigned short	SxCamera::getModel(DevicePtr deviceptr) {
	if (!deviceptr) {
		std::string	msg = stringprintf("no device");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// learn the model number
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get model number");
	Request<sx_camera_model_t>      modelrequest(
		RequestBase::vendor_specific_type,
		RequestBase::device_recipient, (uint16_t)0,
		(uint8_t)SX_CMD_CAMERA_MODEL, (uint16_t)0);

	// send the request
	deviceptr->controlRequest(&modelrequest);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request was successful");
	return modelrequest.data()->model;
}

/**
 * \brief Connect to a given device
 *
 * \param _deviceptr	the device to connect to
 */
void	SxCamera::connect(DevicePtr _deviceptr) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "connect to %hu/%hu",
		_deviceptr->getVendorId(), _deviceptr->getProductId());

	// make sure we have a device
	if (!_deviceptr) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no device to specify");
		throw BadParameter("no device specified");
	}

	// make sure the new name matches the old one
	std::string	newname = SxName(DeviceName::Camera, _deviceptr)
					.toString();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "trying newname = %s", newname.c_str());
	if (name().toString() != newname) {
		std::string	msg = stringprintf("reconnect name mismatch: "
			"%s != %s", name().toString().c_str(),
			newname.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw BadParameter(msg);
	}

	// remember the device ptr
	deviceptr = _deviceptr;

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
	model = getModel(deviceptr);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "model = %04x", model);

	// find out whether this is a model with an interline CCD
	_has_interline_ccd = (0x10 == (0x7f & model));

	// remove all ccds
	ccdinfo.clear();

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
 * \brief Disconnect the camera
 *
 * \param _deviceptr	the device to connect to
 */
void	SxCamera::disconnect() {
	// make sure we have the exlusive access to the USB device
	std::unique_lock<std::recursive_mutex>	_lock(mutex);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "reconnecting SX device");
	// clean up all the out endpoint
	try {
		outendpoint.reset();
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("failed to destroy %s: %s",
			demangle(typeid(x).name()).c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	}

	// clean up all the in endpoint
	try {
		inendpoint.reset();
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("failed to destroy %s: %s",
			demangle(typeid(x).name()).c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	}

	// clean up the interface
	try {
		interface.reset();
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("failed to destroy %s: %s",
			demangle(typeid(x).name()).c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	}

	// clean up the device itself
	try {
		deviceptr.reset();
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("failed to destroy %s: %s",
			demangle(typeid(x).name()).c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	}
}

/**
 * \brief Reconnect the camera to a new USB device
 *
 * \param _deviceptr	the device to connect to
 */
void	SxCamera::reconnect(DevicePtr _deviceptr) {
	// make sure we have the exlusive access to the USB device
	std::unique_lock<std::recursive_mutex>	_lock(mutex);

	// connect to the new device
	connect(_deviceptr);
}

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
SxCamera::SxCamera(SxCameraLocator& locator, DevicePtr _deviceptr)
	: Camera(SxName(DeviceName::Camera, _deviceptr).cameraname()),
	  _locator(locator), deviceptr(_deviceptr) {
	// the default is to send requests over the data end point
	// XXX here we use the USB control requests
	useControlRequests = true;

	// make sure camera is not busy
	_busy = false;
	_purpose = "";

	// now try to connect to the device
	connect(_deviceptr);
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
	try {
		SxName::userFriendlyName(product, model);
	} catch (NotFound& x) {
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
	try {
		controlRequest(&resetrequest);
	} catch (USBError& x) {
		refresh();
		throw x;
	}
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
	if (!deviceptr) {
		std::string	msg = stringprintf("%s has no deviceptr",
			name().toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw DeviceTimeout(msg);
	}
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
	// before doing anything, check that we have a deviceptr
	if (!deviceptr) {
		refresh();
		if (!deviceptr) {
			std::string	msg = stringprintf("refresh failed");
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw DeviceTimeout(msg);
		}
	}

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
 *
 * \param purpose	the purpose for the reservation
 * \param timeout	the timeout in milliseconds to wait for the device
 *			to become free
 */
bool	SxCamera::reserve(const std::string& purpose, int timeout) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "RESERVE attempt '%s', current = '%s'",
		purpose.c_str(), _purpose.c_str());
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

	// repeat waiting until we either hit a timeout or are successful
	// at reserving the device
	while (1) {
		std::cv_status	status = condition.wait_for(lock,
					std::chrono::milliseconds(timeout));
		if (status == std::cv_status::timeout) {
			return false;
		}
		// we now own the lock and can change the busy flag
		if (!_busy) {
			_busy = true;
			_purpose = purpose;
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"RESERVE camera reserved: '%s'",
				purpose.c_str());
			return true;
		}
	}
}

/**
 * \brief Release the device
 *
 * This signals all waiting threads that the operation has completed and
 * allows them to continue
 */
void	SxCamera::release(const std::string& purpose) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "RESERVE release '%s', current = '%s'",
		purpose.c_str(), _purpose.c_str());
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

/**
 * \brief Find out whether the camera has  flood ilumination LED
 */
bool	SxCamera::hasRBIFlood() const {
	return ((model == SX_MODEL_56) && (model == SX_MODEL_56));
}

/**
 * \brief Refresh the connection
 */
void	SxCamera::refresh() {
	std::string	enclosurename = name().enclosurename();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start refresh %s",
		enclosurename.c_str());

	// release the data structures
	disconnect();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "disconnect the device");

	// forget the enclosure we are currently using
	try {
		_locator.forget(enclosurename);
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("error during forget: %s %s",
			demangle(typeid(x).name()).c_str(), x.what());
	}

	// teset the deviceptr 

	// get the device
	usb::DevicePtr	newdevptr;
	try {
		newdevptr = _locator.deviceForName(enclosurename);
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("error during "
			"deviceForName: %s %s",
			demangle(typeid(x).name()).c_str(), x.what());
	}

	// if we could not get the new pointer, we give up at this point
	if (!newdevptr) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"could not get a new connection for %s",
			enclosurename.c_str());
		return;
	}

	// reconnect to this enclosure
	try {
		reconnect(newdevptr);
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("error during reconnect: "
			"%s %s", demangle(typeid(x).name()).c_str(), x.what());
	}
}

} // namespace sx
} // namespace camera
} // namespace astro
