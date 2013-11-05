/*
 * FilterWheel_impl.h -- Corba filterWheel implementation wrapper
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _FilterWheel_impl_h
#define _FilterWheel_impl_h

#include <camera.hh>
#include <AstroCamera.h>

namespace Astro {

/**
 * \brief Filterwheel servant definition
 */
class FilterWheel_impl : public POA_Astro::FilterWheel {
	astro::camera::FilterWheelPtr	_filterwheel;
public:
	inline	FilterWheel_impl(astro::camera::FilterWheelPtr filterwheel)
		: _filterwheel(filterwheel) { }
	virtual ::CORBA::Long	nFilters();
	virtual ::CORBA::Long	currentPosition();
	virtual void	select(::CORBA::Long position);
	virtual char	*filterName(::CORBA::Long position);
	virtual char	*getName();
};

} // namespace Astro

#endif /* _FilterWheel_impl_h */
