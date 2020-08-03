/*
 * CoolerI.h -- ICE Cooler wrapper
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _CoolerI_h
#define _CoolerI_h

#include <camera.h>
#include <AstroCamera.h>
#include <DeviceI.h>
#include <CallbackHandler.h>

namespace snowstar {

template<>
void	callback_adapter<CoolerCallbackPrx>(CoolerCallbackPrx& p,
	const astro::callback::CallbackDataPtr data);

class CoolerICallback;
typedef std::shared_ptr<CoolerICallback>	CoolerICallbackPtr;

class CoolerI : virtual public Cooler, virtual public DeviceI {
	astro::camera::CoolerPtr	_cooler;
	CoolerICallbackPtr	coolercallbackptr;
public:
	CoolerI(astro::camera::CoolerPtr cooler);
	virtual ~CoolerI();
	float	getSetTemperature(const Ice::Current& current);
	float	getActualTemperature(const Ice::Current& current);
	void	setTemperature(float temperature, const Ice::Current& current);
	bool	isOn(const Ice::Current& current);
	void	setOn(bool onoff, const Ice::Current& current);
static	CoolerPrx	createProxy(const std::string& coolername,
				const Ice::Current& current);
	// dew heater stuff
	bool	hasDewHeater(const Ice::Current& current);
	float	getDewHeater(const Ice::Current& current);
	void	setDewHeater(float dewheatervalue, const Ice::Current& current);
	Interval	dewHeaterRange(const Ice::Current& current);
	// callback stuff
private:
	SnowCallback<CoolerCallbackPrx>	callbacks;
public:
	virtual void	registerCallback(const Ice::Identity& callback,
				const Ice::Current& current);
        virtual void	unregisterCallback(const Ice::Identity& callback,
				const Ice::Current& current);
	void	callbackUpdate(const astro::callback::CallbackDataPtr data);
};

class CoolerICallback : public astro::callback::Callback {
	CoolerI&	_cooler;
public:
	CoolerICallback(CoolerI& cooler) : _cooler(cooler) { }
	virtual astro::callback::CallbackDataPtr	operator()(
		astro::callback::CallbackDataPtr data) {
		_cooler.callbackUpdate(data);
		return data;
	}
};


} // namespace snowstar

#endif /* _CoolerI_h */
