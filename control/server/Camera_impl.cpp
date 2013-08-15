/*
 * Camera_impl.cpp -- Corba Camera servant wrapper
 *
 * (c) 2013 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <Camera_impl.h>
#include "Ccd_impl.h"
#include "FilterWheel_impl.h"
#include "GuiderPort_impl.h"

namespace Astro {

char	*Camera_impl::getName() {
	return strdup(_camera->getName().c_str());
}

CORBA::Long	Camera_impl::nCcds() {
	return _camera->nCcds();
}

Ccd_ptr	Camera_impl::getCcd(::CORBA::Long ccdid) {
	if ((ccdid < 0) || (ccdid >= (int)ccds.size())) {
		// XXX bad thing happens
	}
	Ccd_impl	*ccd = new Ccd_impl(ccds[ccdid]);
	return ccd->_this();
}

bool	Camera_impl::hasFilterWheel() {
	return _camera->hasFilterWheel();
}

FilterWheel_ptr	Camera_impl::getFilterWheel() {
	if (!filterwheel) {
		filterwheel = _camera->getFilterWheel();
	}
	FilterWheel_impl	*fw = new FilterWheel_impl(filterwheel);
	return fw->_this();
}

bool	Camera_impl::hasGuiderPort() {
	return _camera->hasGuiderPort();
}

GuiderPort_ptr	Camera_impl::getGuiderPort() {
	if (!guiderport) {
		guiderport = _camera->getGuiderPort();
	}
	GuiderPort_impl	*gp = new GuiderPort_impl(guiderport);
	return gp->_this();
}

} // namespace Astro
