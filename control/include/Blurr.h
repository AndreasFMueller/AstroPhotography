/*
 * Blurr.h -- compute blurr caused by the telescope
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
public:
	double	radius() const { return _radius; }
	void	radius(const double& radius);
private:
	double	_innerradius;
public:
	double	innerradius() const { return _innerradius; }
	void	innerradius(const double& innerradius);
private:
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
	Image<double>	operator()(const Image<double>& image);
};

} // namespace image
} // namespace astro

#endif /* _Blurr_h */
