/*
 * QhyCamera.cpp -- QHY camera implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <QhyCamera.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroExceptions.h>
#include <QhyUtils.h>
#include <QhyCcd.h>

namespace astro {
namespace camera {
namespace qhy {

/**
 * \brief Auxiliary function generate the camera name from the deviceptr
 */
static astro::DeviceName	cameraname(usb::DevicePtr& deviceptr) {
	QhyName	qhyname(deviceptr);
	return qhyname.name(DeviceName::Camera);
}

/**
 * \brief Construct a camera object
 */
QhyCamera::QhyCamera(usb::DevicePtr& devptr)
	: astro::camera::Camera(cameraname(devptr)), deviceptr(devptr) {
	// get the vendor and product id
	usb::DeviceDescriptorPtr	descriptor = deviceptr->descriptor();
	idProduct = descriptor->idProduct();
	idVendor = descriptor->idVendor();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "constructing device %hx:%hx",
		idVendor, idProduct);

	// get the device based on these
	qhydeviceptr = ::qhy::getDevice(idVendor, idProduct);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "QHY device constructed");

	// construct the device name
	DeviceName	ccdname(name(), DeviceName::Ccd, "Imaging");

	// construct the CcdInfo
	switch (idProduct) {
	case 0x6003:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "constructing ccdinfo");
		{
			CcdInfo	info(ccdname, ImageSize(3040, 2024), 0);
			info.pixelwidth(7.8e-6);
			info.pixelheight(7.8e-6);
			info.addMode(image::Binning(1,1));
			info.addMode(image::Binning(2,2));
			info.addMode(image::Binning(4,4));
			ccdinfo.push_back(info);
		}
		break;
	default:
		debug(LOG_ERR, DEBUG_LOG, 0, "%hu unknown QHY device",
			idProduct);
		throw std::runtime_error("device not implemented");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "QHY camera constructed");
}

/**
 * \brief Destroy a camera
 */
QhyCamera::~QhyCamera() {
}

/**
 * \brief Get the Ccd
 */
CcdPtr	QhyCamera::getCcd0(size_t ccdindex) {
	if (ccdindex) {
		debug(LOG_ERR, DEBUG_LOG, 0, "CCD index %u out of range",
			ccdindex);
		throw NotFound("ccd id out of range");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create QHY ccd: %s",
		ccdinfo[0].toString().c_str());
	return CcdPtr(new QhyCcd(ccdinfo[0], qhydeviceptr, *this));
}

} // namespace qhy
} // namespace camera
} // namespace astro
