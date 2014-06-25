/*
 * CalibrationPointCallback.cpp -- callback for calibration points
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Guider_impl.h>
#include <AstroGuiding.h>
#include <AstroCallback.h>
#include <CalibrationPersistence.h>
#include <Conversions.h>
#include <CalibrationPointCallback.h>
#include <ServerDatabase.h>

namespace Astro {

/**
 * \brief Create Callback object
 *
 * Creating the callback also creates a new calibration record to which 
 * the clibration points added later can refer.
 */
CalibrationPointCallback::CalibrationPointCallback(Guider_impl& guider)
	: _guider(guider) {
	// prepare a calibration object
	astro::guiding::Calibration	calibration;
	calibration.camera = guider.getCameraName();
	calibration.ccdid = guider.getCcdid();
	calibration.guiderport = guider.getGuiderPortName();
	time(&calibration.when);
	for (int i = 0; i < 6; i++) { calibration.a[i] = 0; }

	// create a record 
	astro::guiding::CalibrationRecord	record(0, calibration);

	// add the record to the table
	astro::persistence::Database	db = ServerDatabase().database();
	astro::guiding::CalibrationTable	calibrationtable(db);
	_calibrationid = calibrationtable.add(record);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new calibration created: %d",
		_calibrationid);
}

using astro::guiding::GuiderCalibrationCallbackData;
using astro::guiding::CalibrationPointCallbackData;
using astro::callback::CallbackDataPtr;

/**
 * \brief Callback operator for calibration callback
 *
 * This callback can handle multiple argument types
 */
CallbackDataPtr	CalibrationPointCallback::operator()(CallbackDataPtr data) {
	// add a calibration point to the database
	CalibrationPointCallbackData	*calibrationpoint
		= dynamic_cast<CalibrationPointCallbackData *>(&*data);
	if (NULL != calibrationpoint) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "process a calibration point");
		// convert into a record
		astro::guiding::CalibrationPointRecord	record(0,
			_calibrationid, calibrationpoint->calibrationpoint());

		// add the record to the table
		astro::persistence::Database	db = ServerDatabase().database();
		astro::guiding::CalibrationPointTable	t(db);
		t.add(record);

		// also update the remote clients
		_guider.update(astro::convert(record));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration point added");
	}

	// handle the completion of the calibration
	GuiderCalibrationCallbackData	*calibration
		= dynamic_cast<GuiderCalibrationCallbackData *>(&*data);
	if (NULL != calibration) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "process the completed record");
		// when this callback is called, there already exists
		// a calibration record. So we just get this record...
		astro::persistence::Database	db = ServerDatabase().database();
		astro::guiding::CalibrationTable	t(db);
		astro::guiding::CalibrationRecord	record
			= t.byid(_calibrationid);

		// ... udpate it with the new data ..
		for (int i = 0; i < 6; i++) {
			record.a[i] = calibration->calibration().a[i];
		}

		// ... apply the changes to the table
		t.update(_calibrationid, record);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration point added");

		// inform any clients about the fact that calibration has
		// ended
		_guider.calibration_stop();
	}

	return data;
}

} // namespace Astro
