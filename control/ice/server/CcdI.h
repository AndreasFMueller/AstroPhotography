/*
 * CcdI.h -- ICE CCD wrapper class definition
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _CcdI_h
#define _CcdI_h

#include <camera.h>
#include <AstroCamera.h>

namespace snowstar {

class CcdI : public Ccd {
	astro::camera::CcdPtr	_ccd;
	time_t	laststart;
public:
	CcdI(astro::camera::CcdPtr ccd) : _ccd(ccd) { }
	virtual	~CcdI() { }
	std::string	getName(const Ice::Current& current);
	CcdInfo	getInfo(const Ice::Current& current);
	void	startExposure(const Exposure&, const Ice::Current& current);
	ExposureState	exposureStatus(const Ice::Current& current);
	int	lastExposureStart(const Ice::Current& current);
	void	cancelExposure(const Ice::Current& current);
	Exposure	getExposure(const Ice::Current& current);
	ImagePrx	getImage(const Ice::Current& current);
	bool	hasGain(const Ice::Current& current);
	bool	hasShutter(const Ice::Current& current);
	ShutterState	getShutterState(const Ice::Current& current);
	void	setShutterState(ShutterState, const Ice::Current& current);
	bool	hasCooler(const Ice::Current& current);
	CoolerPrx	getCooler(const Ice::Current& current);
};

} // namespace snowstar

#endif /* _CcdI_h */


#if 0
virtual ::std::string getName(const Ice::Current& current);
virtual Ice::Byte active(const Ice::Current& current);
virtual void activate(Ice::Float, Ice::Float, const Ice::Current& current);
virtual ::std::string getName(const Ice::Current& current);
virtual Ice::Int nFilters(const Ice::Current& current);
virtual Ice::Int currentPosition(const Ice::Current& current);
virtual void select(Ice::Int, const Ice::Current& current);
virtual ::std::string filterName(Ice::Int, const Ice::Current& current);
virtual ::snowstar::FilterwheelState getState(const Ice::Current& current);
virtual ::std::string getName(const Ice::Current& current);
virtual Ice::Int min(const Ice::Current& current);
virtual Ice::Int max(const Ice::Current& current);
virtual Ice::Int current(const Ice::Current& current);
virtual void set(Ice::Int, const Ice::Current& current);
virtual ::std::string getName(const Ice::Current& current);
virtual Ice::Int nCcds(const Ice::Current& current);
virtual ::snowstar::CcdInfo getCcdinfo(Ice::Int, const Ice::Current& current);
virtual ::snowstar::CcdPrx getCcd(Ice::Int, const Ice::Current& current);
virtual bool hasFilterWheel(const Ice::Current& current);
virtual ::snowstar::FilterWheelPrx getFilterWheel(const Ice::Current& current);
virtual bool hasGuiderPort(const Ice::Current& current);
virtual ::snowstar::GuiderPortPrx getGuiderPort(const Ice::Current& current);
#endif
