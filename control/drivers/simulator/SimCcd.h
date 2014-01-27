/*
 * SimCcd.h -- Simulator Ccd definition
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SimCcd_h
#define _SimCcd_h

#include <SimLocator.h>
#include <SimCamera.h>
#include <Stars.h>

namespace astro {
namespace camera {
namespace simulator {

class SimCcd : public Ccd {
	SimLocator&	_locator;
	double	starttime;
	shutter_state	shutter;
	StarField	starfield;
	StarCamera<unsigned short>	starcamera;
public:
	SimCcd(const CcdInfo& _info, SimLocator& locator);

	virtual void	startExposure(const Exposure& exposure);
	virtual Exposure::State	exposureStatus();
	virtual void	cancelExposure();
	virtual bool	wait();

	virtual shutter_state	getShutterState() { return shutter; }
	virtual void	setShuterState(const shutter_state& state);

	virtual astro::image::ImagePtr	getRawImage();

	virtual bool	hasCooler() const { return true; }
	virtual CoolerPtr	getCooler0() { return _locator.cooler(); }
};

} // namespace simulator
} // namespace camera
} // namespace astro

#endif /* _SimCcd_h */
