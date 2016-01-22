/*
 * ControlDevice.cpp -- device 
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include <BasicProcess.h>
#include <CalibrationStore.h>
#include <CalibrationProcess.h>

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
		"callback for calibration completion called");
	// handle the calibration
	{
		GuiderCalibrationCallbackData   *cal
			= dynamic_cast<GuiderCalibrationCallbackData *>(&*data);
		if (NULL != cal) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration update");
			_controldevice->saveCalibration(cal->data());
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
}

/**
 * \brief destroy the control device
 */
ControlDeviceBase::~ControlDeviceBase() {
	_guider->removeGuidercalibrationCallback(_callback);
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "received calibration to save as %d",
		_calibration->calibrationid());
	*_calibration = cal;
	if (!_database) {
		return;
	}
	CalibrationStore	calstore(_database);
	calstore.saveCalibration(*_calibration);
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

//////////////////////////////////////////////////////////////////////
// Specialization to GuiderPort
//////////////////////////////////////////////////////////////////////
template<>
int	ControlDevice<camera::GuiderPort, GuiderCalibration>::startCalibration(
		TrackerPtr tracker) {
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

template<>
void	ControlDevice<camera::GuiderPort, GuiderCalibration>::calibrationid(int calid) {
	// get the callibration from the database
	this->calibrationid(calid);
}

//////////////////////////////////////////////////////////////////////
// Specialization to AdaptiveOptics
//////////////////////////////////////////////////////////////////////
template<>
int	ControlDevice<camera::AdaptiveOptics, AdaptiveOpticsCalibration>::startCalibration(
		TrackerPtr tracker) {
	_calibration->calibrationtype(BasicCalibration::AO);

	// XXX Missing implementation
#if 0
	// create a new calibraiton process
	AOCalibrationProcess	aocalibrationprocess
		= new AOCalibrationProcess();
	process = BasicProcessPtr(aocalibrationprocess);
#endif

	// start the calibration
	return ControlDeviceBase::startCalibration(tracker);
}

} // namespace guiding
} // namespace astro
