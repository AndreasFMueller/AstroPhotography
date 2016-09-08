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
#include <IceConversions.h>

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
		GuiderPrx guider) {
	return calibration_command(guiderfactory, guider, std::string("GP"));
}

int	Guide::calibration_command(GuiderFactoryPrx guiderfactory,
		GuiderPrx guider, const std::string& calarg) {
	// try to interpret the calarg as a calibration type
	ControlType	caltype;
	try {
		caltype = string2calibrationtype(calarg);
	} catch (const std::exception& x) {
		goto numericcalibration;
	}
	// we were successful in interpreting the calibration type, but
	// this only makes sens in certain states
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
		calibration_show(guider->getCalibration(caltype));
		break;
	}
	return EXIT_SUCCESS;

numericcalibration:
	// handle the case where we assume that the argument is a numeric
	// calibration id
	try {
		int	calid = std::stoi(calarg);
		return calibration_command(guiderfactory, calid);
	} catch (const std::exception& x) {
		std::cerr << "cannot retrieve calibration '" << calarg << "': ";
		std::cerr << x.what() << std::endl;
	}
	return EXIT_FAILURE;
}

int	Guide::calibration_command(GuiderFactoryPrx guiderfactory,
		int calibrationid) {
	Calibration	cal = guiderfactory->getCalibration(calibrationid);
	calibration_show(cal);
	return EXIT_SUCCESS;
}

void	Guide::calibration_show(const Calibration& cal) {
	Calibration_display	cd(std::cout);
	cd.verbose(verbose);
	cd(cal);
	std::cout << std::endl;
}

/**
 * \brief Implementation of the list command
 */
int	Guide::list_command(GuiderFactoryPrx guiderfactory,
		GuiderDescriptor descriptor) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get calibrations from remote server");
	idlist	l = guiderfactory->getCalibrations(descriptor,
			ControlGuidePort);
	std::cout << "number of guider port calibrations: " << l.size()
		<< std::endl;
	idlist::iterator	i;
	for (i = l.begin(); i != l.end(); i++) {
		Calibration	cal = guiderfactory->getCalibration(*i);
		Calibration_display	cd(std::cout);
		cd.verbose(verbose);
		cd(cal);
	}
	l = guiderfactory->getCalibrations(descriptor, ControlAdaptiveOptics);
	std::cout << "number of adaptive optics calibrations: " << l.size()
		<< std::endl;
	for (i = l.begin(); i != l.end(); i++) {
		Calibration	cal = guiderfactory->getCalibration(*i);
		Calibration_display	cd(std::cout);
		cd.verbose(verbose);
		cd(cal);
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
int	Guide::calibrate_command(GuiderPrx guider) {
	return calibrate_command(guider, std::string("GP"));
}

int	Guide::calibrate_command(GuiderPrx guider, int calibrationid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "use calibrationid = %d",
		calibrationid);
	if (calibrationid > 0) {
		guider->useCalibration(calibrationid, flipped);
		return EXIT_SUCCESS;
	} else {
		return calibrate_command(guider);
	}
}

int	Guide::calibrate_command(GuiderPrx guider, const std::string& calarg) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibrate with arg '%s'",
		calarg.c_str());
	// make sure the tracker is configured
	if (method != TrackerUNDEFINED) {
		guider->setTrackerMethod(method);
	}
	if (method == TrackerSTAR) {
		if ((star.x == 0) && (star.y == 0)) {
			debug(LOG_WARNING, DEBUG_LOG, 0,
				"warning: calibration star not set");
		}
	}

	// try to interpret the argument as a number
	int	calibrationid;
	try {
		calibrationid = std::stoi(calarg);
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "argument is not an id");
		goto newcalibration;
	}
	guider->useCalibration(calibrationid, flipped);
	return EXIT_SUCCESS;

newcalibration:
	// try to interpret the argument as a calibration type
	try {
		ControlType	caltype = string2calibrationtype(calarg);
		calibrationid = guider->startCalibration(caltype);
		std::cout << "new calibration " << calibrationid;
		std::cout << " in progress" << std::endl;
		return EXIT_SUCCESS;
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"calibration failed for type %s: %s",
			calarg.c_str(), x.what());
	}
	std::cerr << "calibration failed" << std::endl;
	return EXIT_FAILURE;
}

int	Guide::uncalibrate_command(GuiderPrx guider, ControlType type) {
	try {
		guider->unCalibrate(type);
		return EXIT_SUCCESS;
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot uncalibrate: %s",
			x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot uncalibrate");
	}
	return EXIT_FAILURE;
}

int	Guide::flip_command(GuiderPrx guider, ControlType type) {
	try {
		guider->flipCalibration(type);
		return EXIT_SUCCESS;
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot uncalibrate: %s",
			x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot uncalibrate");
	}
	return EXIT_FAILURE;
}

int	Guide::flip_command(GuiderPrx guider) {
	int rc = flip_command(guider, ControlGuidePort);
	if (rc != EXIT_SUCCESS) {
		return rc;
	}
	return flip_command(guider, ControlAdaptiveOptics);
}

} // namespace snowguide
} // namespace app
} // namespace snowstar
