/*
 * ControlDeviceBase.cpp -- implementation of base class of control devices
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include <BasicProcess.h>
#include <CalibrationStore.h>
#include <CalibrationProcess.h>
#include <AOCalibrationProcess.h>
#include <algorithm>
#include "ControlDeviceCallback.h"

using namespace astro::callback;

namespace astro {
namespace guiding {

/**
 * \brief Create a new control device
 */
ControlDeviceBase::ControlDeviceBase(GuiderBase *guider,
		persistence::Database database)
	: _guider(guider), _database(database) {
	_calibrating = false;
	ControlDeviceCallback	*cb = new ControlDeviceCallback(this);
	_callback = CallbackPtr(cb);
	_guider->addGuidercalibrationCallback(_callback);
	_guider->addCalibrationCallback(_callback);
}

/**
 * \brief destroy the control device
 */
ControlDeviceBase::~ControlDeviceBase() {
	_guider->removeGuidercalibrationCallback(_callback);
	_guider->removeCalibrationCallback(_callback);

	delete _calibration;
	_calibration = NULL;
}

/**
 * \brief Retrieve the calibration
 */
void	ControlDeviceBase::calibrationid(int calid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set calibration: %d", calid);

	// handle special case: calid < 0 indicates that we want to remove
	// the calibration
	if (calid <= 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "uncalibrating %s",
			deviceType().name());
		_calibration->reset();
		return;
	}

	// we need a calibration from the store
	CalibrationStore	store(_database);
	
	// get the type of the calibration
	std::type_index	type = typeid(*_calibration);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration type: %s", type.name());

	// check for guider calibration
	if (type == typeid(GuiderCalibration)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "GP calibration %d", calid);
		if (!store.containscomplete(calid, BasicCalibration::GP)) {
			throw std::runtime_error("no such calibration id");
		}
		GuiderCalibration	*gcal
			= dynamic_cast<GuiderCalibration *>(_calibration);
		if (NULL == gcal) {
			return;
		}
		*gcal = store.getGuiderCalibration(calid);
		gcal->calibrationid(calid);
		return;
	}

	// check for adaptive optics calibration
	if (type == typeid(AdaptiveOpticsCalibration)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "AO calibration %d", calid);
		if (!store.containscomplete(calid, BasicCalibration::AO)) {
			throw std::runtime_error("no such calibration id");
		}
		AdaptiveOpticsCalibration	*acal
			= dynamic_cast<AdaptiveOpticsCalibration *>(_calibration);
		if (NULL == acal) {
			return;
		}
		*acal = store.getAdaptiveOpticsCalibration(calid);
		acal->calibrationid(calid);
		return;
	}
}

bool	ControlDeviceBase::iscalibrated() const {
	return (calibrationid() > 0) ? true : false;
}

bool	ControlDeviceBase::flipped() const {
	if (_calibration) {
		return _calibration->flipped();
	}
	return false;
}

void	ControlDeviceBase::flip() {
	if (_calibration) {
		return _calibration->flip();
	}
}

void	ControlDeviceBase::addCalibrationPoint(const CalibrationPoint& point) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ADD CALIBRATION POINT %d %s",
		_calibration->calibrationid(), point.toString().c_str());
	if (!_calibrating) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "not calibrating!");
		return;
	}
	_calibration->add(point);
	CalibrationStore	store(_database);
	store.addPoint(_calibration->calibrationid(), point);
}

const std::string&	ControlDeviceBase::name() const {
	return _guider->name();
}

const std::string&	ControlDeviceBase::instrument() const {
	return _guider->instrument();
}

camera::Imager&	ControlDeviceBase::imager() {
	return _guider->imager();
}

std::string	ControlDeviceBase::ccdname() const {
	return _guider->ccdname();
}

const camera::Exposure&	ControlDeviceBase::exposure() const {
	return _guider->exposure();
}

void	ControlDeviceBase::exposure(const camera::Exposure& e) {
	_guider->exposure(e);
}

/**
 * \brief start the calibration
 */
int	ControlDeviceBase::startCalibration(TrackerPtr /* tracker */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "common calibration start");
	if (_database) {
		// initialize the calibration as far as we can
		_calibration->calibrationid(0);
		if (configurationType() == typeid(GuiderCalibration)) {
			_calibration->calibrationtype(BasicCalibration::GP);
		}
		if (configurationType() == typeid(AdaptiveOpticsCalibration)) {
			_calibration->calibrationtype(BasicCalibration::AO);
		}
		CalibrationRecord	record(0, *_calibration);

		// set data describing the device
		record.name = _guider->name();
		record.instrument = _guider->instrument();
		record.ccd = _guider->ccdname();
		record.controldevice = devicename();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "quality: %f", record.quality);

		// add specific attributes
		GuiderCalibration	*gcal
			= dynamic_cast<GuiderCalibration *>(_calibration);
		if (NULL != gcal) {
			record.focallength = gcal->focallength;
			record.masPerPixel = gcal->masPerPixel;
		}

		// record der Tabelle zufügen
		CalibrationTable	calibrationtable(_database);
		_calibration->calibrationid(calibrationtable.add(record));
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"saved %s calibration record id = %d",
			BasicCalibration::type2string(
				_calibration->calibrationtype()).c_str(),
			_calibration->calibrationid());
	}

	// start the process
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting process");
	process->start();
	_calibrating = true;

	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration %d started", 
		_calibration->calibrationid());
	return _calibration->calibrationid();
}

/**
 * \brief cancel the calibration process
 */
void	ControlDeviceBase::cancelCalibration() {
	process->stop();
}

/**
 * \brief wait for the calibration to complete
 */
bool	ControlDeviceBase::waitCalibration(double timeout) {
	return process->wait(timeout);
}

/**
 * \brief save a guider calibration
 */
void	ControlDeviceBase::saveCalibration(const BasicCalibration& cal) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"received calibration %s to save as %d, %d points",
		cal.toString().c_str(), _calibration->calibrationid(),
		cal.size());
	_calibrating = false;
	if (!_database) {
		return;
	}
	// update the calibration in the database
	BasicCalibration	calcopy = cal;
	calcopy.calibrationid(_calibration->calibrationid());
	calcopy.complete(true);
	if (calcopy.calibrationid() <= 0) {
		return;
	}
	CalibrationStore	calstore(_database);
	calstore.updateCalibration(calcopy);
	*_calibration = calcopy;
}

/**
 * \brief Check whether a parameter exists
 */
bool	ControlDeviceBase::hasParameter(const std::string& name) const {
	std::map<std::string, double>::const_iterator	i
		= parameters.find(name);
	return (i != parameters.end());
}

/**
 * \brief return value associated with a parameter
 */
double	ControlDeviceBase::parameter(const std::string& name) const {
	if (!hasParameter(name)) {
		std::string	cause = stringprintf("no value for '%s'",
			name.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
		throw std::runtime_error(cause);
	}
	return parameters.find(name)->second;
}

/**
 * \brief return value associated with a parameter, with default if not present
 */
double	ControlDeviceBase::parameter(const std::string& name, double value) const {
	if (!hasParameter(name)) {
		return value;
	}
	return parameters.find(name)->second;
}

/**
 * \brief set a parameter value
 */
void	ControlDeviceBase::setParameter(const std::string& name, double value) {
	if (hasParameter(name)) {
		parameters[name] = value;
		return;
	}
	parameters.insert(std::make_pair(name, value));
}

/**
 * \brief Computing the correction for the base: no correction
 */
Point	ControlDeviceBase::correct(const Point& point, double /* Deltat */,
		bool /* stepping */) {
	return point;
}

} // namespace guiding
} // namespace astro
