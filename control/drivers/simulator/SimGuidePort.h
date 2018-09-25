/*
 * SimGuidePort.h -- GuidePort definition for the simulator camera
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SimGuidePort_h
#define _SimGuidePort_h

#include "SimLocator.h"
#include <AstroTypes.h>

namespace astro {
namespace camera {
namespace simulator {

/**
 * \brief Simulated guider port
 */
class SimGuidePort : public GuidePort {
	SimLocator&	_locator;
	double	starttime;
	Point	_drift;
	Point	_offset;
	double	lastactivation;
	double	ra, dec;
	double	pixelspeed;
	Point	_ravector, _decvector;
	SimGuidePort(const SimGuidePort& other);
	SimGuidePort&	operator=(const SimGuidePort& other);
public:
	void	update();
	SimGuidePort(SimLocator& locator);
	virtual uint8_t	active();
	virtual void	activate(float raplus, float raminus,
		float decplus, float decminus);

	// parameters for the simulation
	const Point&	drift() const { return _drift; }
	void	drift(const Point& drift) { _drift = drift; }

	const Point&	ravector() const { return _ravector; }
	void	ravector(const Point& ravector) { _ravector = ravector; }

	const Point&	decvector() const { return _decvector; }
	void	decvector(const Point& decvector) { _decvector = decvector; }

	// retrive offset and rotation
	Point	offset();
};

} // namespace simulator
} // namespace camera
} // namespace astro


#endif /* _SimGuidePort_h */
