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

namespace astro {
namespace task {

Loop::Loop(CcdPtr ccd, const Exposure& exposure, FITSdirectory& directory)
	: _ccd(ccd), _exposure(exposure), _directory(directory) {
	_targetmean = -1;
	_targetmedian = -1;
	_nImages = 0;
	_period = 1;
	_align = false;
}

static const double	a = 0.5;

static double	scalefactor(double x) {
	double	y = x - 1;
	double	result =  y * (1 - a * exp(-y * y)) + 1;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "x = %f, scalefactor = %f", x, result);
	return result;
}

void	Loop::execute() {
	// find the first image time
	time_t	start = time(NULL);
	if (_align) {
		start %= _period;
		start += _period;
	}
	time_t	next = start;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "time for next image: %d", start);

	// now initialize exposure computation loop
	double	exposuretime = _exposure.exposuretime;
	unsigned int	counter = 0;
	while ((counter++ < _nImages) || (_nImages == 0)) {
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
		_directory.add(image);

		// compute the next exposure time, for this we need the
		// mean of the pixel values
		if (_targetmean > 0) {
			double	mnew = mean(image);
			double	newexp = exposuretime * scalefactor(_targetmean / mnew);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "target mean = %f, "
				"actual mean = %f, current exposure time = %f, "
				"new = %f", _targetmean, mnew, exposuretime,
				newexp);
			exposuretime = newexp;
		}
		if ((_targetmean < 0) && (_targetmedian > 0)) {
			double	mnew = median(image);
			double	newexp = exposuretime * scalefactor(_targetmedian / mnew);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "target median = %f, "
				"actual median = %f, current exposure time = %f, "
				"new = %f", _targetmedian, mnew, exposuretime,
				newexp);
			exposuretime = newexp;
		}
		if (exposuretime > _period) {
			exposuretime = _period;
		}

		// now wait to the next time we start an image, if there is
		// any time to sleep at all
		if ((counter < _nImages) || (_nImages == 0)) {
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
