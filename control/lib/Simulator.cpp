/*
 * Simulator.cpp -- guiding simulator camera
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Simulator.h>
#include <AstroAdapter.h>
#include <debug.h>
#include <unistd.h>

using namespace astro::image;
using namespace astro::camera;

namespace astro {
namespace camera {
namespace sim {

static double	now() {
	struct timeval	now;
	gettimeofday(&now, NULL);
	return now.tv_sec + 0.000001 * now.tv_usec;
}

//////////////////////////////////////////////////////////////////////
// Simulator camera implementation
//////////////////////////////////////////////////////////////////////

SimCamera::SimCamera() {
	CcdInfo	ccd0;
	ccd0.size = ImageSize(640, 480);
	ccd0.name = "primary ccd";
	ccd0.binningmodes.insert(Binning(1, 1));
	ccdinfo.push_back(ccd0);
	// position
	x = 320;
	y = 240;

	vx = 0.1;
	vy = 0.2;
	delta = 1;
	alpha = 1;

	// neither movement nor exposures are active
	movestart = -1;
	exposurestart = -1;
	lastmovetime = now();
}

CcdPtr	SimCamera::getCcd(size_t id) {
	return CcdPtr(new SimCcd(ccdinfo[0], *this));
}

uint8_t	SimCamera::active() {
	return 0;
}

void	SimCamera::complete_movement() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "completing movement");
	// if we are still moving, figure out by how much we have moved
	if (movestart > 0) {
		double	interval = movetime;
		double	nowtime = now();
		if (nowtime < (movestart + movetime)) {
			interval = (movestart + movetime - nowtime);
		}
		// figure out in which direction the movement actually
		// goes
		double	movementangle = 0;
		switch (direction) {
		case GuiderPort::DECMINUS:
			movementangle = 3 * M_PI / 2;
			break;
		case GuiderPort::RAMINUS:
			movementangle = M_PI;
			break;
		case GuiderPort::DECPLUS:
			movementangle = M_PI / 2;
			break;
		case GuiderPort::RAPLUS:
			break;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "direction: %.0f right angles",
			round(2 * movementangle / M_PI));

		// add the movement to the coordinates
		x += interval * delta * cos(alpha + movementangle);
		y += interval * delta * sin(alpha + movementangle);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "new coordinates: (%f, %f)",
			x, y);

		// leave the remaining movement active
		movetime -= interval;
		if (movetime > 0) {
			movestart = nowtime;
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"remaining move time: %f", movetime);
		} else {
			movestart = -1;
		}
	}
}

void	SimCamera::activate(float raplus, float raminus, float decplus,
		float decminus) {
	// complete any pending movement
	complete_movement();

	// set the new movement state
	movestart = now();
	if (raplus > 0) {
		direction = GuiderPort::RAPLUS;
		movetime = raplus;
		return;
	}
	if (raminus > 0) {
		direction = GuiderPort::RAMINUS;
		movetime = raminus;
		return;
	}
	if (decplus > 0) {
		direction = GuiderPort::DECPLUS;
		movetime = decplus;
		return;
	}
	if (decminus > 0) {
		direction = GuiderPort::DECMINUS;
		movetime = decminus;
		return;
	}
}

void	SimCamera::startExposure(const Exposure& _exposure) {
	exposure = _exposure;
	exposurestart = now();
}

Exposure::State	SimCamera::exposureStatus() {
	if (exposurestart < 0) {
		return Exposure::idle;
	}
	double	nowtime = now();
	if (nowtime < exposurestart + exposure.exposuretime) {
		return Exposure::exposing;
	}
	return Exposure::exposed;
}

void	SimCamera::await_exposure() {
	// compute remaining exposure time
	double	nowtime = now();
	double	exposed = nowtime - exposurestart;
	if (exposure.exposuretime > exposed) {
		double	remaining = exposure.exposuretime - exposed;
		struct timeval	tv;
		tv.tv_sec = trunc(remaining);
		tv.tv_usec = trunc(1000000 * remaining - tv.tv_sec);
		select(0, NULL, NULL, NULL, &tv);
	}
}

ImagePtr	SimCamera::getImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieving image");
	// check whether the image is ready
	switch (exposureStatus()) {
	case Exposure::idle:
		throw std::runtime_error("camera idle");
		break;
	case Exposure::exposed:
		break;
	case Exposure::exposing:
		await_exposure();
		break;
	case Exposure::cancelling:
		throw std::runtime_error("cannot happen");
		break;
	}
	exposurestart = -1;

	// complete any pending motions
	complete_movement();

	// add base motion
	double	nowtime = now();
	x += vx * (nowtime - lastmovetime);
	y += vy * (nowtime - lastmovetime);
	lastmovetime = nowtime;

	// create the image based on the current position parameters
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating 640x480 image");
	Image<unsigned short>	image(640, 480);
	// write image contents
	debug(LOG_DEBUG, DEBUG_LOG, 0, "drawing star at %f,%f", x, y);
	for (unsigned int xi = 0; xi < 640; xi++) {
		for (unsigned int yi = 0; yi < 480; yi++) {
			double	r = hypot(xi - x, yi - y);
			unsigned short	value = 10000 * exp(-r * r / 5);
			image.pixel(xi, yi) = value;
		}
	}
	// now extract the window defiend in the frame
	debug(LOG_DEBUG, DEBUG_LOG, 0, "extracting %s window",
		exposure.frame.toString().c_str());
	WindowAdapter<unsigned short>	wa(image, exposure.frame);
	return ImagePtr(new Image<unsigned short>(wa));
}

//////////////////////////////////////////////////////////////////////
// Simulator CCD implementation
//////////////////////////////////////////////////////////////////////
void	SimCcd::startExposure(const Exposure& exposure) throw (not_implemented) {
	camera.startExposure(exposure);
}

Exposure::State	SimCcd::exposureStatus() throw (not_implemented) {
	return camera.exposureStatus();
}

ImagePtr	SimCcd::getImage() throw (not_implemented) {
	return camera.getImage();
}

//////////////////////////////////////////////////////////////////////
// Simulator Guiderport implementation
//////////////////////////////////////////////////////////////////////

SimGuiderPort::SimGuiderPort(SimCamera& _camera) : camera(_camera) {
}

SimGuiderPort::~SimGuiderPort() {
}

uint8_t	SimGuiderPort::active() {
	return camera.active();
}

void	SimGuiderPort::activate(float raplus, float raminus, float decplus,
		float decminus) {
	camera.activate(raplus, raminus, decplus, decminus);
}

GuiderPortPtr	SimCamera::getGuiderPort() throw (not_implemented) {
	return GuiderPortPtr(new SimGuiderPort(*this));
}

} // namespace sim
} // namespace camera
} // namespace astro
