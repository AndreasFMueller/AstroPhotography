/*
 * AstroProjection.h -- Projection of images
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroProjection_h
#define _AstroProjection_h

#include <AstroTypes.h>
#include <AstroImage.h>
#include <AstroTransform.h>

namespace astro {
namespace image {
namespace project {

/**
 * \brief A projection
 *
 * Projections are affine transformations composed with a radius dependent
 * homothety.
 */
class Projection : public astro::image::transform::Transform {
	float	b[2];
	float	w(float r) const;
public:
	Projection();
	Projection(double angle, const Point& translation,
                double scalefactor = 1);
	Point	operator()(const Point& p) const;
};

/**
 * \brief Apply a projection to an image
 */
template<typename Pixel>
class ProjectionAdapter : public ConstImageAdapter<Pixel> {
	const astro::image::transform::PixelInterpolationAdapter<Pixel> image;
	Projection	projection;
	Point	center;
	Point	targetcenter;
public:
	ProjectionAdapter(const ImageSize targetsize,
		const ConstImageAdapter<Pixel>& _image,
		const Projection& _projection)
		: ConstImageAdapter<Pixel>(targetsize), image(_image),
		  projection(_projection) {
		center = ConstImageAdapter<Pixel>::getSize().center();
		targetcenter = targetsize.center();
	}
	virtual const Pixel	pixel(unsigned int x, unsigned int y) const;
};

template<typename Pixel>
const Pixel	ProjectionAdapter<Pixel>::pixel(unsigned int x, unsigned int y) const {
	Point	p(x - center.x(), y - center.y());
	return image.pixel(projection(p) + targetcenter);
}

/**
 * \brief Residuals needed to analyze transforms
 */
class Residual : public std::pair<ImagePoint, Point> {
public:
	Residual(const ImagePoint& _from, const Point& _offset)
		: std::pair<ImagePoint, Point>(_from, _offset) {
	}
	const ImagePoint&	from() const { return first; }
	ImagePoint&	from() { return first; }
	const Point&	offset() const { return second; }
	Point&	offset() { return second; }
};

/**
 * \brief Analysis of a transformation
 */
class ProjectionAnalyzer {
	const ConstImageAdapter<double>& baseimage;
	unsigned int	spacing;
	unsigned int	patchsize;
public:
	ProjectionAnalyzer(const ConstImageAdapter<double>& _baseimage,
		unsigned int _spacing = 128, unsigned int _patchsize = 128)
                : baseimage(_baseimage),
                  spacing(_spacing), patchsize(_patchsize)  {
	}
        std::vector<Residual>	operator()(const ConstImageAdapter<double>& image) const;
};


} // namespace project
} // namespace image
} // namespace astro

#endif /* _AstroProjection_h */
