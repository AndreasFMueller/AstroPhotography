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
#if 0
	// This is a temporary exposure structure. It can be changed at
	// any time, but 
#endif
	ImageStream&	_stream;
	Ccd	*_ccd;
	volatile std::atomic_bool	_running;
public:
	bool	running() const { return _running; }
private:
	std::thread	_thread;
#if 0
	std::mutex	_mutex; // mediate access to _exposure
#endif
public:
	ImageStreamThread(ImageStream& stream, Ccd *ccd);
	~ImageStreamThread();
	void	run();
	void	stop();
	void	wait();
};

} // namespace camera
} // namespace astro
