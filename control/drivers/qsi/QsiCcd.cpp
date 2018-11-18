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
	std::lock_guard<std::recursive_mutex>	lock(_camera.mutex);
	_last_state = CcdState::idle;
	_thread = NULL;
}

/**
 * \brief Destroy the CCD
 */
QsiCcd::~QsiCcd() {
	// abort an exposure in progress, if any
	try {
		cancelExposure();
	} catch (...) {
	}
	// XXX turn off the cooler
}

static void	start_main(QsiCcd *qsiccd) {
	qsiccd->run();
}

/**
 * \brief start an exposure
 *
 * \param exposure	exposure parameters
 */
void	QsiCcd::startExposure(const Exposure& exposure) {
	std::unique_lock<std::recursive_mutex>	lock(_camera.mutex);

	// set the state to exposure, this also ensures we actually
	// are in a state where we could start a new exposure. The
	// base class method also sets the state to exposing
	Ccd::startExposure(exposure);

	// before starting a new exposure, we should clean up the thread
	// if it is still around. The thread will set the state to exposed.
	if (_thread) {
		if (_thread->joinable()) {
			lock.unlock();
			_thread->join();
			lock.lock();
		}
		delete _thread;
		_thread = NULL;
	}

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
		debug(LOG_ERR, DEBUG_LOG, 0, "bad exposure parameters: %s",
			x.what());
		cancelExposure();
		throw BadParameter(x.what());
	}

	// launch a thread waiting for the camera
	_exposure_done = false;
	_thread = new std::thread(start_main, this);
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
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "get Ccd state");
	bool	imageReady = false;
	while (!imageReady) {
		Timer::sleep(0.1);
		
		std::unique_lock<std::recursive_mutex>	lock(
			_camera.mutex);

		// get the camera state
		START_STOPWATCH;
		_camera.camera().get_ImageReady(&imageReady);
		END_STOPWATCH("get_ImageReady()");
	}
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "qsistate=%d", qsistate);

	// now the image is ready and we should indiate that with
	// the state going to exposed
	state(CcdState::exposed);
		
	_exposure_done = true;
}

/**
 * \brief get the current camera state
 *
 * \return the current QSI state
 */
CcdState::State	QsiCcd::exposureStatus() {
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "checking exopsure status");
	std::unique_lock<std::recursive_mutex>	lock(_camera.mutex,
		std::try_to_lock);
	if (!lock) {
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "return last state %d",
		//	(int)_last_state);
		return _last_state;
	}
	// while the exposure thread is running, we don't need to check
	// the camera status, the update thread will do that for us.
	if (_thread) {
		if (_exposure_done) {
			_thread->join();
			delete _thread;
			_thread = NULL;
		}
		return _last_state = state();
	}

#if 0
	// in all other cases, we have to query the camera again
	try {
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "checking camera state");
		// reading the camera state
		QSICamera::CameraState	qsistate;
		START_STOPWATCH;
		_camera.camera().get_CameraState(&qsistate);
		END_STOPWATCH("get_CameraState()");
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "qsistate = %s",
		//	state2string(qsistate).c_str());

		// compute the new state depending on the QSI state
		switch (state()) {
		case CcdState::idle:
			switch (qsistate) {
			case QSICamera::CameraIdle:
				break;
			case QSICamera::CameraWaiting:
			case QSICamera::CameraExposing:
				state(CcdState::exposing);
				break;
			case QSICamera::CameraReading:
			case QSICamera::CameraDownload:
				debug(LOG_DEBUG, DEBUG_LOG, 0, "turn LED on");
				_camera.camera().put_LEDEnabled(true);
				state(CcdState::exposed);
				break;
			case QSICamera::CameraError:
				break;
			}
			break;
		case CcdState::exposing:
			switch (qsistate) {
			case QSICamera::CameraIdle:
			case QSICamera::CameraWaiting:
				state(CcdState::exposed);
				break;
			case QSICamera::CameraExposing:
				state(CcdState::exposing);
				break;
			case QSICamera::CameraReading:
			case QSICamera::CameraDownload:
				debug(LOG_DEBUG, DEBUG_LOG, 0, "turn LED on");
				_camera.camera().put_LEDEnabled(true);
				state(CcdState::exposed);
				break;
			case QSICamera::CameraError:
				break;
			}
			break;
		case CcdState::exposed:
			switch (qsistate) {
			case QSICamera::CameraIdle:
			case QSICamera::CameraWaiting:
			case QSICamera::CameraExposing:
			case QSICamera::CameraReading:
			case QSICamera::CameraDownload:
			case QSICamera::CameraError:
				break;
			}
			break;
		case CcdState::cancelling:
			switch (qsistate) {
			case QSICamera::CameraIdle:
				state(CcdState::idle);
				break;
			case QSICamera::CameraWaiting:
				break;
			case QSICamera::CameraExposing:
			case QSICamera::CameraReading:
			case QSICamera::CameraDownload:
				state(CcdState::exposing);
				break;
			case QSICamera::CameraError:
				break;
			}
			break;
		case CcdState::streaming:
			// just ignore this state
			break;
		}
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "new state %s",
		//	CcdState::state2string(state()).c_str());
		_last_state = state();
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "could not get the state: %s",
			x.what());
	}
#endif
	return _last_state = state();
}

/**
 * \brief Cancel the current exposure
 */
void	QsiCcd::cancelExposure() {
	std::lock_guard<std::recursive_mutex>	lock(_camera.mutex);
	_camera.camera().AbortExposure();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "turn LED on");
	_camera.camera().put_LEDEnabled(true);
	_camera.camera().AbortExposure();
	state(CcdState::idle);
}

/**
 * \brief Get the stateu of the shutter
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
	std::lock_guard<std::recursive_mutex>	lock(_camera.mutex);
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
	return CoolerPtr(cooler);
}

} // namespace qsi
} // namespace camera
} // namespace astro

