/*
 * ConnectedComponent.h -- find the connected component of a point in an image
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ConnectedComponent_h
#define _ConnectedComponent_h

#include <AstroImage.h>
#include <AstroTypes.h>
#include <AstroDebug.h>

namespace astro {
namespace image {

class ComponentBase : public Image<unsigned char> {
	ImagePoint	_point;
	unsigned long	_npoints;
	
	int	grow();
	int	growpixel(unsigned int x, unsigned int y);
protected:
	void	process();
public:
	ImagePoint	point() const { return _point; }
	void	point(const ImagePoint& p) { _point = p; }

	unsigned long	npoints() const { return _npoints; }

	ComponentBase(const ImageSize& size, const ImagePoint& point);

	std::list<ImagePoint>	points() const;
private:
	Point	_center;
public:
	double	radius();
	const Point&	center() const { return _center; }
};

template<typename Pixel>
class Component : public ComponentBase {
	Pixel		_limit;

public:
	Pixel	limit() const { return _limit; }
	void	limit(Pixel l) { _limit = l; }

	Component(const ConstImageAdapter<Pixel>& image, Pixel limit,
		const ImagePoint &point)
		: ComponentBase(image.getSize(), point), _limit(limit) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"component of %s in %s, limit %f",
			point.toString().c_str(),
			image.getSize().toString().c_str(), _limit);

		int	w = image.getSize().width();
		int	h = image.getSize().height();
		// fill the component image with 0/1
		long	counter = 0;
		for (int x = 0; x < w; x++) {
			for (int y = 0; y < h; y++) {
				Pixel	v = image.pixel(x, y);
				if (v > _limit) {
					pixel(x, y) = 1;
					counter++;
				} else {
					pixel(x, y) = 0;
				}
			}
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%ld active points", counter);

		process();
	}
};

} // namespace image
} // namespace astro

#endif /* _ConnectedComponent_h */
