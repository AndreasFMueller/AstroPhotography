/*
 * Image.cpp -- image implementation
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroImage.h>

namespace astro {
namespace image {

/**
 * \brief Comparison of YUYV pixel values
 */ 
bool	YUYVPixel::operator==(const YUYVPixel& other) const {
	return (y == other.y) && (uv == other.uv);
}

/**
 * \brief Construction of YUYV from 8bit luminance
 */
YUYVPixel::YUYVPixel(unsigned char _y, unsigned char _uv)
	: y(_y), uv(_uv) { }
/**
 * \brief Construction of YUYV from 16bit luminance
 *
 * This constructor is used for the conversion of images with 16bit
 * pixel values to YUYV images.
 */
YUYVPixel::YUYVPixel(unsigned short _y, unsigned char _uv)
	: y(_y >> 8), uv(_uv) { }
/**
 * \brief Construction of YUYV from 32bit luminance
 *
 * This constructor is used for the conversion of images with 32bit
 * pixel values to YUYV images.
 */
YUYVPixel::YUYVPixel(unsigned long _y, unsigned char _uv)
	: y(_y >> 24), uv(_uv) { }


// specializations for the integer types: these types are used to
// compensate dynamic range differences
template<>
unsigned char	convert(const unsigned short& p) {
	return p >> 8;
}

template<>
unsigned char	convert(const unsigned long& p) {
	return p >> 24;
}
template<>
unsigned short	convert(const unsigned char& p) {
	return p << 8;
}
template<>
unsigned short	convert(const unsigned long& p) {
	return p >> 16;
}
template<>
unsigned long	convert(const unsigned char& p) {
	return p << 24;
}
template<>
unsigned long	convert(const unsigned short& p) {
	return p << 16;
}

template<>
unsigned char	convert(const YUYVPixel& p) {
	return p.y;
}

} // namespace image
} // namespace astro
