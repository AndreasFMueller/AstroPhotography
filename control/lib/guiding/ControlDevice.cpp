/*
 * ControlDevice.cpp -- device 
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include <BasicProcess.h>
#include <CalibrationStore.h>
#include <CalibrationProcess.h>
#include <AOCalibrationProcess.h>

using namespace astro::callback;

namespace astro {
namespace guiding {

/**
 * \brief Callback classes for control devices
 */
class ControlDeviceCallback : public Callback {
	ControlDeviceBase	*_controldevice;
public:
	ControlDeviceCallback(ControlDeviceBase *controldevice)
		: _controldevice(controldevice) {
	}
	CallbackDataPtr	operator()(CallbackDataPtr data);
};

/**
 * \brief processing method to process callback 
 */
CallbackDataPtr	ControlDeviceCallback::operator()(CallbackDataPtr data) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"control device callback called");
	// handle calibration point upates
	{
		CalibrationPointCallbackData	*cal
			= dynamic_cast<CalibrationPointCallbackData *>(&*data);
		if (NULL != cal) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration point: %s",
				cal->data().toString().c_str());
			_controldevice->addCalibrationPoint(cal->data());
			return data;
		}
	}
	// handle the calibration when it completes
	{
		GuiderCalibrationCallbackData   *cal
			= dynamic_cast<GuiderCalibrationCallbackData *>(&*data);
		if (NULL != cal) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration update");
			_controldevice->saveCalibration(cal->data());
			return data;
		}
	}
	// handle progress information
	{
		ProgressInfoCallbackData	*cal
			= dynamic_cast<ProgressInfoCallbackData *>(&*data);
		if (NULL != cal) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "progress update");
			if (cal->data().aborted) {
				_controldevice->calibrating(false);
			}
			return data;
		}
	}

	return data;
}

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
	CalibrationStore	store(_database);
	
	// get the type of the calibration
	std::type_index	type = typeid(*_calibration);

	// check for guider calibration
	if (type == typeid(GuiderCalibration)) {
		if (!store.contains(calid, BasicCalibration::GP)) {
			throw std::runtime_error("no such calibration id");
		}
		GuiderCalibration	*gcal
			= dynamic_cast<GuiderCalibration *>(_calibration);
		if (NULL == gcal) {
			return;
		}
		*gcal = store.getGuiderCalibration(calid);
		return;
	}

	// check for adaptive optics calibration
	if (type == typeid(AdaptiveOpticsCalibration)) {
		if (!store.contains(calid, BasicCalibration::AO)) {
			throw std::runtime_error("no such calibration id");
		}
		AdaptiveOpticsCalibration	*acal
			= dynamic_cast<AdaptiveOpticsCalibration *>(_calibration);
		if (NULL == acal) {
			return;
		}
		*acal = store.getAdaptiveOpticsCalibration(calid);
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration id = %d",
		calcopy.calibrationid());
	calcopy.calibrationid(_calibration->calibrationid());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration id = %d",
		calcopy.calibrationid());
	CalibrationStore	calstore(_database);
	calstore.updateCalibration(calcopy);
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
Point	ControlDeviceBase::correct(const Point& point, double /* Deltat */) {
	return point;
}

//////////////////////////////////////////////////////////////////////
// Specialization to GuiderPort
//////////////////////////////////////////////////////////////////////
template<>
int	ControlDevice<camera::GuiderPort,
		GuiderCalibration>::startCalibration(TrackerPtr tracker) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "GP calibration start");
	// reset the current calibration, just to make sure we don't confuse
	// it with the previous
	_calibration->reset();

	// set the focal length
	GuiderCalibration	*gcal
		= dynamic_cast<GuiderCalibration *>(_calibration);
	gcal->focallength = parameter(std::string("focallength"), 1.0);

	// create the calibration process
	CalibrationProcess	*calibrationprocess
		= new CalibrationProcess(_guider, _device, tracker, _database);
	process = BasicProcessPtr(calibrationprocess);

	// set the device specific
	calibrationprocess->focallength(gcal->focallength);

	// compute angular size of pixels
	gcal->masPerPixel = (_guider->pixelsize() / gcal->focallength)
				* (180 * 3600 * 1000 / M_PI);

	// start the process and update the record in the database
	return ControlDeviceBase::startCalibration(tracker);
}

/**
 * \brief apply a correction and send it to the GuiderPort
 */
template<>
Point	ControlDevice<camera::GuiderPort,
		GuiderCalibration>::correct(const Point& point, double Deltat) {
	// give up if not configured
	if (!_calibration->complete()) {
		return point;
	}

	// now compute the calibration
	Point	correction = _calibration->operator()(point, Deltat);

	// XXX apply the correction to the guider port
	debug(LOG_DEBUG, DEBUG_LOG, 0, "apply correction: %s",
		correction.toString().c_str());

	// no remaining error after a guider port correction ;-)
	return Point(0, 0);
}

//////////////////////////////////////////////////////////////////////
// Specialization to AdaptiveOptics
//////////////////////////////////////////////////////////////////////
template<>
int	ControlDevice<camera::AdaptiveOptics,
		AdaptiveOpticsCalibration>::startCalibration(
			TrackerPtr tracker) {
	_calibration->calibrationtype(BasicCalibration::AO);

	// create a new calibraiton process
	AOCalibrationProcess	*aocalibrationprocess
		= new AOCalibrationProcess(_guider, _device, tracker,
			_database);
	process = BasicProcessPtr(aocalibrationprocess);

	// start the calibration
	return ControlDeviceBase::startCalibration(tracker);
}

/**
 * \brief Apply correction to adaptive optics device
 */
template<>
Point	ControlDevice<camera::AdaptiveOptics,
		AdaptiveOpticsCalibration>::correct(const Point& point,
			double Deltat) {
	// give up if not configured
	if (!_calibration->complete()) {
		return point;
	}

	// now compute the calibration
	Point	correction = _calibration->operator()(point, Deltat);

	// get the current correction
	_device->set(_device->get() - correction);

	// get the remaining correction
	return _calibration->offset(_device->get());
}

} // namespace guiding
} // namespace astro
