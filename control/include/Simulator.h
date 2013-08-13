/*
 * Simulator.h -- guiding camera simulator
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Simulator_h
#define _Simulator_h

#include <AstroCamera.h>
#include <includes.h>
#include <pthread.h>

using namespace astro::camera;
using namespace astro::image;

namespace astro {
namespace camera {
namespace sim {

class SimCcd;
class SimGuiderPort;

/**
 * \brief Simulator camera for guiding code tests
 *
 * This camera simulates a guiding camera on a slightly misaligned telescope.
 * Whenever an image is taken, it places it at the current (x,y) coordinates.
 * These coordinates are initialized to the center of the image, but they
 * change over time according to the variables vx and vy. The speed can 
 * further be modified by activating the guider port available with the
 * camera. Activating the guider port in right ascencsion for a given time
 * changes the coordinates based on the velocity set in the member variable
 * delta and the direction set in the ra.alpha. Similarly for declination.
 */
class SimCamera : public Camera {
	// position
	double	x, y;

public:
	// motion status
	double	delta;
	
	// common movement
	double	vx;
	double	vy;

	// guiderport control
	typedef struct movement {
		double	starttime;	// time when movement started
		double	duration;	// duration of movement
		int	direction;	// +- 1
		double	alpha;
		void clear() { starttime = -1; }
	} movement;

	movement	ra;
	movement	dec;

	// exposure control
	Exposure	exposure;
private:
	double	exposurestart;
	double	lastexposure;

	// complete the current movement
	pthread_mutex_t	mutex;
	void	complete(movement& mov);
	void	complete_movement();
	void	await_exposure();
public:
	SimCamera();
	virtual ~SimCamera();
	virtual CcdPtr	getCcd(size_t id);
	virtual GuiderPortPtr	getGuiderPort() throw (not_implemented);
	// guider port functions
	uint8_t	active();
	void	activate(float raplus, float raminus,
			float decplus, float decminus);
	// ccd functions
	void	startExposure(const Exposure& exposure);
	Exposure::State	exposureStatus();
	ImagePtr	getImage();
};

/**
 * \brief Simulator camera CCD
 */
class SimCcd : public Ccd {
	SimCamera&	camera;
public:
	SimCcd(const CcdInfo& info, SimCamera& _camera) : Ccd(info), camera(_camera) { }
	virtual ~SimCcd() { }
	virtual void	startExposure(const Exposure& exposure)
		throw (not_implemented);
	virtual Exposure::State	exposureStatus() throw (not_implemented);
	virtual ImagePtr	getImage() throw (not_implemented);
};

/**
 * \brief Simulator camera guider port
 */
class SimGuiderPort : public GuiderPort {
	SimCamera&	camera;
public:
	SimGuiderPort(SimCamera& camera);
	virtual ~SimGuiderPort();
	virtual uint8_t	active();
	virtual void	activate(float raplus, float raminus,
		float decplus, float decminus);
};

} // namespace sim
} // namespace camera
} // namespace astro

#endif /* _Simulator_h */
