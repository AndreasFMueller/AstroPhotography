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

namespace snowstar {

class MountI : virtual public Mount, virtual public DeviceI {
	astro::device::MountPtr	_mount;
public:
	MountI(astro::device::MountPtr mount);
	virtual ~MountI();
	virtual RaDec	getRaDec(const Ice::Current& current);
	virtual AzmAlt	getAzmAlt(const Ice::Current& current);
	virtual void	GotoRaDec(const RaDec& radec,
				const Ice::Current& current);
	virtual void	GotoAzmAlt(const AzmAlt& azmalt,
				const Ice::Current& current);
	virtual void	cancel(const Ice::Current& current);
	virtual mountstate	state(const Ice::Current& current);
};

} // namespace snowstar

#endif /* _MountI_h */
