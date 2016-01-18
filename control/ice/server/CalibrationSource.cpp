/*
 * CalibrationSource.cpp -- implementation of the calibration source
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "CalibrationSource.h"
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroGuiding.h>
#include <CalibrationPersistence.h>
#include <CalibrationStore.h>
#include <IceConversions.h>

namespace snowstar {

/**
 * \brief Create a creation source object
 */
CalibrationSource::CalibrationSource(astro::persistence::Database database)
	: _database(database) {
}

/**
 * \brief Get the calibration for a given id
 */
Calibration	CalibrationSource::get(int id) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get calibration %d", id);
	Calibration	calibration;
	try {
		astro::guiding::CalibrationTable	ct(_database);
		if (!ct.exists(id)) {
			NotFound	exception;
			exception.cause = astro::stringprintf("calibration %d "
				"does not exist", id);
			debug(LOG_ERR, DEBUG_LOG, 0, "%s",
				exception.cause.c_str());
			throw exception;
		}
		astro::guiding::CalibrationRecord	r = ct.byid(id);
		calibration.id = r.id();
		calibration.timeago = converttime(r.when);
		calibration.guider.instrumentname = r.instrument;
		calibration.guider.ccdIndex
			= instrumentName2index(r.instrument,
				InstrumentGuiderCCD, r.ccd);
		calibration.guider.guiderportIndex
			= instrumentName2index(r.instrument,
				InstrumentGuiderPort, r.controldevice);
		calibration.focallength = r.focallength;
		calibration.masPerPixel = r.masPerPixel;
		calibration.complete = (r.complete) ? true : false;
		calibration.det = r.det;
		calibration.quality = r.quality;
		for (int i = 0; i < 6; i++) {
			calibration.coefficients.push_back(r.a[i]);
		}
		switch (r.controltype) {
		case astro::guiding::BasicCalibration::GP:
			calibration.controltype = CalibrationTypeGuiderPort;
			break;
		case astro::guiding::BasicCalibration::AO:
			calibration.controltype = CalibrationTypeAdaptiveOptics;
			break;
		}

		// add calibration points
		astro::guiding::CalibrationStore	store(_database);
		std::list<astro::guiding::CalibrationPointRecord>	points
			= store.getCalibrationPoints(id);
		std::list<astro::guiding::CalibrationPointRecord>::iterator i;
		for (i = points.begin(); i != points.end(); i++) {
			calibration.points.push_back(convert(*i));
		}
		return calibration;
	} catch (std::exception& ex) {
		std::string	msg = astro::stringprintf("calibrationd run %d "
			"not found: %s", id, ex.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw NotFound(msg);
	}
}

} // namespace snowstar
