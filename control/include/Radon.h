/*
 * Radon.h -- classes and structures for the radon related transforms
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroImage.h>

namespace astro {
namespace image {
namespace radon {

/*
 * \brief Radon transform
 */
class RadonTransform : public ConstImageAdapter<double> {
	const ConstImageAdapter<double>&	_image;
	Image<double>	_radon;
public:
	RadonTransform(const ImageSize& size,
		const ConstImageAdapter<double>& image);
	virtual double	pixel(int x, int y) const {
		return _radon.pixel(x, y);
	}
};

/**
 * \brief Adapter class that allows access for arbitratry y-arguments
 */
class RadonAdapter : public ConstImageAdapter<double> {
	RadonTransform	_radon;
public:
	RadonAdapter(const ImageSize& size,
		const ConstImageAdapter<double>& image);
	virtual double	pixel(int x, int y) const;
};

/**
 * \brief BackProjection used to invert the Radon transform
 */
class BackProjection : public ConstImageAdapter<double> {
	const ConstImageAdapter<double>&	_radon;
	Image<double>	_backprojection;
	void	anglesum(int angleindex);
public:
	BackProjection(const ImageSize& size,
		const ConstImageAdapter<double>& radon);
	virtual double	pixel(int x, int y) const;
};

/**
 * \brief Class representing a segment of a curve
 */
class segment {
	int	_x, _y;
	double	_w;
public:
	segment(int x, int y, double w);
	int	x() const { return _x; }
	int	y() const { return _y; }
	double	w() const { return _w; }
};

/**
 * \brief Class representing a circle built from curve segments
 */
class circle {
	void	add_segments(const segment& s);
public:
	typedef	std::vector<segment>	segments_t;
	typedef std::shared_ptr<segments_t>	segment_ptr;
private:
	segment_ptr	_segments;
public:
	circle(double r);
	double	value(const ConstImageAdapter<double>& image, int x, int y) const;
	double	length() const;
};

/**
 * \brief An adapter to compute the circle transform for a given circle
 */
class CircleAdapter : public ConstImageAdapter<double> {
	const circle&	_circ;
	const ConstImageAdapter<double>&	_image;
public:
	CircleAdapter(const ConstImageAdapter<double>& image,
		const circle& circ);
	~CircleAdapter();
	virtual double	pixel(int x, int y) const;
};

} // namespace radon
} // namespace image
} // namespace astro
