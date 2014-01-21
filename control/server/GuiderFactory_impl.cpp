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

extern astro::persistence::Database	database;

namespace Astro {

//////////////////////////////////////////////////////////////////////
// GuiderFactory implementation
//////////////////////////////////////////////////////////////////////

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
	astro::guiding::TrackingStore	store(database);
	return list2idlist(store.getTrackings(astro::convert(descriptor)));
}

/**
 * \brief Retrieve a list of guide run ids for a descriptor
 */
GuiderFactory::idlist	*GuiderFactory_impl::getAllGuideruns() {
	// get a Tracking store and retrieve a list of ids from it
	astro::guiding::TrackingStore	store(database);
	return list2idlist(store.getAllTrackings());
}

/**
 * \brief Retrieve a list of guide run ids for a descriptor
 */
GuiderFactory::idlist	*GuiderFactory_impl::getCalibrations(
				const Astro::GuiderDescriptor& descriptor) {
	// get a Tracking store and retrieve a list of ids from it
	astro::guiding::CalibrationStore	store(database);
	return list2idlist(store.getCalibrations(astro::convert(descriptor)));
}

/**
 * \brief Retrieve a list of calibrations for a descriptor
 */
GuiderFactory::idlist	*GuiderFactory_impl::getAllCalibrations() {
	// get a Tracking store and retrieve a list of ids from it
	astro::guiding::CalibrationStore	store(database);
	return list2idlist(store.getAllCalibrations());
}

/**
 * \brief Retrieve a guide history based on an id
 */
TrackingHistory	*GuiderFactory_impl::getTrackingHistory(CORBA::Long id) {
	return getTrackingHistory(id);
}

/**
 * \brief Retrieve a guide history based on an id
 */
TrackingHistory	*getTrackingHistory(CORBA::Long id) {
	TrackingHistory	*history = new TrackingHistory();
	double	now = astro::Timer::gettime();

	try {
		// get the database
		astro::guiding::GuidingRunTable	gt(database);
		astro::guiding::GuidingRunRecord	r = gt.byid(id);
		history->guiderunid = id;
		history->timeago = now - r.whenstarted;
		history->guider.cameraname
			= CORBA::string_dup(r.camera.c_str());
		history->guider.ccdid = r.ccdid;
		history->guider.guiderportname
			= CORBA::string_dup(r.guiderport.c_str());

		// get the tracking points
		astro::guiding::TrackingStore	store(database);
		std::list<astro::guiding::TrackingPointRecord>	points
			= store.getHistory(id);
		history->points.length(points.size());
		std::list<astro::guiding::TrackingPointRecord>::iterator i;
		int	j = 0;
		for (i = points.begin(); i != points.end(); i++, j++) {
			(history->points)[j] = astro::convert(*i);
		}

		// that's it, we have copied all the data
		return history;
	} catch (const std::exception& x) {
		delete history;
		char	buffer[1024];
		snprintf(buffer, sizeof(buffer),
			"tracking history %d not found: %s",
			id, x.what());
		throw NotFound(buffer);
	}
}

/**
 * \brief Get a calibration based on the id
 */
Astro::Calibration	*GuiderFactory_impl::getCalibration(CORBA::Long id) {
	return getCalibration(id);
}

/**
 * \brief Get a calibration based on the id
 */
Astro::Calibration	*getCalibration(CORBA::Long id) {
	Calibration	*calibration = new Calibration();
	try {
		double	now = astro::Timer::gettime();

		// get the calibration record
		astro::guiding::CalibrationTable	ct(database);
		astro::guiding::CalibrationRecord	r = ct.byid(id);
		calibration->id = id;
		calibration->timeago = now - r.when;
		calibration->guider.cameraname
			= CORBA::string_dup(r.camera.c_str());
		calibration->guider.ccdid = r.ccdid;
		calibration->guider.guiderportname
			= CORBA::string_dup(r.guiderport.c_str());
		calibration->coefficients[0] = r.a[0];
		calibration->coefficients[1] = r.a[1];
		calibration->coefficients[2] = r.a[2];
		calibration->coefficients[3] = r.a[3];
		calibration->coefficients[4] = r.a[4];
		calibration->coefficients[5] = r.a[5];

		// add all the calibration points
		astro::guiding::CalibrationStore	store(database);
		std::list<astro::guiding::CalibrationPointRecord>	points
			= store.getCalibrationPoints(id);
		calibration->points.length(points.size());
		std::list<astro::guiding::CalibrationPointRecord>::iterator i;
		int	j = 0;
		for (i = points.begin(); i != points.end(); i++, j++) {
			(calibration->points)[j] = astro::convert(*i);
		}

		// that's it, all data copied
		return calibration;
	} catch (std::runtime_error& x) {
		delete calibration;
		char	buffer[128];
		snprintf(buffer, sizeof(buffer),
			"calibration %d not found", id);
		throw NotFound(buffer);;
	}

}

} // namespace Astro
