/*
 * QsiCcd.cpp -- QSI CCD implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */

#include <QsiCcd.h>
#include <QsiCooler.h>
#include <AstroDebug.h>

using namespace astro::image;

namespace astro {
namespace camera {
namespace qsi {

QsiCcd::QsiCcd(const CcdInfo& info, QsiCamera& camera)
	: Ccd(info), _camera(camera) {
}

QsiCcd::~QsiCcd() {
	// abort an exposure in progress, if any
	try {
		cancelExposure();
	} catch (...) {
	}
	// XXX turn of the cooler
}

void	QsiCcd::startExposure(const Exposure& exposure) {
	// set the binning mode
	_camera.camera().put_BinX(exposure.mode.getX());
	_camera.camera().put_BinY(exposure.mode.getX());

	// compute the frame size in binned pixels, as this is what
	// the QSI camera expects
	ImagePoint	origin = exposure.frame.origin() / exposure.mode;
	ImageSize	size = exposure.frame.size() / exposure.mode;
	ImageRectangle	frame(origin, size);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "requesting %s image",
		frame.toString().c_str());

	// set the subframe
	_camera.camera().put_NumX(size.width());
	_camera.camera().put_NumX(size.height());
	_camera.camera().put_StartX(origin.x());
	_camera.camera().put_StartX(origin.y());

	// get shutter info
	bool	light = (exposure.shutter == SHUTTER_OPEN);
	_camera.camera().StartExposure(exposure.exposuretime, light);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%fsec %s exposure started",
		exposure.exposuretime, (light) ? "light" : "dark");
}

Exposure::State	QsiCcd::exposureStatus() {
	QSICamera::CameraState	state;
	_camera.camera().get_CameraState(&state);
	switch (state) {
	case QSICamera::CameraIdle:
		return Exposure::idle;
	case QSICamera::CameraWaiting:
		return Exposure::exposed;
	case QSICamera::CameraExposing:
		return Exposure::exposing;
	case QSICamera::CameraReading:
	case QSICamera::CameraDownload:
	case QSICamera::CameraError:
		return Exposure::exposed;
	}
}

void	QsiCcd::cancelExposure() {
	_camera.camera().AbortExposure();
}

shutter_state	QsiCcd::getShutterState() {
	throw std::runtime_error("cannot query current shutter state");
}

void	QsiCcd::setShutterState(const shutter_state& state) {
	throw std::runtime_error("cannot directly control shutter state");
}

ImagePtr	QsiCcd::getImage() {
	int	x, y, z;
	_camera.camera().get_ImageArraySize(x, y, z);
	if (z > 1) {
		throw std::runtime_error("multiplane images not implemented");
	}
	ImageSize	size(x, y);
	Image<unsigned short>	*image = new Image<unsigned short>(size);
	ImagePtr	result(image);
	_camera.camera().get_ImageArray(image->pixels);
	addMetadata(*image);
	return result;
}

CoolerPtr	QsiCcd::getCooler0() {
	QsiCooler	*cooler = new QsiCooler(_camera);
	return CoolerPtr(cooler);
}


} // namespace qsi
} // namespace camera
} // namespace astro

