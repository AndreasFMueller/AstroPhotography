/*
 * AsiStream.h -- stream thread for the ASI cameras
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _AsiStream_h
#define _AsiStream_h

#include <AsiCcd.h>
#include <thread>
#include <mutex>

namespace astro {
namespace camera {
namespace asi {

class AsiStream {
	AsiCcd	*_ccd;
	bool	_running;
	std::thread	_thread;	
public:
	AsiStream(AsiCcd *ccd);
	~AsiStream();
	void	stop();
	void	run();
};

} // namespace asi
} // namespace camera
} // namespace astro


#endif /* _AsiStream_h */
