/*
 * calcmd.cpp -- calibration related commands
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "guide.h"
#include <cstdlib>
#include "display.h"
#include <iostream>
#include <AstroDebug.h>
#include <AstroFormat.h>

namespace snowstar {
namespace app {
namespace snowguide {

/**
 * \brief Implementation of the cancel command
 *
 * This command cancels a calibration process or a guiding process.
 */
int	Guide::cancel_command(GuiderPrx guider) {
	if (guider->getState() == GuiderCALIBRATING) {
		guider->cancelCalibration();
		return EXIT_SUCCESS;
	}
	if (guider->getState() == GuiderGUIDING) {
		guider->stopGuiding();
		return EXIT_SUCCESS;
	}
	std::cerr << "nothing to cancel, wrong state" << std::endl;
	return EXIT_FAILURE;
}

/**
 * \brief Display calibration
 *
 * This command retrieves the calibration information from the guider
 * and displays it
 */
int	Guide::calibration_command(GuiderFactoryPrx guiderfactory,
		GuiderPrx guider, int calibrationid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieving calibration %d",
		calibrationid);
	Calibration	cal;
	if (calibrationid <= 0) {
		switch (guider->getState()) {
		case GuiderUNCONFIGURED:
		case GuiderIDLE:
		case GuiderCALIBRATING:
			std::cerr << "not calibrated, specify calibration id";
			std::cerr << std::endl;
			return EXIT_FAILURE;
			break;
		case GuiderCALIBRATED:
		case GuiderGUIDING:
			cal = guider->getCalibration();
			break;
		}
	} else {
		cal = guiderfactory->getCalibration(calibrationid);
	}

	Calibration_display	cd(std::cout);
	cd(cal);
	std::cout << std::endl;
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the list command
 */
int	Guide::list_command(GuiderFactoryPrx guiderfactory,
		GuiderDescriptor descriptor) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get calibrations from remote server");
	idlist	l = guiderfactory->getCalibrations(descriptor);
	std::cout << "number of calibrations: " << l.size() << std::endl;
	idlist::iterator	i;
	for (i = l.begin(); i != l.end(); i++) {
		Calibration	cal = guiderfactory->getCalibration(*i);
		(Calibration_display(std::cout))(cal);
	}
	return EXIT_SUCCESS;
}

/**
 * \brief Remove calibrations
 */
int	Guide::trash_command(GuiderFactoryPrx guiderfactory, std::list<int> ids) {
	std::list<int>::const_iterator	i;
	for (i = ids.begin(); i != ids.end(); i++) {
		try {
			guiderfactory->deleteCalibration(*i);
		} catch (const NotFound& x) {
			std::cerr << "cannot delete calibration " << *i << ": ";
			std::cerr << x.cause << std::endl;
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of calibrate command
 */
int	Guide::calibrate_command(GuiderPrx guider, int calibrationid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "use calibrationid = %d",
		calibrationid);
	if (calibrationid > 0) {
		guider->useCalibration(calibrationid);
		return EXIT_SUCCESS;
	} else {
		if ((star.x == 0) && (star.y == 0)) {
			throw std::runtime_error("calibration star not set");
		}
	}
	if (method != TrackerUNDEFINED) {
		guider->setTrackerMethod(method);
	}
	calibrationid = guider->startCalibration();
	std::cout << "new calibration " << calibrationid << " in progress";
	std::cout << std::endl;
	return EXIT_SUCCESS;
}

} // namespace snowguide
} // namespace app
} // namespace snowstar
