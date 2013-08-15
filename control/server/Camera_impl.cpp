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

CcdInfo	*Camera_impl::getCcdinfo(::CORBA::Long ccdid) {
	// get the CCD info
	const astro::camera::CcdInfo&	info = _camera->getCcdInfo(ccdid);

	// create a result structure,
	Astro::CcdInfo	*result = new Astro::CcdInfo();

	// copy simple members first
	result->name = ::CORBA::string_dup(info.name().c_str());
	result->id = info.getId();
	result->size.width = info.size().width();
	result->size.height = info.size().height();
	result->shutter = info.shutter();

	// copy binning modes
	const astro::camera::BinningSet&	binningset = info.modes();
	result->binningmodes.length(binningset.size());
	astro::camera::BinningSet::const_iterator	i;
	int	j;
	for (i = binningset.begin(), j = 0; i != binningset.end(); i++, j++) {
		Astro::BinningMode	*bm = new BinningMode();
		bm->x = ::CORBA::Long(i->getX());
		bm->y = ::CORBA::Long(i->getY());
		Astro::BinningMode_var	bmv = bm;
		result->binningmodes[j++] = bmv;
	}

	// done
	return result;
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
