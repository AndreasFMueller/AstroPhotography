/*
 * AstroPostprocessing.h
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroImage.h>

namespace astro {
namespace image {
namespace post {

/**
 * \brief Rescale an image so that pixel luminance value are in a range
 *
 * The rescaled image has pixel luminance values between _minimum and
 * _maximum
 */
class Rescale {
	double	_minimum;
public:
	double	minimum() const { return _minimum; }
	void	minimum(double m) { _minimum = m; }
private:
	double	_maximum;
public:
	double	maximum() const { return _maximum; }
	void	maximum(double m) { _maximum = m; }
private:
	double	_scale;
public:
	double	scale() const { return _scale; }
	void	scale(double s) { _scale = s; }
public:
	Rescale();
	ImagePtr	operator()(ImagePtr image) const;
};

/**
 * \brief Produce HDR image from an image based on a star mask
 */
class HDR {
	double	_radius;
public:
	double	radius() const { return _radius; }
	void	radius(double r) { _radius = r; }
private:
	double	_degree;
public:
	double	degree() const { return _degree; }
	void	degree(double d) { _degree = d; }
private:
	ImagePtr	_mask;
public:
	ImagePtr	mask() const { return _mask; }
	void	mask(ImagePtr m) { _mask = m; }
public:
	HDR();
	ImagePtr	operator()(ImagePtr image) const;
};

} // namespace post
} // namespace image
} // namespace astro
