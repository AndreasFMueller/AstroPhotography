/*
 * NiceMount.h -- Nice mount driver
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _NiceMount_h
#define _NiceMount_h

#include <AstroDevice.h>
#include <NiceDevice.h>
#include <device.h>

namespace astro {
namespace device {
namespace nice {

class NiceMount : public astro::device::Mount {
	snowstar::MountPrx	_mount;
public:
	NiceMount(snowstar::MountPrx mount, const DeviceName& devicename);
	virtual ~NiceMount();

	virtual astro::device::Mount::state_type        state();
        virtual RaDec   getRaDec();
        virtual AzmAlt  getAzmAlt();
        virtual LongLat location();
        virtual time_t  time();
        virtual void    Goto(const RaDec& radec);
        virtual void    Goto(const AzmAlt& azmalt);
        virtual bool    telescopePositionWest();
	virtual location_source_type	location_source();
        virtual void    cancel();
};

} // namesapce nice
} // namespace device
} // namespace astro

#endif /* _NiceMount_h */
