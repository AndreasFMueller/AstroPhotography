/*
 * BacklashWork.cpp -- work to be done to characterize backlash
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroImager.h>
#include <AstroDebug.h>
#include <AstroGuiding.h>
#include <Backlash.h>

namespace astro {
namespace guiding {

/**
 * \brief Construct a new BAcklashWork object
 *
 * This constructor also installs the imager, tracker and the guideport
 * that is needed.
 */
BacklashWork::BacklashWork(camera::ImagerPtr imager, TrackerPtr tracker,
		camera::GuidePortPtr guideport)
	: _imager(imager), _tracker(tracker), _guideport(guideport) {
	_interval = 5;
};

/**
 * \brief Move and wait until move is complete
 *
 * \param interval	duration of the diferent moves
 */
void	BacklashWork::move(double interval) {
	double	i = fabs(interval);
	switch (_direction) {
	case backlash_dec:
		if (interval > 0) {
			_guideport->activate(0, 0, i, 0);
		} else {
			_guideport->activate(0, 0, 0, i);
		}
		break;
	case backlash_ra:
		if (interval > 0) {
			_guideport->activate(i, 0, 0, 0);
		} else {
			_guideport->activate(0, i, 0, 0);
		}
		break;
	}
	Timer::sleep(interval);
}

/**
 * \brief Main method doing the work
 */
void	BacklashWork::main(astro::thread::Thread<BacklashWork>& thread) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start backlash main method");
	try {
		// setup of the common variables
		std::vector<BacklashPoint>	data;
		double	starttime = Timer::gettime();
		int	counter = 0;
		move(-_interval);

		// repeat up/down movement
		debug(LOG_DEBUG, DEBUG_LOG, 0, "start backlash measuring cycle");
		do {
			// get an image (need a imager for this)
			_imager->startExposure(_exposure);
			_imager->wait();
			ImagePtr	image = _imager->getImage();
			
			// find the offset
			Point	imagepoint = (*_tracker)(image);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "point = %s",
				imagepoint.toString().c_str());

			// convert to a BacklashPoint
			BacklashPoint	backlashpoint;
			backlashpoint.id = counter++;
			backlashpoint.time = Timer::gettime() - starttime;
			backlashpoint.xoffset = imagepoint.x();
			backlashpoint.yoffset = imagepoint.y();
			data.push_back(backlashpoint);
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"added a new BacklashPoint %s",
				backlashpoint.toString().c_str());

			// send it through the callback
			point(backlashpoint);

			// if we have enough data, create a new analysis
			if (data.size() >= 5) {
				BacklashAnalysis	analysis(_direction);
				BacklashResult	r = analysis(data);
				result(r);
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"new analysis: %s",
					r.toString().c_str());
					
			}

			// move the guideport
			if ((counter >> 1) % 2) {
				move(-_interval);
			} else {
				move(+_interval);
			}
		} while (!thread.terminate());

		// we should tell via a callback, that the sequence has ended,
		// maybe with a point with negative id?
		BacklashPoint	backlashpoint;
		backlashpoint.id = -1;
		backlashpoint.time = starttime;
		point(backlashpoint);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"BacklashWork::main terminated by exception: %s",
			x.what());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "BacklashWork::main terminates");
}

/**
 * \brief Method to send a single point information to the callback
 */
void	BacklashWork::point(const BacklashPoint& bp) {
	if (_pointcallback) {
		CallbackBacklashPointPtr p(new CallbackBacklashPoint(bp));
		(*_pointcallback)(p);
	}
}

/**
 * \brief Method to send the analysis results to the callback
 */
void	BacklashWork::result(const BacklashResult& br) {
	if (_resultcallback) {
		CallbackBacklashResultPtr p(new CallbackBacklashResult(br));
		(*_resultcallback)(p);
	}
}

} // namespace guiding
} // namespace astro
