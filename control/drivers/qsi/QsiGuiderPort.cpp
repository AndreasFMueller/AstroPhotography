/*
 * QsiGuiderPort.cpp -- QSI filter wheel implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <QsiGuiderPort.h>
#include <AstroExceptions.h>

namespace astro {
namespace camera {
namespace qsi {

QsiGuiderPort::QsiGuiderPort(QsiCamera& camera)
	: GuiderPort(DeviceName(camera.name(), DeviceName::Guiderport,
		"guiderport")),
	  _camera(camera) {
}

QsiGuiderPort::~QsiGuiderPort() {
}

uint8_t	QsiGuiderPort::active() {
	throw std::runtime_error("not implemented yet");
}

static long	milliseconds(float time) {
	long	result = 1000 * time;
	return result;
}

void	QsiGuiderPort::activate(float raplus, float raminus,
		float decplus, float decminus) {
	if ((raplus > 0) && (raminus > 0)) {
		throw std::invalid_argument("cannot activate both RA lines");
	}
	if ((decplus > 0) && (decminus > 0)) {
		throw std::invalid_argument("cannot activate both DEC lines");
	}
	if (raplus > 0) {
		_camera.camera().PulseGuide(QSICamera::guideEast,
			milliseconds(raplus));
	}
	if (raminus > 0) {
		_camera.camera().PulseGuide(QSICamera::guideWest,
			milliseconds(raminus));
	}
	if (decplus > 0) {
		_camera.camera().PulseGuide(QSICamera::guideNorth,
			milliseconds(decplus));
	}
	if (decminus > 0) {
		_camera.camera().PulseGuide(QSICamera::guideSouth,
			milliseconds(decminus));
	}
}

} // namespace qsi
} // namespace camera
} // namespace astro
