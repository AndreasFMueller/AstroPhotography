/*
 * AtikCamera.cpp -- ATIK camera implementation
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AtikCamera.h>
#include <AtikUtils.h>
#include <AtikCcd.h>
#include <atikccdusb.h>

namespace astro {
namespace camera {
namespace atik {

/**
 * \brief Constructor for an ATIK camera
 */
AtikCamera::AtikCamera(::AtikCamera *camera)
	: Camera(cameraname(camera)), _camera(camera) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating ATIK camera %s",
		name().toString().c_str());
	// get the capabilities
	const char	*name;
	CAMERA_TYPE	type;
	_camera->getCapabilities(&name, &type, &capa);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "capabilities of %s retrieved", name);

	// create CCD info for each CCD
	ImageSize	size(capa.pixelCountX, capa.pixelCountY);
	CcdInfo	info(ccdname(_camera, "Imaging"), size, 0);
	for (unsigned int binx = 1; binx <= capa.maxBinX; binx++) {
		for (unsigned int biny = 1; biny < capa.maxBinY; biny++) {
			info.addMode(Binning(binx, biny));
		}
	}
	info.pixelwidth(capa.pixelSizeX * 1e-6);
	info.pixelheight(capa.pixelSizeY * 1e-6);

	// exposure times
	if (capa.supportsLongExposure) {
		info.maxexposuretime(3600);
	} else {
		info.maxexposuretime(capa.maxShortExposure);
	}
	info.minexposuretime(0.2);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "add ccdinfo %s",
		info.toString().c_str());
	ccdinfo.push_back(info);

	CcdInfo	info8(ccdname(_camera, "8bit"), size, 0);
	for (unsigned int binx = 1; binx <= capa.maxBinX; binx++) {
		for (unsigned int biny = 1; biny < capa.maxBinY; biny++) {
			info8.addMode(Binning(binx, biny));
		}
	}
	info8.pixelwidth(capa.pixelSizeX * 1e-6);
	info8.pixelheight(capa.pixelSizeY * 1e-6);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add ccdinfo %s",
		info8.toString().c_str());
	ccdinfo.push_back(info8);
}

AtikCamera::~AtikCamera() {
}

CcdPtr	AtikCamera::getCcd0(size_t ccdid) {
	if (ccdid >= ccdinfo.size()) {
		std::string	msg = stringprintf("ccd id %d out of range",
			ccdid);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::range_error(msg);
	}
	return CcdPtr(new AtikCcd(ccdinfo[ccdid], _camera));
}

unsigned int	AtikCamera::nCcds() const {
	return (capa.has8BitMode) ? 2 : 1;
}

bool	AtikCamera::hasFilterWheel() const {
	return capa.hasFilterWheel;
}

FilterWheelPtr	AtikCamera::getFilterWheel0() {
	return FilterWheelPtr(NULL);
}

bool	AtikCamera::hasGuidePort() const {
	return capa.hasGuidePort;
}

GuidePortPtr	AtikCamera::getGuidePort0() {
	return GuidePortPtr(NULL);
}

} // namespace atik
} // namespace camera
} // namespace astro
