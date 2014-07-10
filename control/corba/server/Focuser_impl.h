/*
 * Focuser_impl.h -- CORBA Focuser wrapper
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Focuser_impl_h
#define _Focuser_impl_h

#include <AstroCamera.h>
#include <camera.hh>

namespace Astro {

/**
 * \brief Focuser servant definition
 */
class Focuser_impl : public POA_Astro::Focuser {
	astro::camera::FocuserPtr	_focuser;
public:
	typedef astro::camera::Focuser	device_type;
	inline Focuser_impl(astro::camera::FocuserPtr& focuser)
		: _focuser(focuser) { }
	virtual CORBA::UShort	min();
	virtual CORBA::UShort	max();
	virtual CORBA::UShort	current();
	virtual CORBA::UShort	backlash();
	virtual void	set(CORBA::UShort value);
	virtual char	*getName();
};

} // namespace astro

#endif /* _Focuser_impl_h */
