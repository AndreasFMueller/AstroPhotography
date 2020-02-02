/*
 * MountI.h -- mount servant declaration
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _MountI_h
#define _MountI_h

#include <device.h>
#include <AstroDevice.h>
#include <DeviceI.h>
#include <CallbackHandler.h>

namespace snowstar {

template<>
void	callback_adapter<MountCallbackPrx>(MountCallbackPrx& p,
		const astro::callback::CallbackDataPtr data);

class MountI : virtual public Mount, virtual public DeviceI {
	astro::device::MountPtr	_mount;
public:
	MountI(astro::device::MountPtr mount);
	virtual ~MountI();
	MountI(const Mount&) = delete;
	MountI&	operator=(const MountI&) = delete;
	virtual RaDec	getRaDec(const Ice::Current& current);
	virtual AzmAlt	getAzmAlt(const Ice::Current& current);
	virtual LongLat	getLocation(const Ice::Current& current);
	virtual locationtype	getLocationSource(const Ice::Current& current);
	virtual Ice::Long	getTime(const Ice::Current& current);
	virtual void	GotoRaDec(const RaDec& radec,
				const Ice::Current& current);
	virtual void	GotoAzmAlt(const AzmAlt& azmalt,
				const Ice::Current& current);
	virtual bool	telescopePositionWest(const Ice::Current& current);
	virtual void	cancel(const Ice::Current& current);
	virtual mountstate	state(const Ice::Current& current);
	virtual bool	hasGuideRates(const Ice::Current& current);
	virtual RaDec	getGuideRates(const Ice::Current& current);
	virtual void	registerCallback(const Ice::Identity& mountcallback,
				const Ice::Current& current);
	virtual void	unregisterCallback(const Ice::Identity& mountcallback,
				const Ice::Current& current);
	// this method is used to channel callback data back to the
	// MountCallback
private:
	SnowCallback<MountCallbackPrx>	callbacks;
public:
	void	callbackUpdate(const astro::callback::CallbackDataPtr data);
};


/**
 * \brief Callback class for mount monitoring
 *
 * Instances if this class should be given to the mount callback to receive
 * callbacks
 */
class MountICallback : public astro::callback::Callback {
	MountI&	_mount;
public:
	MountICallback(MountI& mount) : _mount(mount) { }
	virtual astro::callback::CallbackDataPtr	operator()(
		astro::callback::CallbackDataPtr data) {
		_mount.callbackUpdate(data);
		return data;
	}
};

} // namespace snowstar

#endif /* _MountI_h */
