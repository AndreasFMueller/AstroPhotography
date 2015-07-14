/*
 * NiceCcd.h  -- wrapper for CCD 
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _NiceCcd_h
#define _NiceCcd_h

#include <AstroCamera.h>
#include <camera.h>
#include <NiceDevice.h>

using namespace astro::camera;

namespace astro {
namespace camera {
namespace nice {

class NiceCcd : public Ccd, public NiceDevice {
	snowstar::CcdPrx	_ccd;
public:
	NiceCcd(snowstar::CcdPrx ccdprx, const DeviceName& devicename);
	virtual ~NiceCcd();

	virtual void	startExposure(const Exposure& exposure);
	virtual Exposure::State	exposureStatus();
	virtual void	cancelExposure();

	virtual Shutter::state	getShutterState();
	virtual void	setShutterState(const Shutter::state& state);

	virtual astro::image::ImagePtr	getRawImage();

	virtual bool	hasGain();
	virtual std::pair<float, float>	gainInterval();

	virtual bool	hasCooler() const;
	virtual CoolerPtr	getCooler0();
};


} // namespace nice
} // namespace camera
} // namespace astro

#endif /* _NiceCcd_h */
