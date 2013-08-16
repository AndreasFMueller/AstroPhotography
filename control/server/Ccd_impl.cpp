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
