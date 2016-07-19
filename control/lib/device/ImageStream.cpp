/*
 * ImageStream.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>
#include <AstroDebug.h>
#include <mutex>
#include <thread>

namespace astro {
namespace camera {

//////////////////////////////////////////////////////////////////////
// ImageStreamThread is private data for the Image Stream base class
//////////////////////////////////////////////////////////////////////
class ImageStreamThread {
	// This is a temporary exposure structure. It can be changed at
	// any time, but 
	Exposure	_exposure;
	ImageStream&	_stream;
	Ccd	*_ccd;
	bool	_running;
	std::thread	_thread;
	std::mutex	_mutex; // mediate access to _exposure
public:
	const Exposure&	exposure();
	void	exposure(const Exposure& e);
	ImageStreamThread(const Exposure& exposure, ImageStream& stream,
		Ccd *ccd);
	~ImageStreamThread();
	void	run();
	void	stop();
};

/**
 * \brief Auxiliary function to lauch the thread
 */
static void	imagestreammain(ImageStreamThread *ist) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "imagestreammain starting");
	ist->run();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "imagestreammain terminates");
}

/**
 * \brief Construct a new thread
 */
ImageStreamThread::ImageStreamThread(const Exposure& exposure,
	ImageStream& stream, Ccd *ccd)
	: _exposure(exposure), _stream(stream), _ccd(ccd), _running(true),
	  _thread(imagestreammain, this) {
}

/**
 * \brief Kill the thread
 */
ImageStreamThread::~ImageStreamThread() {
	_running = false;
	_thread.join();
}

/**
 * \brief main function of the thread
 */
void	ImageStreamThread::run() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start the image stream thread");
	while (_running) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "start new exposure");
		_stream.streamExposure(exposure());
		_ccd->startExposure(_stream.streamExposure());
		_ccd->wait();
		ImagePtr	image = _ccd->getImage();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "image retrieved");
		_stream.add(_ccd->getExposure(), image);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "terminating the image stream thread");
}

/**
 * \brief controlled read access to the exposure structure
 */
const Exposure&	ImageStreamThread::exposure() {
	std::unique_lock<std::mutex>    lock(_mutex);
	return _exposure;
}

/**
 * \brief controlled write access to the exposure structure
 */
void	ImageStreamThread::exposure(const Exposure& e) {
	std::unique_lock<std::mutex>    lock(_mutex);
	_exposure = e;
}

/**
 * \brief Stop the thread
 */
void	ImageStreamThread::stop() {
	_running = false;
}

//////////////////////////////////////////////////////////////////////
// Implementation of the ImageStream class
//////////////////////////////////////////////////////////////////////

/**
 * \brief Construct a stream
 */
ImageStream::ImageStream(unsigned long _maxqueuelength)
	: ImageQueue(_maxqueuelength) {
	private_data = NULL;
}

/**
 * \brief Destroy the stream
 *
 * If it has a running thread, we have to kill it
 */
ImageStream::~ImageStream() {
	if (private_data) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cancelling thread");
		ImageStreamThread	*t = (ImageStreamThread *)private_data;
		delete t;
		private_data = NULL;
	}
}

/**
 * \brief start a stream with a given exposure structure
 */
void	ImageStream::startStream(const Exposure& exposure) {
	// make sure the stream is not running yet
	if (private_data) {
		throw std::logic_error("stream already running");
	}
	// find out whether we also are a CCD, in which case we can really
	// start the thread
	Ccd	*ccd = dynamic_cast<Ccd *>(this);
	if (NULL == ccd) {
		debug(LOG_ERR, DEBUG_LOG, 0, "not a CCD, cannot stream");
		throw CannotStream();
	}

	// start the thread with the information we have gathered
	ImageStreamThread	*t = new ImageStreamThread(exposure, *this, ccd);
	private_data = t;
}

/**
 * \brief Stop the stream
 */
void	ImageStream::stopStream() {
	if (private_data) {
		ImageStreamThread	*t = (ImageStreamThread *)private_data;
		t->stop();
	}
	throw CannotStream();
}

/**
 * \brief change the stream exposure
 */
void	ImageStream::streamExposure(const Exposure& exposure) {
	_streamexposure = exposure;
	if (private_data) {
		ImageStreamThread	*t = (ImageStreamThread *)private_data;
		t->exposure(exposure);
	}
}

/**
 * \brief get the current exposure settings
 */
const Exposure&	ImageStream::streamExposure() const {
	return _streamexposure;
}

/**
 * \brief Process an image entry
 *
 * This method sends the entry to the queue if no sink is defined, but
 * if there is a sink, the image is sent there.
 */
void	ImageStream::operator()(const ImageQueueEntry& entry) {
	if (_imagesink) {
		(*_imagesink)(entry);
	} else {
		ImageQueueEntry	newentry(entry);
		try {
			ImageQueue::add(newentry);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "new queue entry %ld",
				newentry.sequence);
		} catch (ImageDropped& x) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "entry dropped");
		}
	}
}

} // namespace camera
} // namespace astro
