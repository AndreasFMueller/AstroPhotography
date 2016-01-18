/*
 * ControlDevice.cpp -- device 
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include <BasicProcess.h>
#include <CalibrationStore.h>
#include <CalibrationProcess.h>

namespace astro {
namespace guiding {

/**
 * \brief Create a new control device
 */
ControlDeviceBase::ControlDeviceBase(const std::string& instrumentname,
	camera::Imager& imager, persistence::Database database)
	: _instrument(instrumentname), _imager(imager), _database(database) {
	_calibrationid = 0;
	pcal = new PersistentCalibration;
	pcal->instrument = instrument();
	pcal->ccd = ccdname();
	pcal->focallength = 0;
	pcal->masPerPixel = 0;
	pcal->controltype = BasicCalibration::GP;
}

/**
 * \brief destroy the control device
 */
ControlDeviceBase::~ControlDeviceBase() {
	delete pcal;
	pcal = NULL;
}

/**
 * \brief start the calibration
 */
int	ControlDeviceBase::startCalibration(TrackerPtr /* tracker */) {
	if (!_database) {
		return 0;
	}
	pcal->controldevice = devicename();
	CalibrationRecord	record(0, *pcal);
	CalibrationTable	calibrationtable(_database);
	_calibrationid = calibrationtable.add(record);
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"saved %s calibration record id = %d", _calibrationid,
		BasicCalibration::type2string((BasicCalibration::CalibrationType)(pcal->controltype)).c_str());
	return _calibrationid;
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
	*pcal = cal;
	if (!_database) {
		return;
	}
	CalibrationStore	calstore(_database);
	calstore.saveCalibration(_calibrationid, cal);
}

//////////////////////////////////////////////////////////////////////
// Specialization to GuiderPort
//////////////////////////////////////////////////////////////////////
template<>
int	ControlDevice<camera::GuiderPort, GuiderCalibration>::startCalibration(
		TrackerPtr tracker) {
	pcal->controltype = BasicCalibration::GP;
	pcal->focallength = focallength();

	// compute the pixel size
	astro::camera::CcdInfo	info = imager().ccd()->getInfo();
        double   pixelsize = (info.pixelwidth() * exposure().mode().x()
                        + info.pixelheight() * exposure().mode().y()) / 2.;
        debug(LOG_DEBUG, DEBUG_LOG, 0, "pixelsize: %.2fum",
                1000000 * pixelsize);
	pcal->masPerPixel = (pixelsize / focallength())
				* (180*3600*1000 / M_PI);

	// XXX this does not work, because this cannot be a Guider*
	CalibrationProcess	*calibrationprocess
		= new CalibrationProcess((Guider *)this, tracker, _database);
	process = BasicProcessPtr(calibrationprocess);

	calibrationprocess->focallength(focallength());
	calibrationprocess->pixelsize(pixelsize);

	return ControlDeviceBase::startCalibration(tracker);
}

//////////////////////////////////////////////////////////////////////
// Specialization to AdaptiveOptics
//////////////////////////////////////////////////////////////////////
template<>
int	ControlDevice<camera::AdaptiveOptics, AdaptiveOpticsCalibration>::startCalibration(
		TrackerPtr tracker) {
	pcal->controltype = BasicCalibration::AO;

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
