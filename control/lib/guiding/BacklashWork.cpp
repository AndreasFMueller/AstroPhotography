/*
 * BacklashWork.cpp -- work to be done to characterize backlash
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroImager.h>
#include <AstroDebug.h>
#include <AstroGuiding.h>
#include <AstroEvent.h>
#include <Backlash.h>
#include <AstroCamera.h>

namespace astro {
namespace guiding {

/**
 * \brief Construct a new BAcklashWork object
 *
 * This constructor also installs the imager, tracker and the guideport
 * that is needed.
 */
BacklashWork::BacklashWork(guiding::Guider& guider, TrackerPtr tracker)
	: _guider(guider), _tracker(tracker) {
	_interval = 5;
	_lastpoints = 0;
};

/**
 * \brief set the number of points to include
 */
void	BacklashWork::lastPoints(int n) {
	if (n == 0) {
		_lastpoints = 0;
		return;
	}
	if (n < 8) {
		throw std::range_error("need at least 8 points");
	}
	_lastpoints = n;
}

/**
 * \brief Move and wait until move is complete
 *
 * \param interval	duration of the diferent moves
 */
void	BacklashWork::move(double interval) {
	float	i = fabs(interval);
	switch (_direction) {
	case backlash_dec:
		if (interval > 0) {
			_guider.guideport()->activate(
				camera::GuidePortActivation(0., 0., i, 0.));
		} else {
			_guider.guideport()->activate(
				camera::GuidePortActivation(0., 0., 0., i));
		}
		break;
	case backlash_ra:
		if (interval > 0) {
			_guider.guideport()->activate(
				camera::GuidePortActivation(i, 0., 0., 0.));
		} else {
			_guider.guideport()->activate(
				camera::GuidePortActivation(0., i, 0., 0.));
		}
		break;
	}
	Timer::sleep(i);
}

/**
 * \brief Main method doing the work
 */
void	BacklashWork::main(astro::thread::Thread<BacklashWork>& thread) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start backlash main method");

	std::string	msg = stringprintf(
		"start backlash characterization guideport %s",
		_guider.guideport()->name().toString().c_str());
	astro::event(EVENT_CLASS, astro::events::INFO,
		astro::events::Event::GUIDE, msg);
	double	starttime = Timer::gettime();
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "guider exposure: %s",
			_exposure.toString().c_str());
		_guider.exposure(_exposure);
		ImagePtr	image = _guider.GuiderBase::getImage();

		if (!image) {
			debug(LOG_ERR, DEBUG_LOG, 0, "no image");
			throw std::runtime_error("no image");
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "new image retrieved");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "image: %s",
			image->size().toString().c_str());
		
		// find the offset
		Point	originpoint = (*_tracker)(image);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "point = %s",
			originpoint.toString().c_str());

		// setup of the common variables
		std::vector<BacklashPoint>	data;
		int	counter = 0;
		move(-_interval);

		// repeat up/down movement
		debug(LOG_DEBUG, DEBUG_LOG, 0, "start backlash measuring cycle");
		do {
			// get an image (need a imager for this)
			_guider.exposure(_exposure);
			ImagePtr	image = _guider.getImage();
			
			// find the offset
			Point	imagepoint = (*_tracker)(image) - originpoint;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "point = %s",
				imagepoint.toString().c_str());

			// convert to a BacklashPoint
			BacklashPoint	backlashpoint;
			backlashpoint.id = counter;
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
			if (data.size() >= 8) {
				BacklashAnalysis	analysis(_direction,
					_interval, _lastpoints);
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
			counter++;
		} while (!thread.terminate());
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"BacklashWork::main terminated by exception: %s",
			x.what());
		msg = stringprintf("backlash characterization with guideport %s "
			"terminated by exception %s",
			_guider.guideport()->name().toString().c_str(),
			demangle_string(x).c_str());
		astro::event(EVENT_CLASS, astro::events::ERR,
			astro::events::Event::GUIDE, msg);
	}

	// we should tell via a callback, that the sequence has ended,
	// maybe with a point with negative id?
	BacklashPoint	backlashpoint;
	backlashpoint.id = -1;
	backlashpoint.time = starttime;
	point(backlashpoint);

	msg = stringprintf("end backlash characterization with guideport %s",
		_guider.guideport()->name().toString().c_str());
	astro::event(EVENT_CLASS, astro::events::INFO,
		astro::events::Event::GUIDE, msg);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "BacklashWork::main terminates");
}

/**
 * \brief Method to send a single point information to the callback
 */
void	BacklashWork::point(const BacklashPoint& bp) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add a BacklashPoint");
	if (_callback) {
		CallbackBacklashPointPtr p(new CallbackBacklashPoint(bp));
		(*_callback)(p);
	}
}

/**
 * \brief Method to send the analysis results to the callback
 */
void	BacklashWork::result(const BacklashResult& br) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add a BacklashResult");
	if (_callback) {
		CallbackBacklashResultPtr p(new CallbackBacklashResult(br));
		(*_callback)(p);
	}
}

} // namespace guiding
} // namespace astro
