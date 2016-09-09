/*
 * CalibrationSource.cpp -- implementation of the calibration source
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroGuiding.h>
#include <IceConversions.h>
#include "CalibrationSource.h"

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
		astro::guiding::CalibrationStore	store(_database);
		// make sure the calibration actually exists
		if (!store.contains(id)) {
			std::string	msg = astro::stringprintf("calibration "
				"%d does not exist", id);
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}

		// now try to get the calibration
		astro::guiding::CalibrationPtr	cal = store.getCalibration(id);
		Calibration	calibration = convert(cal);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration %d time %f",
			calibration.id, calibration.timeago);

#if 0
		// add the calibration points, this is independent of the
		// control device type
		std::list<astro::guiding::CalibrationPointRecord>	points
			= store.getCalibrationPoints(id);
		std::list<astro::guiding::CalibrationPointRecord>::iterator i;
		for (i = points.begin(); i != points.end(); i++) {
			calibration.points.push_back(convert(*i));
		}
#endif

		// we have the calibration now, just return it
		return calibration;
	} catch (std::exception& ex) {
		std::string	msg = astro::stringprintf("calibration %d "
			"not found: %s", id, ex.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw NotFound(msg);
	}
}

} // namespace snowstar

