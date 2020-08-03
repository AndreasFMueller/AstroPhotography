/*
 * GuidePortI.h -- ICE GuidePort wrapper class definition
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _GuidePortI_h
#define _GuidePortI_h

#include <camera.h>
#include <AstroCamera.h>
#include <DeviceI.h>
#include <CallbackHandler.h>

namespace snowstar {

template<>
void	callback_adapter<GuidePortCallbackPrx>(GuidePortCallbackPrx& p,
		const astro::callback::CallbackDataPtr data);

class GuidePortICallback;
typedef std::shared_ptr<GuidePortICallback>	GuidePortICallbackPtr;

class GuidePortI : virtual public GuidePort, virtual public DeviceI {
	astro::camera::GuidePortPtr	_guideport;
	GuidePortICallbackPtr	guideportcallbackptr;
public:
	GuidePortI(astro::camera::GuidePortPtr guideport);
	virtual	~GuidePortI();

	virtual Ice::Byte	active(const Ice::Current& current);
	virtual void	activate(float, float, const Ice::Current& current);
static	GuidePortPrx	createProxy(const std::string& name,
		const Ice::Current& current);
private:
	SnowCallback<GuidePortCallbackPrx>	callbacks;
public:
	virtual void	registerCallback(const Ice::Identity& callback,
				const Ice::Current& current);
	virtual void	unregisterCallback(const Ice::Identity& callback,
				const Ice::Current& current);
	void	callbackActivate(const astro::callback::CallbackDataPtr data);
};

class GuidePortICallback : public astro::callback::Callback {
	GuidePortI&	_guideport;
public:
	GuidePortICallback(GuidePortI& guideport) : _guideport(guideport) { }
	virtual astro::callback::CallbackDataPtr	operator()(
		astro::callback::CallbackDataPtr data) {
		_guideport.callbackActivate(data);
		return data;
	}
};

} // namespace snowstar

#endif /* _GuidePortI_h */
