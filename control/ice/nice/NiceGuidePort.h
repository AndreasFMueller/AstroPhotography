/*
 * NiceGuidePort.h --
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _NiceGuidePort_h
#define _NiceGuidePort_h

#include <AstroCamera.h>
#include <camera.h>
#include <NiceDevice.h>

using namespace astro;

namespace astro {
namespace camera {
namespace nice {

class NiceGuidePort;
class NiceGuidePortCallbackI : public snowstar::GuidePortCallback {
	NiceGuidePort&	_guideport;
public:
	NiceGuidePortCallbackI(NiceGuidePort& guideport)
		: _guideport(guideport) { }
	virtual void	activate(
			const snowstar::GuidePortActivation& activation,
			const Ice::Current& current);
	virtual void	stop(const Ice::Current& current);
};

class NiceGuidePort : public GuidePort, public NiceDevice {
	snowstar::GuidePortPrx	_guideport;
	Ice::ObjectPtr	_guideport_callback;
	Ice::Identity	_guideport_identity;
public:
	NiceGuidePort(snowstar::GuidePortPrx guideport,
		const DeviceName& devicename);
	virtual ~NiceGuidePort();

	virtual uint8_t	active();
	virtual void	activate(float raplus, float raminus,
				float decplus, float decminus);
};

} // namespace nice
} // namespace camera
} // namespace astro

#endif /* _NiceGuidePort_h */
