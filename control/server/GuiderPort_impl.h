/*
 * GuiderPort_impl.h -- Corba GuiderPort implementation wrapper
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _GuiderPort_impl_h
#define _GuiderPort_impl_h

#include <device.hh>
#include <AstroCamera.h>

namespace Astro {

/**
 * \brief GuiderPort servant definition
 */
class GuiderPort_impl : public POA_Astro::GuiderPort {
	astro::camera::GuiderPortPtr	_guiderport;
public:
	inline GuiderPort_impl(astro::camera::GuiderPortPtr guiderport)
		: _guiderport(guiderport) { }
	virtual CORBA::Char	active();
	virtual void	activate(::CORBA::Float ra, ::CORBA::Float dec);
};

} // namespace Astro

#endif /* _GuiderPort_impl_h */
