/*
 * NetCcd.h -- Network based CCD definitions
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswi
 */
#ifndef _NetCcd_h
#define _NetCcd_h

#include <NetCamera.h>
#include <camera.hh>

namespace astro {
namespace camera {
namespace net {

/**
 * \brief Network client for CCDs
 */
class NetCcd : public Ccd {
	Astro::Ccd_ptr	_ccd;
	void	synchronize();
public:
	NetCcd(const CcdInfo& _info, Astro::Ccd_ptr ccd);
	NetCcd(Astro::Ccd_ptr ccd);
	~NetCcd();

	virtual void    startExposure(const astro::camera::Exposure& exposure);
	virtual astro::camera::Exposure::State exposureStatus();
	virtual void    cancelExposure();

	virtual shutter_state   getShutterState();
	virtual void    setShutterState(const shutter_state& state);

	virtual astro::image::ImagePtr  getImage();

	virtual bool    hasCooler() const;
	virtual CoolerPtr       getCooler0();
};

} // namespace net
} // namespace camera
} // namespace astro

#endif /* _NetCcd_h */
