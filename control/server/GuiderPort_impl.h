/*
 * GuiderPort_impl.h -- Corba GuiderPort implementation wrapper
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _GuiderPort_impl_h
#define _GuiderPort_impl_h

#include <camera.hh>
#include <AstroCamera.h>

namespace Astro {

/**
 * \brief GuiderPort servant definition
 */
class GuiderPort_impl : public POA_Astro::GuiderPort {
	astro::camera::GuiderPortPtr	_guiderport;
public:
	typedef	astro::camera::GuiderPort	device_type;
	inline GuiderPort_impl(astro::camera::GuiderPortPtr guiderport)
		: _guiderport(guiderport) { }
	virtual CORBA::Char	active();
	virtual void	activate(::CORBA::Float ra, ::CORBA::Float dec);
	virtual char	*getName();
};

} // namespace Astro

#endif /* _GuiderPort_impl_h */
