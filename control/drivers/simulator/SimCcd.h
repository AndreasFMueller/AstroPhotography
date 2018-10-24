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
#include <thread>

namespace astro {
namespace camera {
namespace simulator {

class SimCcd : public Ccd {
	SimLocator&	_locator;
	std::thread	*_thread;
	double	starttime;
	Shutter::state	shutter;
	StarField	starfield;
	StarCamera<unsigned short>	starcamera;
	RaDec	_last_direction;
	void	catalogStarfield(const RaDec& direction);
public:
	SimCcd(const CcdInfo& _info, SimLocator& locator);

	virtual void	startExposure(const Exposure& exposure);
	virtual CcdState::State	exposureStatus();
	virtual void	cancelExposure();

	virtual Shutter::state	getShutterState() { return shutter; }
	virtual void	setShuterState(const Shutter::state& state);

	virtual astro::image::ImagePtr	getRawImage();

	virtual bool	hasCooler() const { return true; }
	virtual CoolerPtr	getCooler0() { return _locator.cooler(); }
	virtual std::string	userFriendlyName() const;

private:
	astro::image::ImagePtr	_image;
public:
	void	createimage();
};

} // namespace simulator
} // namespace camera
} // namespace astro

#endif /* _SimCcd_h */
