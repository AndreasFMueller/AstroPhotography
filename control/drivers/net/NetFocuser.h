/*
 * NetFocuser.h -- corba/network based focuser driver
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _NetFocuser_h
#define _NetFocuser_h

#include <AstroCamera.h>
#include <device.hh>

namespace astro {
namespace camera {
namespace net {

/**
 * \brief Network client for a Focuser
 */
class NetFocuser : public Focuser {
	Astro::Focuser_var	_focuser;
public:
	NetFocuser(Astro::Focuser_var focuser);
	~NetFocuser();
	virtual unsigned short  min();
	virtual unsigned short  max();
	virtual unsigned short  current();
	virtual void	set(unsigned short value);
};

} // namespace net
} // namespace camera
} // namespace astro

#endif /* _NetFocuser_h */
