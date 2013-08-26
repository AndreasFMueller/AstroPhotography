/*
 * NetGuiderPort.h -- Corba/network guider port definition
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _NetGuiderPort_h
#define _NetGuiderPort_h

#include <AstroCamera.h>
#include <device.hh>

namespace astro {
namespace camera {
namespace net {

class NetGuiderPort : public astro::camera::GuiderPort {
	Astro::GuiderPort_var	_guiderport;
public:
	NetGuiderPort(Astro::GuiderPort_var guiderport);
	~NetGuiderPort();
	virtual uint8_t	active();
	virtual void	activate(float raplus, float raminus,
				float decplus, float decminus);
};

} // namespace net
} // namespace camera
} // namespace astro

#endif /* _NetGuiderPort_h */
