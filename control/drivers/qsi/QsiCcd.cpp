/*
 * QsiCcd.cpp -- QSI CCD implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */

#include <QsiCcd.h>
#include <QsiCooler.h>
#include <AstroDebug.h>
#include <includes.h>

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
	Ccd::startExposure(exposure);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "start QSI exposure");
	// set the binning mode
	_camera.camera().put_BinX(exposure.mode.getX());
	_camera.camera().put_BinY(exposure.mode.getY());

	// compute the frame size in binned pixels, as this is what
	// the QSI camera expects
	ImagePoint	origin = exposure.frame.origin() / exposure.mode;
	ImageSize	size = exposure.frame.size() / exposure.mode;
	ImageRectangle	frame(origin, size);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "requesting %s image",
		frame.toString().c_str());

	// set the subframe
	_camera.camera().put_NumX(size.width());
	_camera.camera().put_NumY(size.height());
	_camera.camera().put_StartX(origin.x());
	_camera.camera().put_StartY(origin.y());

	// get shutter info
	bool	light = (exposure.shutter == SHUTTER_OPEN);
	_camera.camera().StartExposure(exposure.exposuretime, light);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%fsec %s exposure started",
		exposure.exposuretime, (light) ? "light" : "dark");

	// check the current state of the camera
	exposureStatus();
}

std::string	state2string(QSICamera::CameraState qsistate) {
	switch (qsistate) {
	case QSICamera::CameraIdle:
		return std::string("idle");
	case QSICamera::CameraWaiting:
		return std::string("waiting");
	case QSICamera::CameraExposing:
		return std::string("exposing");
	case QSICamera::CameraReading:
		return std::string("reading");
	case QSICamera::CameraDownload:
		return std::string("download");
	case QSICamera::CameraError:
		return std::string("error");
	}
	return std::string("unknonw");
}

Exposure::State	QsiCcd::exposureStatus() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "checking camera state");
	QSICamera::CameraState	qsistate;
	_camera.camera().get_CameraState(&qsistate);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "qsistate = %s",
		state2string(qsistate).c_str());
	switch (state) {
	case Exposure::idle:
		switch (qsistate) {
		case QSICamera::CameraIdle:
			break;
		case QSICamera::CameraWaiting:
		case QSICamera::CameraExposing:
			state = Exposure::exposing;
			break;
		case QSICamera::CameraReading:
		case QSICamera::CameraDownload:
			state = Exposure::exposed;
			break;
		case QSICamera::CameraError:
			break;
		}
		break;
	case Exposure::exposing:
		switch (qsistate) {
		case QSICamera::CameraIdle:
		case QSICamera::CameraWaiting:
			state = Exposure::exposed;
			break;
		case QSICamera::CameraExposing:
			state = Exposure::exposing;
			break;
		case QSICamera::CameraReading:
		case QSICamera::CameraDownload:
			state = Exposure::exposed;
			break;
		case QSICamera::CameraError:
			break;
		}
		break;
	case Exposure::exposed:
		switch (qsistate) {
		case QSICamera::CameraIdle:
		case QSICamera::CameraWaiting:
		case QSICamera::CameraExposing:
		case QSICamera::CameraReading:
		case QSICamera::CameraDownload:
		case QSICamera::CameraError:
			break;
		}
		break;
	case Exposure::cancelling:
		switch (qsistate) {
		case QSICamera::CameraIdle:
			state = Exposure::idle;
			break;
		case QSICamera::CameraWaiting:
			break;
		case QSICamera::CameraExposing:
		case QSICamera::CameraReading:
		case QSICamera::CameraDownload:
			state = Exposure::exposing;
			break;
		case QSICamera::CameraError:
			break;
		}
		break;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new state %d", state);
	return state;
}

void	QsiCcd::cancelExposure() {
	_camera.camera().AbortExposure();
	state = Exposure::idle;
}

shutter_state	QsiCcd::getShutterState() {
	throw std::runtime_error("cannot query current shutter state");
}

void	QsiCcd::setShutterState(const shutter_state& /* state */) {
	throw std::runtime_error("cannot directly control shutter state");
}

ImagePtr	QsiCcd::getRawImage() {
	int	x, y, z;
	_camera.camera().get_ImageArraySize(x, y, z);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "x = %d, y = %d, z = %d", x, y, z);
	if (z != 2) {
		throw std::runtime_error("only ushort images supported");
	}
	ImageSize	size(x, y);
	Image<unsigned short>	*image = new Image<unsigned short>(size);
	ImagePtr	result(image);
	_camera.camera().get_ImageArray(image->pixels);
	return result;
}

CoolerPtr	QsiCcd::getCooler0() {
	QsiCooler	*cooler = new QsiCooler(_camera);
	return CoolerPtr(cooler);
}


} // namespace qsi
} // namespace camera
} // namespace astro

