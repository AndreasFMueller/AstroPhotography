/*
 * ImageStreamThread.h -- ImageStreamThread definitions
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>
#include <AstroDebug.h>
#include <AstroUtils.h>
#include <mutex>
#include <thread>

namespace astro {
namespace camera {

/**
 * \brief ImageStreamThread is private data for the Image Stream base class
 */
class ImageStreamThread {
	// This is a temporary exposure structure. It can be changed at
	// any time, but 
	ImageStream&	_stream;
	Ccd	*_ccd;
	bool	_running;
public:
	bool	running() const { return _running; }
private:
	std::thread	_thread;
	std::mutex	_mutex; // mediate access to _exposure
public:
	ImageStreamThread(ImageStream& stream, Ccd *ccd);
	~ImageStreamThread();
	void	run();
	void	stop();
};

} // namespace camera
} // namespace astro
