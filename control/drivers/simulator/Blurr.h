/*
 * Blurr.h -- compute blurr
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Blurr_h
#define _Blurr_h

#include <AstroImage.h>

namespace astro {
namespace image {

class Blurr {
	double	_radius;
	double	_innerradius;
public:
	Blurr(double radius = 0, double innerradius = 0)
		: _radius(radius), _innerradius(innerradius) { }
	void	operator()(const Image<double>& image);
	double	radius() const { return _radius; }
	void	radius(const double& radius) { _radius = radius; }
	double	innerradius() const { return _innerradius; }
	void	innerradius(const double& innerradius) {
		_innerradius = innerradius;
	}
};

} // namespace image
} // namespace astro

#endif /* _Blurr_h */
