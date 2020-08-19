/*
 * FilterWheelI.h -- filter wheel implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _FilterWheelI_h
#define _FilterWheelI_h

#include <camera.h>
#include <AstroCamera.h>
#include <DeviceI.h>
#include <CallbackHandler.h>

namespace snowstar {

template<>
void	callback_adapter<FilterWheelCallbackPrx>(FilterWheelCallbackPrx p,
	const astro::callback::CallbackDataPtr data);

class FilterWheelICallback;
typedef std::shared_ptr<FilterWheelICallback>        FilterWheelICallbackPtr;

/**
 * \brief FilterWheel servant
 */
class FilterWheelI : virtual public FilterWheel, virtual public DeviceI {
	astro::camera::FilterWheelPtr	_filterwheel;
	FilterWheelICallbackPtr	filterwheelcallbackptr;
public:
	FilterWheelI(astro::camera::FilterWheelPtr filterwheel);
	virtual	~FilterWheelI();
	virtual int	nFilters(const Ice::Current& current);
	virtual int	currentPosition(const Ice::Current& current);
	virtual void	select(int, const Ice::Current& current);
	virtual void	selectName(const std::string& filtername,
		const Ice::Current& current);
	virtual std::string	filterName(int, const Ice::Current& current);
	virtual FilterwheelState	getState(const Ice::Current& current);
static	FilterWheelPrx	createProxy(const std::string& filterwheelname,
		const Ice::Current& current);
private:
	SnowCallback<FilterWheelCallbackPrx>	callbacks;
public:
	virtual void	registerCallback(const Ice::Identity& callback,
				const Ice::Current& current);
	virtual void	unregisterCallback(const Ice::Identity& callback,
				const Ice::Current& current);
	void	callbackUpdate(const astro::callback::CallbackDataPtr data);
};

/**
 * \brief FilterWheel callback
 */
class FilterWheelICallback : public astro::callback::Callback {
	FilterWheelI&	_filterwheel;
public:
	FilterWheelICallback(FilterWheelI& filterwheel)
		: _filterwheel(filterwheel) { }
	virtual astro::callback::CallbackDataPtr	operator()(
		astro::callback::CallbackDataPtr data) {
		_filterwheel.callbackUpdate(data);
		return data;
	}
};


} // namespace snowstar

#endif /* _FilterWheelI_h */
