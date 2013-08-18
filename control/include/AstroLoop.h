/*
 * AstroLoop.h -- a class to fetch images in a loop
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroLoop_h
#define _AstroLoop_h

#include <AstroCamera.h>
#include <AstroIO.h>

namespace astro {
namespace task {

/**
 * \brief Exposure filter
 */
class ExposureTimer {
public:
	typedef enum { NONE, MEAN, MEDIAN } timer_method;
private:
	double	_exposuretime;
	double	_targetvalue;
	timer_method	_method;
	double	_relaxation;
	double	_limit;
public:
	ExposureTimer(double exposuretime = 0, double targetvalue = 1,
		timer_method method = NONE)
		: _exposuretime(exposuretime), _targetvalue(targetvalue),
		  _method(method), _relaxation(0.5) { }
	double	exposuretime() const { return _exposuretime; }
	void	exposuretime(double exposuretime) { _exposuretime = exposuretime; }
	double	relaxation() const { return _relaxation; }
	void	relaxation(double relaxation) { _relaxation = relaxation; }
	double	limit() const { return _limit; }
	void	limit(double limit) { _limit = limit; }
	void	update(ImagePtr image);
	operator double () { return _exposuretime; }
};

/**
 * \brief Loop task
 *
 * This task takes a number of images in a loop
 */
class Loop {
	astro::camera::CcdPtr	_ccd;
	astro::camera::Exposure	_exposure;
	astro::io::FITSdirectory&	_directory;
	ExposureTimer	_timer;
	unsigned int	_nImages;
	unsigned int	_counter;
	unsigned int	_period;
	bool	_align;
public:
	Loop(astro::camera::CcdPtr ccd, const astro::camera::Exposure& exposure, astro::io::FITSdirectory& directory);
	// accessors
	const astro::camera::Exposure&	exposure() const { return _exposure; }
	const ExposureTimer&	timer() const { return _timer; }
	void	timer(const ExposureTimer& timer) { _timer = timer; }
	unsigned int	nImages() const { return _nImages; }
	void	nImages(unsigned int nImages) { _nImages = nImages; }
	unsigned int	period() const { return _period; }
	void	period(unsigned int period) { _period = period; }
	bool	align() const { return _align; }
	void	align(bool align) { _align = align; }
	// do the work
	void	execute();
	unsigned int	counter() const { return _counter; }
};

} // namespace task
} // namespace astro

#endif /* _AstroLoop_h */
