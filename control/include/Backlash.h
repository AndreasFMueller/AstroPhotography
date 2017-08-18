/*
 * Backlash.h -- data structures for backlash analysis
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _Backlash_h
#define _Backlash_h

#include <AstroGuiding.h>
#include <AstroCallback.h>
#include <ostream>

namespace astro {
namespace guiding {

std::ostream&	operator<<(std::ostream& out, const BacklashPoint& point);
std::ostream&	operator<<(std::ostream& out, const BacklashPoints& points);

std::ostream&	operator<<(std::ostream& out, const BacklashResult&);

std::ostream&	operator<<(std::ostream& out, const BacklashData&);

/**
 * \brief The class that does the actual computation of the backlash analysis
 */
class BacklashAnalysis {
	backlash_t	_direction;
	double	_interval;
	int	_lastpoints;
	double	drift(const std::vector<BacklashPoint>& points,
			const BacklashResult& r) const;
	std::vector<BacklashPoint>::const_iterator	begin(const std::vector<BacklashPoint>& points) const;
public:
	BacklashAnalysis(backlash_t direction = backlash_dec,
		double interval = 5, int lastpoints = 0)
		: _direction(direction), _interval(interval),
		  _lastpoints(lastpoints) { }
	BacklashResult	operator()(const std::vector<BacklashPoint>& points);
};

/**
 * \brief The work class for backlash characterization
 */
class BacklashWork {
	backlash_t	_direction;
public:
	void	direction(backlash_t direction) { _direction = direction; }
	backlash_t	direction() const { return _direction; }
private:
	double	_interval;
public:
	double	interval() const { return _interval; }
	void	interval(double i) { _interval = i; }
private:
	int	_lastpoints;
public:
	int	lastPoints() const { return _lastpoints; }
	void	lastPoints(int n);
private:
	guiding::Guider&	_guider;
	camera::Exposure	_exposure;
public:
	camera::Exposure	exposure() const { return _exposure; }
	void	exposure(const camera::Exposure& e) { _exposure = e; }
private:
	TrackerPtr	_tracker;
	camera::GuidePortPtr	_guideport;

	// callbacks
	callback::CallbackPtr	_callback;
public:
	void	callback(callback::CallbackPtr pcb) {
			_callback = pcb;
	}
private:
	void	point(const BacklashPoint&);
	void	result(const BacklashResult&);
	void	move(double i);
public:
	BacklashWork(guiding::Guider& imager, TrackerPtr tracker);
	void	main(astro::thread::Thread<BacklashWork>& thread);
	void	stop();
};

} // namespace guiding
} // namespace astro

#endif /* _Backlash_h */
