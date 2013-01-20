/*
 * ImageBase.cpp
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroImage.h>

namespace astro {
namespace image {

/**
 * \brief Compare two images
 *
 * Two images are considered equal if the have identical size.
 */
bool	ImageBase::operator==(const ImageBase& other) const {
	return (size == other.size);
}

/**
 * \brief Compute the pixel offset into an Image based on coordinates
 */
int     ImageBase::pixeloffset(int x, int y) const {
	return x + size.width * y;
}

/**
 * \brief Compute the pixel offset into an Image based on an ImagePoint
 */
int     ImageBase::pixeloffset(const ImagePoint& p) const {
	return this->pixeloffset(p.x, p.y);
}

} // namespace image
} // namespace astro
