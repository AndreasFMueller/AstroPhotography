/*
 * ServerDatabase.cpp -- server database implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <ServerDatabase.h>
#include <AstroDebug.h>
#include <AstroUtils.h>
#include <AstroGuiding.h>
#include <Conversions.h>
#include <TrackingStore.h>
#include <CalibrationStore.h>

namespace Astro {

static astro::persistence::Database	_database;

/**
 * \brief construct the database
 */
ServerDatabase::ServerDatabase(const std::string& databasefile) {
	astro::persistence::DatabaseFactory	factory;
	_database = factory.get(databasefile);
}

/**
 * \brief construct a database, i.e. use the precreated database
 */
ServerDatabase::ServerDatabase() {
}

/**
 * \brief Get the database
 */
astro::persistence::Database	ServerDatabase::database() {
	return _database;
}

/**
 * \brief Get a tracking history based on the id
 */
TrackingHistory	*ServerDatabase::getTrackingHistory(CORBA::Long id) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getTrackingHistory");
	TrackingHistory	*history = new TrackingHistory();
	double	now = astro::Timer::gettime();

	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve history %d", id);
		// get the database
		astro::guiding::GuidingRunTable	gt(_database);
		astro::guiding::GuidingRunRecord	r = gt.byid(id);
		history->guiderunid = id;
		history->timeago = now - r.whenstarted;
		history->guider.cameraname
			= CORBA::string_dup(r.camera.c_str());
		history->guider.ccdid = r.ccdid;
		history->guider.guiderportname
			= CORBA::string_dup(r.guiderport.c_str());

		// get the tracking points
		astro::guiding::TrackingStore	store(_database);
		std::list<astro::guiding::TrackingPointRecord>	points
			= store.getHistory(id);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "adding %d points",
			points.size());
		history->points.length(points.size());
		std::list<astro::guiding::TrackingPointRecord>::iterator i;
		int	j = 0;
		for (i = points.begin(); i != points.end(); i++, j++) {
			(history->points)[j] = astro::convert(*i);
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "points transferred");

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
// XXX This method is redundant, the CalibrationStore class now implements
// XXX essentially the same functionality. This method should be reimplemented
// XXX base on the calibration store
Astro::Calibration	*ServerDatabase::getCalibration(CORBA::Long id) {
	Calibration	*calibration = new Calibration();
	try {
		double	now = astro::Timer::gettime();

		// get the calibration record
		astro::guiding::CalibrationTable	ct(_database);
		astro::guiding::CalibrationRecord	r = ct.byid(id);
		calibration->id = id;
		calibration->timeago = now - r.when;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "time ago: %f",
			calibration->timeago);
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
		astro::guiding::CalibrationStore	store(_database);
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
