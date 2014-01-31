/*
 * Camera_impl.cpp -- Corba Camera servant wrapper
 *
 * (c) 2013 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <Camera_impl.h>
#include "Ccd_impl.h"
#include "FilterWheel_impl.h"
#include "GuiderPort_impl.h"
#include <ServantBuilder.h>

namespace Astro {

/**
 * \brief Construct a camera implementation object
 */
Camera_impl::Camera_impl(astro::camera::CameraPtr camera) : _camera(camera) {
	for (unsigned int id = 0; id < _camera->nCcds(); id++) {
		ccds.push_back(_camera->getCcd(id));
	}
}

/**
 * \brief Get the name of the camera
 */
char	*Camera_impl::getName() {
	std::string	name = _camera->name();
	return CORBA::string_dup(name.c_str());
}

/**
 * \brief get the number of Ccds
 */
CORBA::Long	Camera_impl::nCcds() {
	return _camera->nCcds();
}

/**
 * \brief Retrieve CcdInfo from a camera
 */
CcdInfo	*Camera_impl::getCcdinfo(::CORBA::Long ccdid) {
	// make sure the ccdid exists
	if ((ccdid < 0) || (ccdid >= _camera->nCcds())) {
		debug(LOG_ERR, DEBUG_LOG, 0, "CCD id %d out of range", ccdid);
		NotFound	notfound;
		notfound.cause = (const char *)"CCD id out of range";
		throw notfound;
	}

	// get the CCD info
	astro::camera::CcdInfo	info = _camera->getCcdInfo(ccdid);

	// create a result structure,
	Astro::CcdInfo	*result = new Astro::CcdInfo();

	// copy simple members first
	std::string	ccdname = info.name();
	result->name = CORBA::string_dup(ccdname.c_str());
	result->id = info.getId();
	result->size.width = info.size().width();
	result->size.height = info.size().height();
	result->shutter = info.shutter();
	result->pixelwidth = info.pixelwidth();
	result->pixelheight = info.pixelheight();

	// copy binning modes
	astro::camera::BinningSet	binningset = info.modes();
	result->binningmodes.length(binningset.size());
	astro::camera::BinningSet::const_iterator	i;
	int	j;
	for (i = binningset.begin(), j = 0; i != binningset.end(); i++, j++) {
		Astro::BinningMode	*bm = new Astro::BinningMode();
		bm->x = ::CORBA::Long(i->getX());
		bm->y = ::CORBA::Long(i->getY());
		Astro::BinningMode_var	bmv = bm;
		result->binningmodes[j] = bmv;
	}

	// done
	return result;
}

/**
 * \brief Get a given Ccd
 */
Ccd_ptr	Camera_impl::getCcd(::CORBA::Long ccdid) {
	if ((ccdid < 0) || (ccdid >= (int)ccds.size())) {
		debug(LOG_ERR, DEBUG_LOG, 0, "request for nonexistant CCD %d",
			ccdid);
		NotFound	notfound;
		notfound.cause = (const char *)"CCD Id does not exist";
		throw notfound;
	}

        ServantBuilder<Ccd, Ccd_impl>     servantbuilder;
	astro::camera::CcdPtr	ccd = _camera->getCcd(ccdid);
        return servantbuilder(ccd);
}

/**
 * \brief Find out whether the Camera has a filter wheel
 */
bool	Camera_impl::hasFilterWheel() {
	return _camera->hasFilterWheel();
}

/**
 * \brief Get the Filter wheel
 */
FilterWheel_ptr	Camera_impl::getFilterWheel() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "requesting filter wheel");
	if (!_camera->hasFilterWheel()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "request filter wheel on "
			"camera that does not have one");
		NotImplemented	notimplemented;
		notimplemented.cause
			= (const char *)"camera does not have a filter wheel";
		throw notimplemented;
	}
	if (!filterwheel) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve filter wheel");
		filterwheel = _camera->getFilterWheel();
	}

	ServantBuilder<FilterWheel, FilterWheel_impl>	servantbuilder;
	return servantbuilder(filterwheel);
}

/**
 * \brief Find out whether there is a Guider Port on this camera
 */
bool	Camera_impl::hasGuiderPort() {
	return _camera->hasGuiderPort();
}

/**
 * \brief Get the GuiderPort
 */
GuiderPort_ptr	Camera_impl::getGuiderPort() {
	if (!_camera->hasGuiderPort()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "request guider port on "
			"camera that does not have one");
		NotImplemented	notimplemented;
		notimplemented.cause
			= (const char *)"camera does not have a guider port";
		throw notimplemented;
	}
	if (!guiderport) {
		guiderport = _camera->getGuiderPort();
	}
	ServantBuilder<GuiderPort, GuiderPort_impl>	servantbuilder;
	return servantbuilder(guiderport);
}

} // namespace Astro
