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

BacklashWork::BacklashWork(camera::ImagerPtr imager, TrackerPtr tracker,
		camera::GuidePortPtr guideport)
	: _imager(imager), _tracker(tracker), _guideport(guideport) {
	_interval = 5;
};

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

void	BacklashWork::main(astro::thread::Thread<BacklashWork>& thread) {
	// setup
	std::vector<BacklashPoint>	data;
	double	starttime = Timer::gettime();
	int	counter = 0;
	move(-_interval);

	// repeat up/down movement
	do {
		// get an image (need a imager for this)
		_imager->startExposure(_exposure);
		_imager->wait();
		ImagePtr	image = _imager->getImage();
		
		// find the offset
		Point	imagepoint = (*_tracker)(image);

		// convert to a BacklashPoint
		BacklashPoint	p;
		p.id = counter++;
		p.time = Timer::gettime() - starttime;
		p.xoffset = imagepoint.x();
		p.yoffset = imagepoint.y();
		data.push_back(p);

		// send it through the callback
		point(p);

		// if we have enough data, create a new analysis
		if (data.size() >= 5) {
			BacklashAnalysis	analysis(_direction);
			result(analysis(data));
		}

		// move the guideport
		if ((counter >> 1) % 2) {
			move(-_interval);
		} else {
			move(+_interval);
		}
	}

	// we should tell via a callback, that the sequence has ended,
	// maybe with a point with negative id?
}

void	BacklashWork::stop() {
	// XXX missing
}

void	BacklashWork::point(const BacklashPoint& bp) {
	if (_pointcallback) {
		CallbackBacklashPointPtr	p(new CallbackBacklashPoint(bp));
		(*_pointcallback)(p);
	}
}

void	BacklashWork::result(const BacklashResult& br) {
	if (_resultcallback) {
		CallbackBacklashResultPtr	p(new CallbackBacklashResult(br));
		(*_resultcallback)(p);
	}
}

} // namespace guiding
} // namespace astro
