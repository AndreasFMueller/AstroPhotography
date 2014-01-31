/*
 * GuiderFactory_impl.cpp -- implementation of the GuiderFactory servant
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <GuiderFactory_impl.h>
#include <Guider_impl.h>
#include <Conversions.h>
#include <OrbSingleton.h>
#include <PoaNameMap.h>
#include <TrackingStore.h>
#include <CalibrationStore.h>
#include <AstroUtils.h>
#include <ServerDatabase.h>

namespace Astro {

//////////////////////////////////////////////////////////////////////
// GuiderFactory implementation
//////////////////////////////////////////////////////////////////////

/**
 * \brief
 */
GuiderFactory_impl::~GuiderFactory_impl() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroying the guider factory");
}

/**
 * \brief create a list of available guiders
 */
GuiderFactory::GuiderList	*GuiderFactory_impl::list() {
	std::vector<astro::guiding::GuiderDescriptor>	l
		= _guiderfactory->list();
	// create a list to return
	return NULL;
}

/**
 * \brief build a guider from the descriptor
 */
Guider_ptr	GuiderFactory_impl::get(
	const Astro::GuiderDescriptor& descriptor) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"get guider from camera %s, ccd %ld, guiderport %s",
		(const char *)descriptor.cameraname,
		descriptor.ccdid,
		(const char *)descriptor.guiderportname);

	// build the guider name, which we also use as an object
	// identifier
	astro::guiding::GuiderDescriptor	gd(
		(const char *)descriptor.cameraname,
		descriptor.ccdid,
		(const char *)descriptor.guiderportname);
	std::string	guidername = gd.toString();
	PortableServer::ObjectId_var	oid
		= PortableServer::string_to_ObjectId(guidername.c_str());

	// get the poa with the right name
	OrbSingleton	orb;
	PoaName	name = poaname<Guider>();
	PortableServer::POA_var	poa = orb.findPOA(name);

	// try whether this is a servant that already exists
	try {
		CORBA::Object_var	obj = poa->id_to_reference(oid);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "guider %s already exists",
			guidername.c_str());
		return Guider::_narrow(obj);
	} catch (PortableServer::POA::ObjectNotActive&) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s has no servant yet",
			gd.toString().c_str());
	}
	
	// get a servant
	try {
		astro::guiding::GuiderPtr	guider
			= _guiderfactory->get(astro::convert(descriptor));
		Astro::Guider_impl	*g = new Astro::Guider_impl(guider);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "activating guider %s",
			guidername.c_str());
		poa->activate_object_with_id(oid, g);
		CORBA::Object_var	obj = poa->id_to_reference(oid);
		return Guider::_narrow(obj);
	} catch (...) {
		NotFound	notfound;
		notfound.cause = CORBA::string_dup("guider not found");
		throw notfound;
	}
}

static GuiderFactory::idlist	*list2idlist(const std::list<long>& ids) {
	GuiderFactory::idlist	*list = new GuiderFactory::idlist();
	list->length(ids.size());
	std::list<long>::const_iterator	i;
	int	j = 0;
	for (i = ids.begin(); i != ids.end(); i++, j++) {
		(*list)[j] = *i;
	}
	return list;
}

/**
 * \brief Retrieve a list of guide run ids for a descriptor
 */
GuiderFactory::idlist	*GuiderFactory_impl::getGuideruns(
				const Astro::GuiderDescriptor& descriptor) {
	// get a Tracking store and retrieve a list of ids from it
	astro::persistence::Database	database = ServerDatabase().database();
	astro::guiding::TrackingStore	store(database);
	return list2idlist(store.getTrackings(astro::convert(descriptor)));
}

/**
 * \brief Retrieve a list of guide run ids for a descriptor
 */
GuiderFactory::idlist	*GuiderFactory_impl::getAllGuideruns() {
	// get a Tracking store and retrieve a list of ids from it
	astro::persistence::Database	database = ServerDatabase().database();
	astro::guiding::TrackingStore	store(database);
	return list2idlist(store.getAllTrackings());
}

/**
 * \brief Retrieve a list of guide run ids for a descriptor
 */
GuiderFactory::idlist	*GuiderFactory_impl::getCalibrations(
				const Astro::GuiderDescriptor& descriptor) {
	// get a Tracking store and retrieve a list of ids from it
	astro::persistence::Database	database = ServerDatabase().database();
	astro::guiding::CalibrationStore	store(database);
	return list2idlist(store.getCalibrations(astro::convert(descriptor)));
}

/**
 * \brief Retrieve a list of calibrations for a descriptor
 */
GuiderFactory::idlist	*GuiderFactory_impl::getAllCalibrations() {
	// get a Tracking store and retrieve a list of ids from it
	astro::persistence::Database	database = ServerDatabase().database();
	astro::guiding::CalibrationStore	store(database);
	return list2idlist(store.getAllCalibrations());
}

/**
 * \brief Retrieve a guide history based on an id
 */
TrackingHistory	*GuiderFactory_impl::getTrackingHistory(CORBA::Long id) {
	return ServerDatabase().getTrackingHistory(id);
}

/**
 * \brief Get a calibration based on the id
 */
Astro::Calibration	*GuiderFactory_impl::getCalibration(CORBA::Long id) {
	return ServerDatabase().getCalibration(id);
}

} // namespace Astro
