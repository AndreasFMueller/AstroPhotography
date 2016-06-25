/*
 * AsiCamera.cpp -- ASI camera implementation
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AsiCamera.hh>
#include <utils.h>
#include <AsiCcd.h>
#include <AsiGuiderPort.h>

namespace astro {
namespace camera {
namespace asi {

/**
 * \brief Construct an AsiCamera
 *
 * \param _index	index of the camera
 */
AsiCamera::AsiCamera(int index) : Camera(asiCameraName(index)),
	_index(index) {
	// open the camera
	int	rc = ASIOpenCamera(index);
	if (ASI_SUCCESS != rc) {
		throw std::runtime_error("cannot open camera");
	}

	// get information about the CCD
	ASI_CAMERA_INFO camerainfo;
        ASIGetCameraProperty(&camerainfo, _index);

	// set common variables depending on the camera info
	_hasGuiderPort = (camerainfo.ST4Port) ? true : false;
	_isColor = (camerainfo.IsColorCam) ? true : false;
	_hasCooler = (camerainfo.IsCoolerCam) ? true : false;
	ImageSize	size(camerainfo.MaxWidth, camerainfo.MaxHeight);

	// construct a CcdInfo object for each image format
	int	imgtypeidx = 0;
	while (camerainfo.SupportedVideoFormat[imgtypeidx] != -1) {
		// construct the name for this 
		std::string	it = AsiCcd::imgtype2string(
			camerainfo.SupportedVideoFormat[imgtypeidx]);
		DeviceName	ccdname = name().child(DeviceName::Ccd, it);
		CcdInfo	info(ccdname, size, 0);

		// pixel size
		info.pixelwidth(camerainfo.PixelSize * 1e-6);
		info.pixelheight(camerainfo.PixelSize * 1e-6);

		// add all binning modes
		int	binindex = 0;
		while (camerainfo.SupportedBins[binindex]) {
			int	bin = camerainfo.SupportedBins[binindex];
			info.addMode(Binning(bin, bin));
			binindex++;
		}

		// ASI cameras have no shutter
		info.shutter(false);

		// add the ccdinfo object to the array
		ccdinfo.push_back(info);
	}
}

/**
 * \brief Destroy the AsiCamera
 */
AsiCamera::~AsiCamera() {
	ASICloseCamera(_index);
}

/**
 * \brief Get a CCD from 
 */
CcdPtr	AsiCamera::getCcd0(size_t id) {
	if (id >= nCcds()) {
		std::string	msg = stringprintf("ccd %d does not exist",
			id);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::range_error(msg);
	}
	AsiCcd	*ccd = new AsiCcd(ccdinfo[id], *this);
	ccd->hasCooler(_hasCooler);
	return CcdPtr(ccd);
}

/**
 * \brief Does the camera have guider port?
 *
 * Always returns true, because all ASI cameras have a guider port
 */
bool    AsiCamera::hasGuiderPort() const {
	return _hasGuiderPort;
}

/**
 * \brief Get a guider port
 */
GuiderPortPtr	AsiCamera::getGuiderPort0() {
	return GuiderPortPtr(new AsiGuiderPort(*this));
}

/**
 * \brief Is this a color camera?
 *
 * The variable _isColor was set during construction of the camera
 */
bool	AsiCamera::isColor() const {
	return _isColor;
}

} // namespace asi
} // namespace camera
} // namespace astro
