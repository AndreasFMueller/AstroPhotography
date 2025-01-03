/*
 * ControlDeviceBase.cpp -- implementation of base class of control devices
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include "BasicProcess.h"
#include "CalibrationProcess.h"
#include "AOCalibrationProcess.h"
#include <algorithm>
#include "ControlDeviceCallback.h"
#include "CalibrationPersistence.h"

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
	_guider->addCalibrationCallback(_callback);
	_calibration = NULL;
}

/**
 * \brief destroy the control device
 */
ControlDeviceBase::~ControlDeviceBase() {
	_guider->removeCalibrationCallback(_callback);
}

/**
 * \brief template to wrap around typeid to silence clang
 *
 * see the calibrationid method below
 */
template<typename T>
std::type_index	get_type_index(T const& obj) {
	std::type_index	idx = typeid(obj);
	return idx;
}

/**
 * \brief Retrieve the calibration
 */
void	ControlDeviceBase::calibrationid(int calid, bool meridian_flip) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set calibration: %d, meridian_flip=%s",
		calid, (meridian_flip) ? "true" : "false");

	// handle special case: calid < 0 indicates that we want to remove
	// the calibration
	if (calid <= 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "uncalibrating %s",
			deviceType().name());
		if (_calibration) {
			_calibration->reset();
		}
		return;
	}

	// we need a calibration from the store
	CalibrationStore	store(_database);
	
	// get the type of the calibration
	std::type_index	type = get_type_index(*_calibration);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration type: %s", type.name());

	// get the calibration from the store
	CalibrationPtr	storedcal = store.getCalibration(calid);
	std::type_index	storedtype = get_type_index(*storedcal);

	// we have a problem if they are different
	if (type != storedtype) {
		std::string	cause = stringprintf("calibration %d has "
			"wrong type", calid);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
		throw std::runtime_error(cause);
	}

	// now copy stuff over
	*_calibration = *storedcal;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "meridian flip the calibration: %s",
		(meridian_flip) ? "yes" : "no");
	_calibration->meridian_flipped(meridian_flip);
}

int	ControlDeviceBase::calibrationid() const {
	if (NULL == _calibration) {
		return -1;
	}
	return _calibration->calibrationid();
}

bool	ControlDeviceBase::iscalibrated() const {
	return (calibrationid() > 0) ? true : false;
}

bool	ControlDeviceBase::flipped() const {
	if (_calibration) {
		return _calibration->flipped();
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "no calibration available");
	return false;
}

void	ControlDeviceBase::flip() {
	if (_calibration) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "flipping calibration");
		_calibration->flip();
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "cannot flip nonexistent calibration");
}

bool	ControlDeviceBase::meridian_flipped() const {
	if (_calibration) {
		return _calibration->meridian_flipped();
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "no calibration available");
	return false;
}

void	ControlDeviceBase::meridian_flip() {
	if (_calibration) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "meridian flipping calibration");
		return _calibration->meridian_flip();
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "cannot meridian flip nonexistent calibration");
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
 *
 * \param tracker	the tracker to use for the calibration
 */
int	ControlDeviceBase::startCalibration(TrackerPtr /* tracker */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "common calibration start");

        // reset the current calibration, just to make sure we don't confuse
        // it with the previous
	CalibrationProcess	*calibrationprocess
		= dynamic_cast<CalibrationProcess *>(&*process);
	if (NULL == calibrationprocess) {
		std::string	cause = stringprintf("not a calibration "
			"process: %s",
			demangle_string(*process).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
		throw std::runtime_error(cause);
	}
        _calibration = calibrationprocess->calibration();

        // set the focal length
        _calibration->focallength(parameter(std::string("focallength"), 1.0));
        debug(LOG_DEBUG, DEBUG_LOG, 0, "focallength = %.3f",
		_calibration->focallength());
	process->focallength(_calibration->focallength());

	// get the guider rate
	calibrationprocess->guiderate(parameter(std::string("guiderate"), 0.5));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "guiderate = %.3f", 
		calibrationprocess->guiderate());

	// get the suggested grid pixel size
	float	gridpixels = parameter(std::string("gridpixels"), 0.);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "suggested grid pixel size: %.1f",
		gridpixels);

	_calibration->east(parameter(std::string("telescope_east"), 1.) > 0);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "telescope position %s",
		_calibration->east() ? "east" : "west");
	double	declination = parameter(std::string("declination"), 0.0);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "declination=%.1f", declination);
	_calibration->declination(Angle(declination, Angle::Degrees));

	// compute angular size of pixels
	_calibration->masPerPixel(
		(_guider->pixelsize() / _calibration->focallength())
			* (180 * 3600 * 1000 / M_PI));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "masPerPixel = %.1f",
		_calibration->masPerPixel());

	// database related stuff
	if (_database) {
		// initialize the calibration as far as we can
		_calibration->calibrationid(0);
		if (configurationType() == typeid(GuiderCalibration)) {
			_calibration->calibrationtype(GP);
		}
		if (configurationType() == typeid(AdaptiveOpticsCalibration)) {
			_calibration->calibrationtype(AO);
		}
		CalibrationRecord	record(0, *_calibration);

		// set data describing the device
		record.instrument = _guider->instrument();
		record.ccd = _guider->ccdname();
		record.controldevice = devicename();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "quality: %f", record.quality);

		// resolution attributes
		record.focallength = _calibration->focallength();
		record.masPerPixel = _calibration->masPerPixel();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "record has masPerPixel = %.1f",
			record.masPerPixel);

		// record der Tabelle zufügen
		CalibrationTable	calibrationtable(_database);
		_calibration->calibrationid(calibrationtable.add(record));
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"saved %s calibration record id = %d",
			type2string(_calibration->calibrationtype()).c_str(),
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
void	ControlDeviceBase::saveCalibration() {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"received calibration %s to save as %d, %d points, masPerPixel =%.1f",
		_calibration->toString().c_str(), _calibration->calibrationid(),
		_calibration->size(), _calibration->masPerPixel());
	_calibrating = false;
	if (!_database) {
		return;
	}
	// update the calibration in the database
	CalibrationStore	calstore(_database);
	calstore.updateCalibration(_calibration);
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
