/*
 * GuidePortI.cpp --
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <GuidePortI.h>
#include <NameConverter.h>
#include <ProxyCreator.h>
#include <IceConversions.h>

namespace snowstar {

GuidePortI::GuidePortI(astro::camera::GuidePortPtr guideport)
		: DeviceI(*guideport), _guideport(guideport) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create a callback");
	GuidePortICallback	*guideportcallback
		= new GuidePortICallback(*this);
	GuidePortICallbackPtr	guideportcallbackptr(guideportcallback);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "install callback in guideport");
	_guideport->addCallback(guideportcallbackptr);
}

GuidePortI::~GuidePortI() {
}

Ice::Byte	GuidePortI::active(const Ice::Current& current) {
	CallStatistics::count(current);
	return _guideport->active();
}

void	GuidePortI::activate(float ra, float dec, const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "received activation %f/%f", ra, dec);
	double  raplus = 0, raminus = 0, decplus = 0, decminus = 0;
	if (ra > 0) {
		raplus = ra;
	} else {
		raminus = -ra;
	}
	if (dec > 0) {
		decplus = dec;
	} else {
		decminus = -dec;
	}
	astro::camera::GuidePortActivation	act(raplus, raminus, decplus, decminus);
	_guideport->activate(act);
}

GuidePortPrx	GuidePortI::createProxy(const std::string& guideportname,
			const Ice::Current& current) {
	return snowstar::createProxy<GuidePortPrx>(guideportname, current);
}

void	GuidePortI::registerCallback(const Ice::Identity& guideportcallback,
		const Ice::Current& current) {
	CallStatistics::count(current);
	try {
		callbacks.registerCallback(guideportcallback, current);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot register callback %s: %s",
			astro::demangle(typeid(x).name()).c_str(), x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cannot register callback, unknown reason");
	}
}

void	GuidePortI::unregisterCallback(const Ice::Identity& guideportcallback,
		const Ice::Current& current) {
	CallStatistics::count(current);
	try {
		callbacks.unregisterCallback(guideportcallback, current);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot unregister callback %s: %s",
			astro::demangle(typeid(x).name()).c_str(), x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cannot unregister callback, unknown reason");
	}
}

void	GuidePortI::callbackActivate(const astro::callback::CallbackDataPtr data) {
	try {
		callbacks(data);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot send callback: %s %s",
		astro::demangle(typeid(x).name()).c_str(), x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot send callback, unknown reason");
	}
}

template<>
void	callback_adapter<GuidePortCallbackPrx>(GuidePortCallbackPrx& p,
		const astro::callback::CallbackDataPtr data) {
	astro::camera::ActivationCallbackData	*gpa
		= dynamic_cast<astro::camera::ActivationCallbackData*>(&*data);
	if (gpa != NULL) {
		p->activate(convert(gpa->data()));
		return;
	}
	return;
}

} // namespace snowstar
