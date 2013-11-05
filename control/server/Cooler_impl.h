/*
 * Cooler_impl.h -- CORBA Cooler wrapper
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Cooler_impl_h
#define _Cooler_impl_h

#include <AstroCamera.h>
#include <camera.hh>

namespace Astro {

/**
 * \brief Cooler servant definition
 */
class Cooler_impl : public POA_Astro::Cooler {
	astro::camera::CoolerPtr	_cooler;
public:
	inline Cooler_impl(astro::camera::CoolerPtr& cooler) : _cooler(cooler) { }
	CORBA::Float	getSetTemperature();
	CORBA::Float	getActualTemperature();
	void	setTemperature(CORBA::Float temperature);
	bool	isOn();
	void	setOn(bool onoff);
	virtual char	*getName();
};

} // namespace astro

#endif /* _Cooler_impl_h */
