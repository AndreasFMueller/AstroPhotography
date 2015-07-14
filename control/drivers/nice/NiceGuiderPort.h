/*
 * NiceGuiderPort.h --
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _NiceGuiderPort_h
#define _NiceGuiderPort_h

#include <AstroCamera.h>
#include <camera.h>
#include <NiceDevice.h>

using namespace astro;

namespace astro {
namespace camera {
namespace nice {

class NiceGuiderPort : public GuiderPort, public NiceDevice {
	snowstar::GuiderPortPrx	_guiderport;
public:
	NiceGuiderPort(snowstar::GuiderPortPrx guiderport,
		const DeviceName& devicename);
	virtual ~NiceGuiderPort();

	virtual uint8_t	active();
	virtual void	activate(float raplus, float raminus,
				float decplus, float decminus);
};

} // namespace nice
} // namespace camera
} // namespace astro

#endif /* _NiceGuiderPort_h */
