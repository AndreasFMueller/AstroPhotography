/*
 * NetCcd.h -- Network based CCD definitions
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswi
 */
#ifndef _NetCcd_h
#define _NetCcd_h

#include <NetCamera.h>
#include "../../idl/device.hh"

namespace astro {
namespace caemra {
namespace net {

class NetCcd : public Ccd {
	Astro::Ccd_var	_ccd;
public:
	NetCcd(CcdInfo& _info, Astro::Ccd_var ccd);

	virtual void    startExposure(const Exposure& exposure);
	virtual Exposure::State exposureStatus();
	virtual void    cancelExposure();
	virtual bool    wait();

	virtual shutter_state   getShutterState();
	virtual void    setShuterState(const shutter_state& state);

	virtual astro::image::ImagePtr  getImage();

	virtual bool    hasCooler() const;
	virtual CoolerPtr       getCooler0();
};

} // namespace net
} // namespace camera
} // namespace astro

#endif /* _NetCcd_h */
