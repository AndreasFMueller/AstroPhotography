/*
 * FocuserI.h -- ICE Focuser class wrapper definition
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _FocuserI_h
#define _FocuserI_h

#include <camera.h>
#include <AstroCamera.h>
#include <DeviceI.h>
#include <CallbackHandler.h>

namespace snowstar {

template<>
void	callback_adapter<FocuserCallbackPrx>(FocuserCallbackPrx& p,
	const astro::callback::CallbackDataPtr data);

class FocuserICallback;
typedef std::shared_ptr<FocuserICallback>	FocuserICallbackPtr;

class FocuserI : virtual public Focuser, virtual public DeviceI {
	astro::camera::FocuserPtr	_focuser;
public:
	FocuserI(astro::camera::FocuserPtr focuser);
	virtual ~FocuserI();
	virtual int	min(const Ice::Current& current);
	virtual int	max(const Ice::Current& current);
	virtual int	backlash(const Ice::Current& current);
	virtual int	current(const Ice::Current& current);
	virtual void	set(int position, const Ice::Current& current);
private:
	SnowCallback<FocuserCallbackPrx>	callbacks;
public:
	virtual void	registerCallback(const Ice::Identity& callback,
				const Ice::Current& current);
	virtual void	unregisterCallback(const Ice::Identity& callback,
				const Ice::Current& current);
	void	callbackUpdate(const astro::callback::CallbackDataPtr data);
};

class FocuserICallback : public astro::callback::Callback {
	FocuserI&	_focuser;
public:
	FocuserICallback(FocuserI& focuser) : _focuser(focuser) { }
	virtual astro::callback::CallbackDataPtr	operator()(
		astro::callback::CallbackDataPtr data) {
		_focuser.callbackUpdate(data);
		return data;
	}
};

} // namespace snowstar

#endif /* _FocuserI_h */
