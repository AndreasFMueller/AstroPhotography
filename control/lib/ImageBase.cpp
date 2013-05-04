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
 * \brief Construct an image base from size parameters
 */
ImageBase::ImageBase(int w, int h) : size(w, h) {
	mosaic = NONE;
}

ImageBase::ImageBase(const ImageSize& _size) : size(_size) {
	mosaic = NONE;
}

ImageBase::ImageBase(const ImageRectangle& frame) : size(frame.size) {
	mosaic = NONE;
}

ImageBase::ImageBase(const ImageBase& other) : size(other.size) {
	mosaic = other.mosaic;
}

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

bool	ImageBase::isR(int x, int y) const {
	if (mosaic == NONE) {
		return false;
	}
	return (((y & 0x1) << 1) | (x & 0x1)) == (mosaic & 0x3);
}

bool	ImageBase::isB(int x, int y) const {
	if (mosaic == NONE) {
		return false;
	}
	// this means that the mod 2 remainder of both x and y have to
	// be different from the ones in the mosaic constant. The XOR
	// with 0x3 inverts the coordinates so that we can nevertheless
	// do an equality comparison
	return (0x3 ^ (((y & 0x1) << 1) | (x & 0x1))) == (mosaic & 0x3);
}

bool	ImageBase::isG(int x, int y) const {
	return (isGr(x, y) | isGb(x, y));
	
}

bool	ImageBase::isGr(int x, int y) const {
	if (mosaic == NONE) {
		return false;
	}
	return (0x1 ^ (((y & 0x1) << 1) | (x & 0x1))) == (mosaic & 0x3);
}

bool	ImageBase::isGb(int x, int y) const {
	if (mosaic == NONE) {
		return false;
	}
	return (0x2 ^ (((y & 0x1) << 1) | (x & 0x1))) == (mosaic & 0x3);
}

} // namespace image
} // namespace astro
