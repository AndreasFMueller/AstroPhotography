/*
 * Ccd_impl.cpp -- Corba Ccd implementation wrapper
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "Ccd_impl.h"
#include "Cooler_impl.h"

using namespace astro::camera;

namespace Astro {

::CORBA::Boolean	Ccd_impl::hasGain() {
	return _ccd->hasGain();
}

::CORBA::Boolean	Ccd_impl::hasShutter() {
	return _ccd->hasShutter();
}

ShutterState	Ccd_impl::getShutterState() {
	astro::camera::shutter_state	shutterstate = _ccd->getShutterState();
	switch (shutterstate) {
	case astro::camera::SHUTTER_OPEN:
		return Astro::SHUTTER_OPEN;
		break;
	case astro::camera::SHUTTER_CLOSED:
		return Astro::SHUTTER_CLOSED;
		break;
	}
	// XXX should not happen
}

void	Ccd_impl::setShutterState(ShutterState state) {
	astro::camera::shutter_state	shutterstate;
	switch (state) {
	case Astro::SHUTTER_OPEN:
		shutterstate = astro::camera::SHUTTER_OPEN;
		break;
	case Astro::SHUTTER_CLOSED:
		shutterstate = astro::camera::SHUTTER_CLOSED;
		break;
	}
	_ccd->setShutterState(shutterstate);
}

::CORBA::Boolean	Ccd_impl::hasCooler() {
	return _ccd->hasCooler();
}

Cooler_ptr	Ccd_impl::getCooler() {
	if (_ccd->hasCooler()) {
		// XXX bad things happen
	}
	CoolerPtr	cooler = _ccd->getCooler();

	// convert into a CORBA Cooler_ptr
	Cooler_impl	*coolerptr = new Cooler_impl(cooler);
	return coolerptr->_this();
}

} // namespace Astro
