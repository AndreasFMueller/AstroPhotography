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

class NiceAdaptiveOptics : public AdaptiveOptics, public NiceDevice {
	snowstar::AdaptiveOpticsPrx	_adaptiveoptics;
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
