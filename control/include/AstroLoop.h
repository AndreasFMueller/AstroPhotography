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

class Loop {
	astro::camera::CcdPtr	_ccd;
	astro::camera::Exposure	_exposure;
	astro::io::FITSdirectory&	_directory;
	double	_targetmean;
	double	_targetmedian;
	unsigned int	_nImages;
	unsigned int	_period;
	bool	_align;
public:
	Loop(astro::camera::CcdPtr ccd, const astro::camera::Exposure& exposure, astro::io::FITSdirectory& directory);
	// accessors
	const astro::camera::Exposure&	exposure() const { return _exposure; }
	const double&	targetmean() const { return _targetmean; }
	void	targetmean(double targetmean) { _targetmean = targetmean; }
	const double&	targetmedian() const { return _targetmedian; }
	void	targetmedian(double targetmedian) { _targetmedian = targetmedian; }
	unsigned int	nImages() const { return _nImages; }
	void	nImages(unsigned int nImages) { _nImages = nImages; }
	unsigned int	period() const { return _period; }
	void	period(unsigned int period) { _period = period; }
	bool	align() const { return _align; }
	void	align(bool align) { _align = align; }
	// do the work
	void	execute();
};

} // namespace task
} // namespace astro

#endif /* _AstroLoop_h */
