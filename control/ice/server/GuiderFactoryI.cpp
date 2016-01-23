/*
 * GuiderFactoryI.cpp -- guider factory servant implementation 
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <GuiderFactoryI.h>
#include <CalibrationStore.h>
#include <TrackingStore.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <GuiderI.h>
#include <ProxyCreator.h>
#include <IceConversions.h>
#include <AstroGuiding.h>
#include <CalibrationSource.h>
#include <AstroUtils.h>

namespace snowstar {

/**
 * \brief Create a GuiderFactory implementation object
 *
 * There will usually be only one instance of the factory. 
 * \param _database		the persistence database to use for calibrations
 *				and tracking histories
 * \param _guiderfactory	the factory to use to create the actual objects
 * \param _locator		Locator for guiders, used to store a guider
 *				requested by a guider (the factory only
 *				returns a proxy, which will be converted to
 *				to an actual object by the locator)
 * \param _imagedirectory	the image directory to be used by all guiders
 */
GuiderFactoryI::GuiderFactoryI(astro::persistence::Database _database,
		astro::guiding::GuiderFactory& _guiderfactory,
		GuiderLocator *_locator,
		ImageDirectory& _imagedirectory)
	: database(_database), guiderfactory(_guiderfactory),
	  locator(_locator), imagedirectory(_imagedirectory) {
}

/**
 * \brief Destroy the guider factory object
 */
GuiderFactoryI::~GuiderFactoryI() {
}

/**
 * \brief Get a list of all the guiders available on the server
 *
 * This method forwards the request to the original guider factory and
 * converts the result retrieved so that the ICE interface understands it
 */
GuiderList	GuiderFactoryI::list(const Ice::Current& /* current */) {
	std::vector<astro::guiding::GuiderDescriptor>	l
		= guiderfactory.list();
	GuiderList	result;
	std::vector<astro::guiding::GuiderDescriptor>::const_iterator	i;
	for (i = l.begin(); i != l.end(); i++) {
		result.push_back(convert(*i));
	}
	return result;
}

/**
 * \brief Get the proxy for a specific guider
 *
 * When a guider is created from the original guider factory, it must be
 * stored in the guider locator, whick keeps track of all guiders created
 * by this factory, and allows ICE to retrieve the guider when the client
 * tries to connect to it.
 */
GuiderPrx	GuiderFactoryI::get(const GuiderDescriptor& descriptor,
			const Ice::Current& current) {
	// name of the guider
	astro::guiding::GuiderDescriptor	d = convert(descriptor);
	std::string	gn = guiderdescriptor2name(descriptor);

	// get an GuiderPtr from the original factory
	astro::guiding::GuiderPtr	guider = guiderfactory.get(d);

	// get the focallength from the instrument properties
	try {
		astro::discover::InstrumentPtr	instrument
				= astro::discover::InstrumentBackend::get(
					descriptor.instrumentname);
		double	focallength
			= instrument->getDouble("guiderfocallength");
		guider->focallength(focallength);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "focallength: %.3f",
			focallength);
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no 'guiderfocallength' property "
			"found, using default %f", guider->focallength());
	}

	// create a GuiderI object
	Ice::ObjectPtr	guiderptr = new GuiderI(guider, imagedirectory,
		database);

	// add the guider we have constructed to the D
	locator->add(gn, guiderptr);

	// create a proxy
	std::string	ename = NameConverter::urlencode(gn);
	return createProxy<GuiderPrx>("guider/" + ename, current, false);
}

/**
 * \brief Get all calibrations stored in the database
 */
idlist	GuiderFactoryI::getAllCalibrations(const Ice::Current& /* current */) {
	astro::guiding::CalibrationStore	store(database);
	std::list<long> calibrations = store.getAllCalibrations();
	idlist	result;
	std::copy(calibrations.begin(), calibrations.end(),
		back_inserter(result));
	return result;
}

/**
 * \brief Get all the calibrations for a specific guider
 */
idlist	GuiderFactoryI::getCalibrations(const GuiderDescriptor& guider,
			const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get calibrations");
	astro::guiding::CalibrationStore	store(database);
	std::list<long> calibrations = store.getCalibrations(convert(guider));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got %d calibrations",
		calibrations.size());
	idlist	result;
	std::copy(calibrations.begin(), calibrations.end(),
		back_inserter(result));
	return result;
}

/**
 * \brief get details about a specific calibration
 */
Calibration	GuiderFactoryI::getCalibration(int id,
			const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve calibration %d", id);
	CalibrationSource	source(database);
	return source.get(id);
}

void	GuiderFactoryI::deleteCalibration(int id,
			const Ice::Current& /* current */) {
	astro::guiding::CalibrationStore	store(database);
	if (!store.contains(id)) {
		NotFound	exception;
		exception.cause = astro::stringprintf("calibration %d not found", id);
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot delete: %s", exception.cause.c_str());
		throw exception;
	}
	store.deleteCalibration(id);
}

/**
 * \brief Get all guide run ids available in the database
 */
idlist	GuiderFactoryI::getAllGuideruns(const Ice::Current& /* current */) {
	astro::guiding::TrackingStore	store(database);
	std::list<long>	trackings = store.getAllTrackings();
	idlist	result;
	std::copy(trackings.begin(), trackings.end(), back_inserter(result));
	return result;
}

/**
 * \brief Get the guide run ids for a specific guider
 */
idlist	GuiderFactoryI::getGuideruns(const GuiderDescriptor& guider,
			const Ice::Current& /* current */) {
	astro::guiding::TrackingStore	store(database);
	std::list<long>	trackings = store.getTrackings(convert(guider));
	idlist	result;
	std::copy(trackings.begin(), trackings.end(), back_inserter(result));
	return result;
}

/**
 * \brief Get the tracking history of a specific guide run
 *
 * Note that the result of this operation can be large. A guide run of an
 * hour with 5 updates per second (using an adaptive optics unit) contains
 * 18000 data points. Normal guide runs with 10 second update intervals
 * are quite manageable in size, about 360 points per hour of guiding.
 */
TrackingHistory	GuiderFactoryI::getTrackingHistory(int id,
			const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve history %d", id);
	try {
		astro::guiding::TrackingStore	store(database);
		TrackingHistory	history = convert(store.get(id));
		return history;
	} catch (const std::exception& ex) {
		std::string	msg = astro::stringprintf("tracking history %d "
			"not found: %s", id, ex.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw NotFound(msg);
	}
}

void	GuiderFactoryI::deleteTrackingHistory(int id,
		const Ice::Current& /* current */) {
	astro::guiding::TrackingStore	store(database);
	if (!store.contains(id)) {
		NotFound	exception;
		exception.cause = astro::stringprintf("tracking history %d not "
			"found", id);
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot delete: %s",
			exception.cause.c_str());
		throw exception;
	}
	store.deleteTrackingHistory(id);
}

} // namespace snowstar
