/*
 * AdaptiveOpticsI.h -- ICE AdaptiveOptics wrapper class definition
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AdaptiveOptics_h
#define _AdaptiveOptics_h

#include <device.h>
#include <AstroCamera.h>
#include <DeviceI.h>
#include <CallbackHandler.h>

namespace snowstar {

/**
 * \brief Callback adapter for adaptive optics proxy
 */
template<>
void	callback_adapter<AdaptiveOpticsPrx>(AdaptiveOpticsPrx p,
	const astro::callback::CallbackDataPtr data);

class AdaptiveOpticsICallback;
typedef std::shared_ptr<AdaptiveOpticsICallback> AdaptiveOpticsICallbackPtr;

/**
 * \brief The AdaptiveOptics servant
 */
class AdaptiveOpticsI : virtual public AdaptiveOptics, virtual public DeviceI {
	astro::camera::AdaptiveOpticsPtr	_ao;
	AdaptiveOpticsICallbackPtr	adaptiveopticscallbackptr;
public:
	AdaptiveOpticsI(astro::camera::AdaptiveOpticsPtr ao);
	virtual ~AdaptiveOpticsI();
	virtual void	set(const Point& position, const Ice::Current& current);
	virtual Point	get(const Ice::Current& current);
	virtual bool	hasGuidePort(const Ice::Current& current);
	virtual GuidePortPrx	getGuidePort(const Ice::Current& current);	
	virtual void	center(const Ice::Current& current);
private:
	SnowCallback<AdaptiveOpticsCallbackPrx>	callbacks;
public:
	void	registerCallback(const Ice::Identity& callback,
			const Ice::Current& current);
	void	unregisterCallback(const Ice::Identity& callback,
			const Ice::Current& current);
	void	callbackUpdate(const astro::callback::CallbackDataPtr data);
};

/**
 * \brief The callback to install into the adaptive optics device
 */
class AdaptiveOpticsICallback : public astro::callback::Callback {
	AdaptiveOpticsI&	_adaptiveoptics;
public:
	AdaptiveOpticsICallback(AdaptiveOpticsI& adaptiveoptics)
		: _adaptiveoptics(adaptiveoptics) { }
	virtual astro::callback::CallbackDataPtr	operator()(
		astro::callback::CallbackDataPtr data) {
		_adaptiveoptics.callbackUpdate(data);
		return data;
	}
};


} // namespace snowstar

#endif /* _AdaptiveOptics_h */
