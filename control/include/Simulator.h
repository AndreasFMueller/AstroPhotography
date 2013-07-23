/*
 * Simulator.h -- guiding camera simulator
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Simulator_h
#define _Simulator_h

#include <AstroCamera.h>
#include <includes.h>

using namespace astro::camera;
using namespace astro::image;

namespace astro {
namespace camera {
namespace sim {

class SimCcd;
class SimGuiderPort;

class SimCamera : public Camera {
	// position
	double	x, y;

	// motion status
	double	vx;
	double	vy;
	double	delta;
	double	alpha;
	double	lastmovetime;

	// guiderport control
	double	movestart;
	double	movetime;
	GuiderPort::relay_bits	direction;

	// exposure control
	Exposure	exposure;
	double	exposurestart;

	// complete the current movement
	void	complete_movement();
	void	await_exposure();
public:
	SimCamera();
	virtual ~SimCamera() { }
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
