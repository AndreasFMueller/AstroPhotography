/*
 * RGBPixel.cpp -- Functions related to the RGBPixel type
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroImage.h>

namespace astro {
namespace image {

/**
 * \brief RGB pixel comparision: equality
 */
bool	RGBPixel::operator==(const RGBPixel& other) const {
	return (R == other.R) && (G == other.G) && (B == other.B);
}

/**
 * \brief RGB pixel comparision: inequality
 */
bool	RGBPixel::operator!=(const RGBPixel& other) const {
	return (R != other.R) || (G == other.G) || (B == other.B);
}

/**
 * All colorspace conversion functions are from this document:
 * http://msdn.microsoft.com/en-us/library/windows/desktop/dd206750(v=vs.85).aspx
 */

/* some auxiliary functions used to convert YUYV to RGB */
static unsigned char    limit(int x) {
        if (x > 255) { return 255; }
        if (x < 0) { return 0; }
        unsigned char   result = 0xff & x;
        return result;
}

static unsigned char    red(int c, int d, int e) {
	return limit((298 * c           + 409 * e + 128) >> 8);
}
static unsigned char    green(int c, int d, int e) {
	return limit((298 * c - 100 * d - 208 * e + 128) >> 8);
}
static unsigned char    blue(int c, int d, int e) {
	return limit((298 * c + 516 * d           + 128) >> 8);
}

/**
 * \brief Conversion of YUYV images to RGB
 *
 * In YUYV images, pairs of pixels containing two luminance values
 * and one chroma value u or v are converted to two adjacent RGB pixels.
 * Since the default transformation algorithm of the STL only handles
 * working with one pixel element at a time, we have to provide a
 * specialization of the imageConvert template function for YUYV pixels
 * as source images.
 */
template<>
void	imageConvert(Image<RGBPixel>& dest, const Image<YUYVPixel>& src) {
	if (dest.size != src.size) {
		throw std::length_error("image size mismatch");
	}
	int	offset = 0;
	while (offset < src.size.pixels) {
		int	c, d, e;
		c = src.pixels[offset].y - 16;
		d = src.pixels[offset].uv - 128;
		e = src.pixels[offset + 1].uv - 128;
		dest.pixels[offset].R = red(c, d, e);
		dest.pixels[offset].G = green(c, d, e);
		dest.pixels[offset].B = blue(c, d, e);
		offset++;
		c = src.pixels[offset].y - 16;
		dest.pixels[offset].R = red(c, d, e);
		dest.pixels[offset].G = green(c, d, e);
		dest.pixels[offset].B = blue(c, d, e);
		offset++;
	}
}

static unsigned char	Y(int R, int G, int B) {
	return limit(((  66 * R + 129 * G +  25 * B + 128) >> 8) +  16);
}
static unsigned char	U(int R, int G, int B) {
	return limit((( -38 * R -  74 * G + 112 * B + 128) >> 8) + 128);
}
static unsigned char	V(int R, int G, int B) {
	return limit((( 112 * R -  94 * G -  18 * B + 128) >> 8) + 128);
}

/**
 * \brief Convert RGB images into YUYV.
 *
 * In YUYV images, two pixels only contain one chroma value for each of
 * the chroma channels. This means that what value is actually stored in
 * the target array depends on whether we are converting an even numbered
 * or an odd numbered pixel. This specialization of the imageConvert
 * function thus implements a modified conversion algorithm for this type
 * of image.
 */
template<>
void	imageConvert(Image<YUYVPixel>& dest, const Image<RGBPixel>& src) {
	if (dest.size != src.size) {
		throw std::length_error("image size mismatch");
	}
	int	offset = 0;
	while (offset < src.size.pixels) {
#define	R	src.pixels[offset].R
#define	G	src.pixels[offset].G
#define	B	src.pixels[offset].B
		dest.pixels[offset].y = Y(R, G, B);
		dest.pixels[offset].uv = U(R, G, B);
		offset++;
		dest.pixels[offset].y = Y(R, G, B);
		dest.pixels[offset].uv = V(R, G, B);
		offset++;
#undef R
#undef G
#undef B
	}
}

/* convert a pair of RGB pixels into YUYV */

/*
Y = ( (  66 * R + 129 * G +  25 * B + 128) >> 8) +  16
U = ( ( -38 * R -  74 * G + 112 * B + 128) >> 8) + 128
V = ( ( 112 * R -  94 * G -  18 * B + 128) >> 8) + 128
*/

template<>
unsigned char	convert(const RGBPixel& p) {
	return ((66 * p.R + 129 * p.G +  25 * p.B + 128) >> 8) +  16;
}

} // namespace image
} // namespace astro
