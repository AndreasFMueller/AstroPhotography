/*
 * QsiCcd.cpp -- QSI CCD implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */

#include <QsiCcd.h>
#include <QsiCooler.h>
#include <QsiUtils.h>
#include <AstroDebug.h>
#include <AstroExceptions.h>
#include <includes.h>

using namespace astro::image;

namespace astro {
namespace camera {
namespace qsi {

/**
 * \brief create a QsiCcd
 */
QsiCcd::QsiCcd(const CcdInfo& info, QsiCamera& camera)
	: Ccd(info), _camera(camera) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "construct CCD %s",
		getInfo().name().toString().c_str());
	// initialize the state variables
	std::unique_lock<std::recursive_mutex>	lock(_camera.mutex);

	// find out whether we can set the gain
	_cansetgain = false;
	_camera.camera().get_CanSetGain(&_cansetgain);
}

/**
 * \brief Destroy the CCD
 */
QsiCcd::~QsiCcd() {
	// turn off dependent devices
	if (_cooler) {
		QsiCooler	*cooler = dynamic_cast<QsiCooler*>(&*_cooler);
		if (cooler) {
			try {
				cooler->stop();
			} catch (const std::exception& x) {
				debug(LOG_ERR, DEBUG_LOG, 0, "cannot stop %s",
					x.what());
			} catch (...) {
				debug(LOG_ERR, DEBUG_LOG, 0, "cannot stop");
			}
		}
	}

	// abort an exposure in progress, if any
	try {
		if (CcdState::exposing == state()) {
			cancelExposure();
			wait();
		}
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot cancel: %s", x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot cancel");
	}
}

/**
 * \brief static trampoline method to run the thread
 */
void	QsiCcd::start_main(QsiCcd *qsiccd) noexcept {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting exposure thread");
	try {
		qsiccd->run();
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "thread throws %s: %s",
			demangle_cstr(x), x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "thread crashed");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure thread completes");
}

/**
 * \brief Wait for the exposurethread to complete
 */
void	QsiCcd::wait_thread() {
	if (_thread.joinable()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "waiting for thread");
		_thread.join();
	}
}

/**
 * \brief start an exposure
 *
 * \param exposure	exposure parameters
 */
void	QsiCcd::startExposure(const Exposure& exposure) {
	// before starting a new exposure, we should clean up the thread
	// if it is still around. The thread will set the state to exposed.
	wait_thread();

	// set the state to exposure, this also ensures we actually
	// are in a state where we could start a new exposure. The
	// base class method also sets the state to exposing
	Ccd::startExposure(exposure);

	// protect communication with the camera
	std::unique_lock<std::recursive_mutex>	lock(_camera.mutex);

	// now set up the exposure
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start QSI exposure");
	try {
		// set the binning mode
		START_STOPWATCH;
		_camera.camera().put_BinX(Ccd::exposure.mode().x());
		END_STOPWATCH("put_BinX()");
		START_STOPWATCH;
		_camera.camera().put_BinY(Ccd::exposure.mode().y());
		END_STOPWATCH("put_BinY()");

		// flip the origin
		int	height = getInfo().size().height();
		ImagePoint	origin = Ccd::exposure.frame().origin();
		ImageSize	size = Ccd::exposure.frame().size();
		origin.y(height - (origin.y() + size.height()));

		// compute the frame size in binned pixels, as this is what
		// the QSI camera expects
		origin = origin / Ccd::exposure.mode();
		size = size / Ccd::exposure.mode();

		// bild new image rectangle
		ImageRectangle	frame(origin, size);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "requesting %s image",
			frame.toString().c_str());

		// set the subframe
		START_STOPWATCH;
		_camera.camera().put_NumX(size.width());
		END_STOPWATCH("put_NumX()");
		START_STOPWATCH;
		_camera.camera().put_NumY(size.height());
		END_STOPWATCH("put_NumY()");
		START_STOPWATCH;
		_camera.camera().put_StartX(origin.x());
		END_STOPWATCH("put_StartX()");
		START_STOPWATCH;
		_camera.camera().put_StartY(origin.y());
		END_STOPWATCH("put_StartY()");

		// see whether we need to set the gain
		if (exposure.gain() >= 0) {
			QSICamera::CameraGain	gainvalue;
			if (exposure.gain() <= 0.25) {
				gainvalue = QSICamera::CameraGainLow;
			} else if (exposure.gain() <= 0.75) {
				gainvalue = QSICamera::CameraGainHigh;
			} else {
				gainvalue = QSICamera::CameraGainAuto;
			}
			_camera.camera().put_CameraGain(gainvalue);
		}

		// turn off the led
		debug(LOG_DEBUG, DEBUG_LOG, 0, "turn LED off");
		START_STOPWATCH;
		_camera.camera().put_LEDEnabled(false);
		END_STOPWATCH("put_LEDEnabled()");

		// get shutter info
		bool	light = (Ccd::exposure.shutter() == Shutter::OPEN);
		START_STOPWATCH;
		_camera.camera().StartExposure(Ccd::exposure.exposuretime(),
			light);
		END_STOPWATCH("StartExposure()()");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%.2fsec %s exposure started",
			Ccd::exposure.exposuretime(),
			(light) ? "light" : "dark");
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("exposure %s: %s",
			Ccd::exposure.toString().c_str(), x.what());
			
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		cancelExposure();
		throw BadParameter(msg.c_str());
	}

	// launch a thread waiting for the camera
	_thread = std::thread(start_main, this);
}

