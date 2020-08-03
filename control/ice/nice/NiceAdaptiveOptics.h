/*
 * NiceAdaptiveOptics.h -- implement adaptive optics wrapper for ICE
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _NiceAdaptiveOptics_h
#define _NiceAdaptiveOptics_h

#include <AstroCamera.h>
#include <camera.h>
#include <NiceDevice.h>
#include <NiceGuidePort.h>

namespace astro {
namespace camera {
namespace nice {

class NiceAdaptiveOptics;
class NiceAdaptiveOpticsCallbackI : public snowstar::AdaptiveOpticsCallback {
	NiceAdaptiveOptics&	_adaptiveoptics;
public:
	NiceAdaptiveOpticsCallbackI(NiceAdaptiveOptics& adaptiveoptics)
		: _adaptiveoptics(adaptiveoptics) { }
	virtual void	point(const snowstar::Point& p,
				const Ice::Current& current);
	virtual void	stop(const Ice::Current& current);
};

class NiceAdaptiveOptics : public AdaptiveOptics, public NiceDevice {
	snowstar::AdaptiveOpticsPrx	_adaptiveoptics;
	Ice::ObjectPtr	_adaptiveoptics_callback;
	Ice::Identity	_adaptiveoptics_identity;
public:
	NiceAdaptiveOptics(snowstar::AdaptiveOpticsPrx adaptiveoptics,
		const DeviceName& devicename);
	virtual ~NiceAdaptiveOptics();

protected:
	virtual void	set0(const Point& position);
	virtual GuidePortPtr	getGuidePort0();
};

} // namespace nice
} // namespace camera
} // namespace astro

#endif /* _NiceAdpativeOptics_h */
