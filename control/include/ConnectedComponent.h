/*
 * ConnectedComponent.h -- find the connected component of a point in an image
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ConnectedComponent_h
#define _ConnectedComponent_h

#include <AstroImage.h>

namespace astro {
namespace image {

class ConnectedComponent {
	ImagePoint	_point;
	unsigned char	growpixel(Image<unsigned char>& image,
				unsigned int x, unsigned int y) const;
	int	grow(Image<unsigned char>& image) const;
public:
	ConnectedComponent(const ImagePoint& point) : _point(point) { }
	ImagePtr	operator()(const ImagePtr image) const;
};

} // namespace image
} // namespace astro

#endif /* _ConnectedComponent_h */