/**
 * \brief convert exposure state into a string
 *
 * \param qsistate	camera state code
 */
std::string	state2string(QSICamera::CameraState qsistate) {
	switch (qsistate) {
	case QSICamera::CameraIdle:
		return std::string("idle");
	case QSICamera::CameraWaiting:
		return std::string("waiting");
	case QSICamera::CameraExposing:
		return std::string("exposing");
	case QSICamera::CameraReading:
		return std::string("reading");
	case QSICamera::CameraDownload:
		return std::string("download");
	case QSICamera::CameraError:
		return std::string("error");
	}
	return std::string("unknonw");
}

/**
 * \brief main method for the exposure thread
 */
void	QsiCcd::run() {
	// find the time to completion
	double	endtime = Timer::gettime() + Ccd::exposure.exposuretime();

	//debug(LOG_DEBUG, DEBUG_LOG, 0, "get Ccd state");
	bool	imageReady = false;
	while (!imageReady) {
		// compute how long to sleep
		double	remaining = endtime - Timer::gettime();
		if (remaining < 0.1) {
			remaining = 0.1;
		}
		Timer::sleep(remaining);
		
		// lock the communication
		std::unique_lock<std::recursive_mutex>	lock(_camera.mutex);

		// get the camera state
		START_STOPWATCH;
		_camera.camera().get_ImageReady(&imageReady);
		END_STOPWATCH("get_ImageReady()");
	}
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "qsistate=%d", qsistate);

	// now the image is ready and we should indiate that with
	// the state going to exposed
	state(CcdState::exposed);
}

/**
 * \brief get the current camera state
 *
 * \return the current QSI state
 */
CcdState::State	QsiCcd::exposureStatus() {
	if (state() != CcdState::exposing) {
		wait_thread();
	}
	return state();
}

/**
 * \brief Cancel the current exposure
 */
void	QsiCcd::cancelExposure() {
	state(CcdState::cancelling);
	std::unique_lock<std::recursive_mutex>	lock(_camera.mutex);
	_camera.camera().AbortExposure();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "turn LED on");
	_camera.camera().put_LEDEnabled(true);
	_camera.camera().AbortExposure();
	state(CcdState::idle);
}

/**
 * \brief Get the state of the shutter
 */
Shutter::state	QsiCcd::getShutterState() {
	throw std::runtime_error("cannot query current shutter state");
}

/**
 * \brief set the Shutter state
 */
void	QsiCcd::setShutterState(const Shutter::state& /* state */) {
	throw std::runtime_error("cannot directly control shutter state");
}

/**
 * \brief Retrieve a raw image from the camera
 *
 * We can only get into this method if the state was exposed, i.e. 
 * the image is ready. This means that the ImageArry method will
 * work.
 */
ImagePtr	QsiCcd::getRawImage() {
	std::unique_lock<std::recursive_mutex>	lock(_camera.mutex);
	int	x, y, z;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "turn LED on");
	START_STOPWATCH;
	_camera.camera().put_LEDEnabled(true);
	END_STOPWATCH("put_LEDEnabled()");
	START_STOPWATCH;
	_camera.camera().get_ImageArraySize(x, y, z);
	END_STOPWATCH("put_ImageArraySize()");
	
	debug(LOG_DEBUG, DEBUG_LOG, 0, "x = %d, y = %d, z = %d", x, y, z);
	if (z != 2) {
		throw std::runtime_error("only ushort images supported");
	}
	ImageSize	size(x, y);
	Image<unsigned short>	*image = new Image<unsigned short>(size);
	ImagePtr	result(image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "reading image");
	try {
		START_STOPWATCH;
		_camera.camera().get_ImageArray(image->pixels);
		END_STOPWATCH("get_ImageArray()");
		image->flip(); // origin is in the upper left corner
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot read: %s", x.what());
	} catch (...) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "unknown read failre");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "read complete");
	return result;
}

/**
 * \brief get the cooler of this camera
 */
CoolerPtr	QsiCcd::getCooler0() {
	QsiCooler	*cooler = new QsiCooler(_camera);
	_cooler = CoolerPtr(cooler);
	return _cooler;
}

/**
 * \brief Retrieve the current gain value
 */
float	QsiCcd::getGain() {
	std::unique_lock<std::recursive_mutex>	lock(_camera.mutex);
	QSICamera::CameraGain	gainvalue;
	_camera.camera().get_CameraGain(&gainvalue);
	switch (gainvalue) {
	case QSICamera::CameraGainHigh:
		return 1;
	case QSICamera::CameraGainLow:
		return 0;
	case QSICamera::CameraGainAuto:
		return 0.5;
	default:
		debug(LOG_ERR, DEBUG_LOG, 0, "unknown camera gain setting");
		throw std::logic_error("unkonwn camera gain setting");
	}
}

/**
 * \brief Get the interval of valid gain values
 */
std::pair<float, float>	QsiCcd::gainInterval() {
	return std::make_pair((float)0, (float)1);
}

} // namespace qsi
} // namespace camera
} // namespace astro

