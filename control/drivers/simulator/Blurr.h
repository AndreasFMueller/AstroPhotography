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
	double	epsilon;
	double	normalize;
	void	update();
	double	airy(double r) const;
	double	ring(double r) const;
	double	pattern(double r) const;
	double	aperture(double r) const;
public:
	Blurr(double radius = 1, double innerradius = 0)
		: _radius(radius), _innerradius(innerradius) {
		update();
	}
	void	operator()(const Image<double>& image);
	double	radius() const { return _radius; }
	void	radius(const double& radius);
	double	innerradius() const { return _innerradius; }
	void	innerradius(const double& innerradius);
};

} // namespace image
} // namespace astro

#endif /* _Blurr_h */
