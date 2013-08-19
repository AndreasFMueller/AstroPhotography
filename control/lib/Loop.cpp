/**
 * Loop.cpp -- Implementation of the loop task
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroLoop.h>
#include <AstroDebug.h>
#include <includes.h>
#include <AstroFilterfunc.h>

using namespace astro::camera;
using namespace astro::io;
using namespace astro::image::filter;
using namespace astro::callback;

namespace astro {
namespace task {

/**
 * \brief Initialize the Loop task
 */
Loop::Loop(CcdPtr ccd, const Exposure& exposure, FITSdirectory& directory)
	: _ccd(ccd), _exposure(exposure), _directory(directory) {
	_nImages = 0;
	_period = 1;
	_align = false;
}

/**
 * \brief Execute the loop task
 */
void	Loop::execute() {
	// find the first image time
	time_t	start = time(NULL);
	time_t	next = start;
	if (_align) {
		next = next - (next % _period);
		next += _period;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "time for next image: %d (now %d)",
		next, start);
	if (next > start) {
		int	waittime = next - start;
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"waiting %d seconds for start time", waittime);
		sleep(waittime);
	}

	// make sure that the timer does not increase the exposure time beyond
	// the period
	_timer.limit(_period);

	// now initialize exposure computation loop
	double	exposuretime = _exposure.exposuretime;
	_counter = 0;
	while ((_counter++ < _nImages) || (_nImages == 0)) {
		// make sure the exposure time is not too long
		time_t	now = time(NULL);
		while (next <= now) {
			next += _period;
		}
		if (exposuretime > (next - now)) {
			exposuretime = next - now;
		}

		// get an exposure
		debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure %s, time %fs",
			_exposure.frame.toString().c_str(), exposuretime);

		// get an image with the current parameters
		_exposure.exposuretime = exposuretime;
		_ccd->startExposure(_exposure);
		if (!_ccd->wait()) {
			debug(LOG_ERR, DEBUG_LOG, 0,
				"failed to wait for exposure");
			// XXX bad things should happen
		}
		ImagePtr	image = _ccd->getImage();
		std::string	imagefilename = _directory.add(image);

		// compute the next exposure time, for this we need the
		// mean of the pixel values
		_timer.update(image);

		// if the call back is set, then we should also run the
		// callback on the image
		if (_newImageCallback) {
			CallbackDataPtr	cbd = CallbackDataPtr(
				new ImageCallbackData(imagefilename, image));
			(*_newImageCallback)(cbd);
		}

		// ensure that we don't exceed the period
		exposuretime = _timer;

		// now wait to the next time we start an image, if there is
		// any time to sleep at all
		if ((_counter < _nImages) || (_nImages == 0)) {
			now = time(NULL);
			time_t	delta = next - now;
			if (delta > 0) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"sleep for %d seconds", delta);
				sleep(delta);
			}
		}
	}
}

} // namespace task
} // namespace astro
