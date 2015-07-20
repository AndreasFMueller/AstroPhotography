/*
 * RGBPixel.cpp -- Functions related to the RGBPixel type
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroPixel.h>
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
 * \brief Functions to convert from a pair of YUYV Pixels to a
 *        pair of RGB pixels
 */
void    YUYV2RGB(const YUYVPixel yuyv[2], RGBPixel rgb[2]) {
	int	c, d, e;
	c = yuyv[0].y - 16;
	d = yuyv[0].uv - 128;
	e = yuyv[1].uv - 128;
	rgb[0].R = red(c, d, e);
	rgb[0].G = green(c, d, e);
	rgb[0].B = blue(c, d, e);
	c = yuyv[1].y - 16;
	rgb[1].R = red(c, d, e);
	rgb[1].G = green(c, d, e);
	rgb[1].B = blue(c, d, e);
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
	for (int offset = 0; offset < src.size.pixels; offset += 2) {
		YUYV2RGB(&src.pixels[offset], &dest.pixels[offset]);
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
 * \brief Function to convert from a pair of RGB Pixels to a
 *        pair of YUYV pixels
 */
void    RGB2YUYV(const RGBPixel rgb[2], YUYVPixel yuyv[2]) {
	yuyv[0].y = Y(rgb[0].R, rgb[0].G, rgb[0].B);
	yuyv[0].uv = U(rgb[0].R, rgb[0].G, rgb[0].B);
	yuyv[1].y = Y(rgb[1].R, rgb[1].G, rgb[1].B);
	yuyv[1].uv = V(rgb[1].R, rgb[1].G, rgb[1].B);
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
	for (int offset = 0; offset < src.size.pixels; offset += 2) {
		RGB2YUYV(&src.pixels[offset], &dest.pixels[offset]);
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
