/*
 * QsiCamera.cpp --
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <QsiCamera.h>

namespace astro {
namespace camera {
namespace qsi {

QsiCamera::QsiCamera(const std::string& _name) : Camera(_name) {
	try {
		DeviceName	devname(name());
		camera().put_UseStructuredExceptions(true);
		camera().put_SelectCamera(devname.unitname());
	} catch (...) {
		throw std::runtime_error("cannot select camera");
	}

	// connect to the camera
	_camera.put_Connected(true);

	// get the filterwheel and guiderport information
	camera().get_HasFilterWheel(&_hasfilterwheel);
	camera().get_CanPulseGuide(&_hasguiderport);

	// query the information of the CCD
	long	xsize, ysize;
	camera().get_CameraXSize(&xsize);
	camera().get_CameraYSize(&ysize);
	DeviceName	ccdname(name(), DeviceName::Ccd, "0");
	CcdInfo	info(ccdname, astro::image::ImageSize(xsize, ysize), 0);

	// get pixel dimensions
	double	pixelsize;
	camera().get_PixelSizeX(&pixelsize);
	info.pixelwidth(pixelsize);
	camera().get_PixelSizeY(&pixelsize);
	info.pixelheight(pixelsize);

	// get the binning modes
	bool	p2bin;
	camera().get_PowerOfTwoBinning(&p2bin);
	bool	asymbin;
	camera().get_CanAsymmetricBin(&asymbin);
	short	maxbinx, maxbiny;
	camera().get_MaxBinX(&maxbinx);
	camera().get_MaxBinY(&maxbiny);
	if (p2bin) {
		for (int xbin = 1; xbin <= maxbinx; xbin <<= 1) {
			for (int ybin = 1; ybin <= maxbiny; ybin <<= 1) {
				if (asymbin || (xbin == ybin)) {
					info.addMode(Binning(xbin, ybin));
				}
			}
		}
	} else {
		for (int xbin = 1; xbin <= maxbinx; xbin++) {
			for (int ybin = 1; ybin <= maxbiny; ybin++) {
				if (asymbin || (xbin == ybin)) {
					info.addMode(Binning(xbin, ybin));
				}
			}
		}
	}

	// add the ccdinfo to the 
	ccdinfo.push_back(info);
}

QsiCamera::~QsiCamera() {
	camera().put_Connected(false);
}

void	QsiCamera::reset() {
}

CcdPtr	QsiCamera::getCcd0(size_t id) {
	if (id > 0) {
		throw std::invalid_argument("only CCD 0 defined");
	}
}

bool	QsiCamera::hasFilterWheel() const {
	return _hasfilterwheel;
}

FilterWheelPtr	QsiCamera::getFilterWheel0() {
	return FilterWheelPtr();
}

bool	QsiCamera::hasGuiderPort() const {
	return _hasguiderport;
}

GuiderPortPtr	QsiCamera::getGuiderPort0() {
	return GuiderPortPtr();
}

bool	QsiCamera::isColor() const {
	return false;
}

} // namespace qsi
} // namespace camera
} // namespace astro
