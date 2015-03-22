/*
 * Radon.h -- classes and structures for the radon related transforms
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroImage.h>

namespace astro {
namespace image {
namespace radon {

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
	virtual double	pixel(unsigned int x, unsigned int y) const;
};

} // namespace radon
} // namespace image
} // namespace astro
