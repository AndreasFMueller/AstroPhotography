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

typedef enum backlash_e { backlash_dec = 0, backlash_ra = 1 } backlash_t;

/**
 * \brief Backlash raw data point
 */
class BacklashPoint {
public:
	int	id;
	double	time;
	double	xoffset;
	double	yoffset;
};
typedef callback::CallbackDataEnvelope<BacklashPoint>	CallbackBacklashPoint;
typedef std::shared_ptr<CallbackBacklashPoint>	CallbackBacklashPointPtr;

std::ostream&	operator<<(std::ostream& out, const BacklashPoint& point);
std::ostream&	operator<<(std::ostream& out,
			const std::vector<BacklashPoint>& points);

/**
 * \brief A holder class for the Backlash analysis results
 */
class BacklashResult {
public:
	backlash_t	direction;	// direction
	double	x, y;			// primary direction
	double	longitudinal, lateral;	// errors
	double	forward, backward;	// movements
	double	f, b;			// forward/backward 
	double	offset, drift;
	std::string	toString() const;
	double	operator()(const int k[4], const BacklashPoint& p);
};
typedef callback::CallbackDataEnvelope<BacklashResult>	CallbackBacklashResult;
typedef std::shared_ptr<CallbackBacklashResult>	CallbackBacklashResultPtr;

std::ostream&	operator<<(std::ostream& out, const BacklashResult&);

/**
 * \brief a holder class for backlash data and analysis results
 */
class BacklashData {
public:
	BacklashResult	results;
	std::vector<BacklashPoint>	points;
};
std::ostream&	operator<<(std::ostream& out, const BacklashData&);

/**
 * \brief The class that does the actual computation of the backlash analysis
 */
class BacklashAnalysis {
	backlash_t	_direction;
	double	drift(const std::vector<BacklashPoint>& points,
			const BacklashResult& r) const;
public:
	BacklashAnalysis(backlash_t direction = backlash_dec)
		: _direction(direction) { }
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
	camera::ImagerPtr	_imager;
	camera::Exposure	_exposure;
public:
	camera::Exposure	exposure() const { return _exposure; }
	void	exposure(const camera::Exposure& e) { _exposure = e; }
private:
	TrackerPtr	_tracker;
	camera::GuidePortPtr	_guideport;
	callback::CallbackPtr	_pointcallback;
	callback::CallbackPtr	_resultcallback;
public:
	void	pointcallback(callback::CallbackPtr pcb) {
			_pointcallback = pcb;
	}
	void	resultcallback(callback::CallbackPtr rcb) {
			_resultcallback = rcb;
	}
private:
	void	point(const BacklashPoint&);
	void	result(const BacklashResult&);
	void	move(double i);
public:
	BacklashWork(camera::ImagerPtr imager, TrackerPtr tracker,
			camera::GuidePortPtr guideport);
	void	main(astro::thread::Thread<BacklashWork>& thread);
	void	stop();
};

} // namespace guiding
} // namespace astro

#endif /* _Backlash_h */
