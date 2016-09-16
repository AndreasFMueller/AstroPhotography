/*
 * AstroPixel.h -- types for pixels of various types
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#ifndef _AstroPixel_h
#define _AstroPixel_h

#include <stdexcept>
#include <memory>
#include <type_traits>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <limits>
#include <typeinfo>

namespace astro {
namespace image {

/**
 * \brief Template class to get the value type of pixels
 *
 * This structs are used as traits classes, but also as container
 * for parameters like maximum and minimum value of a type.
 */
template<typename Pixel>
struct pixel_value_type {
	typedef typename Pixel::value_type	value_type;
};
template<>
struct pixel_value_type<unsigned char> {
	typedef unsigned char	value_type;
};
template<>
struct pixel_value_type<unsigned short> {
	typedef unsigned short	value_type;
};
template<>
struct pixel_value_type<unsigned int> {
	typedef unsigned int	value_type;
};
template<>
struct pixel_value_type<unsigned long> {
	typedef unsigned long	value_type;
};
template<>
struct pixel_value_type<float> {
	typedef float	value_type;
};
template<>
struct pixel_value_type<double> {
	typedef double	value_type;
};

/**
 * \brief Wrapper to convert integers to typedefs for TMP
 *
 * This wrapper is from Andrei Alexandrescu's book "Modern C++ Design",
 * section 2.4. It is used to map the size difference of the various
 * pixel value types to a type, so that we can specialize pixel value
 * conversion functions based on this difference. Since we are only
 * dealing with function parameter specialization, not whole classes,
 * we cannot easily specialize integer arguments.
 */
template<long v>
struct Int2Type {
	enum { value = v };
};

/**
 * \brief Pixel values need to be converted in such a way as to minimize
 *        information loss
 * 
 * Pixel values are always basic types, but just about any unsigned integer
 * type or floating point type is acceptable. So we need a set of conversion
 * routines that can convert between those types. The conversion
 * algorithmus to use should further be selected at compile time, because
 * it is a selection that would potentially be done millions of times
 * during the conversion of an image, so it tends to turn into a performance
 * bottleneck.
 *
 * We solve this problem by employing template metaprogramming. First,
 * if one of the types is a floating point type, we just use the copy
 * constructor of the destination type for the conversion. The conversion
 * function convertPixelValue calls the default implementation of the
 * convertPixelX template function for this purpose. It also hands in
 * two instantiations of std::is_integral to identify to the compiler
 * whether the pixels are of integral type.
 *
 * A specialization of convertPixelValueX for the case where both
 * arguments are integer types calls the convertPixelInteger template
 * function with the size difference as third template argument.
 * The default implementation of convertPixelInteger again just uses
 * the copy constructor, which is appropriate if the pixel value types
 * are of the same size.
 *
 * If the sizes are different, then various specializations of the
 * size difference use different amounts of shift to better adapt pixel
 * values to the range offered by the data type. When a long type
 * is converted to a shorter type, less signification bits are
 * discarded. When a short type is converted to a longer one, less
 * significant bits are added with value 0. A conversion from a large
 * type to a small type and back thus only involves loss of the less
 * significant bits that do not fit into the small type. Contrast this
 * with the default copy constructor, which discards the most significant
 * bits of an integral type to fit it into a smaller one.
 */

template<typename destValue, typename srcValue, typename sizedifference>
void	convertPixelInteger(destValue& dest, const srcValue& src,
	sizedifference) {
	dest = destValue(src);
}

/**
 * \brief Specializations for various size differences
 */
#define CONVERT_PIXEL_INTEGER_SHIFT_LEFT(p)				\
template<typename destValue, typename srcValue>				\
void	convertPixelInteger(destValue& dest, const srcValue& src,	\
		Int2Type<p>) {						\
	dest = destValue(src) << (p << 3);				\
}
CONVERT_PIXEL_INTEGER_SHIFT_LEFT(7)
CONVERT_PIXEL_INTEGER_SHIFT_LEFT(6)
CONVERT_PIXEL_INTEGER_SHIFT_LEFT(5)
CONVERT_PIXEL_INTEGER_SHIFT_LEFT(4)
CONVERT_PIXEL_INTEGER_SHIFT_LEFT(3)
CONVERT_PIXEL_INTEGER_SHIFT_LEFT(2)
CONVERT_PIXEL_INTEGER_SHIFT_LEFT(1)

#define	CONVERT_PIXEL_INTEGER_SHIFT_RIGHT(p)				\
template<typename destValue, typename srcValue>				\
void	convertPixelInteger(destValue& dest, const srcValue& src,	\
		Int2Type<-p>) {						\
	dest = destValue(src >> (p << 3));				\
}

CONVERT_PIXEL_INTEGER_SHIFT_RIGHT(1)
CONVERT_PIXEL_INTEGER_SHIFT_RIGHT(2)
CONVERT_PIXEL_INTEGER_SHIFT_RIGHT(3)
CONVERT_PIXEL_INTEGER_SHIFT_RIGHT(4)
CONVERT_PIXEL_INTEGER_SHIFT_RIGHT(5)
CONVERT_PIXEL_INTEGER_SHIFT_RIGHT(6)
CONVERT_PIXEL_INTEGER_SHIFT_RIGHT(7)

/**
 * \brief conversionFunction template
 *
 * This template works around the problem that the compiler may convert
 * unsigned char to int, and thes the converPixelValueX template function
 * may not work correctly. This template is specialized for unsigned char
 * destination values, which means that the compiler cannot optimize the
 * function call away, so it will be called and thus the unsigned char
 * return time is really enforced.
 */
template<typename destValue, typename srcValue>
destValue	conversionFunction(const srcValue& src) {
	return destValue(src);
}

template<>
unsigned char	conversionFunction<unsigned char, float>(const float& src);
template<>
unsigned char	conversionFunction<unsigned char, double>(const double& src);

/**
 * \brief Convert Pixel values template.
 *
 * This template includes two additional template parameters that
 * identify whether the pixel type involved are integer or not.
 * Specializations then allow us to select at compile time a
 * special implementation for the case where both arguments are
 * integer types. 
 */
template<typename destValue, typename srcValue,
	typename isintegraldest, typename isintegralsrc>
void	convertPixelValueX(destValue& dest, const srcValue& src,
		isintegraldest, isintegralsrc) {
	dest = conversionFunction<destValue, srcValue>(src);
}

/**
 * \brief Convert integer pixels
 *
 * The actual conversions in case of both integer arguments depends on
 * the size difference of the two arguments. This template converts
 * the size difference into a type and uses it to call converPixelInteger,
 * which in turn uses a specialization appropriate for this particular
 * difference.
 */
template<typename destValue, typename srcValue>
void	convertPixelValueX(destValue& dest, const srcValue& src,
		std::true_type, std::true_type) {
	convertPixelInteger(dest, src,
		Int2Type<(long)sizeof(destValue) - (long)sizeof(srcValue)>());
}

/**
 * \brief Convert any pixel value to any other pixel value type.
 *
 * This template function just calls convertPixelValueX with two
 * additional arguments that convey to the compiler whether the types
 * are integral types.
 */
template<typename destValue, typename srcValue>
void	convertPixelValue(destValue& dest, const srcValue& src) {
	// otherweise use the default, which does not employ rescaling
	convertPixelValueX(dest, src,
		typename std::is_integral<destValue>::type(),
		typename std::is_integral<srcValue>::type());
}

/**
 * \brief The color_traits struct class is used to select color conversion
 *        methods at compile type;
 *
 * There are essentially three types of color classes that we consider:
 * monochrome images, where pixels are basic types, RGB color images, which
 * have three color values for each pixel, and YUYV color images, which
 * have a lumincance value Y for each pixel, and a U and V color info value
 * for every other pixel. We use the color_traits traits class to pack
 * that information into the pixel type.
 */
struct yuv_color_tag { };
struct yuyv_color_tag { };
struct rgb_color_tag { };
struct multiplane_color_tag { };
struct monochrome_color_tag { };

template<typename P>
struct color_traits {
	typedef typename P::color_category color_category;
};

/**
 * \brief Specializations of the color_traits traits class to indicate
 *        that basic types correspond to monochrome pixels
 */
template<>
struct  color_traits<unsigned char> {
	typedef monochrome_color_tag color_category;
};
template<>
struct  color_traits<unsigned short> {
	typedef monochrome_color_tag color_category;
};
template<>
struct  color_traits<unsigned int> {
	typedef monochrome_color_tag color_category;
};
template<>
struct  color_traits<unsigned long> {
	typedef monochrome_color_tag color_category;
};
template<>
struct  color_traits<float> {
	typedef monochrome_color_tag color_category;
};
template<>
struct  color_traits<double> {
	typedef monochrome_color_tag color_category;
};

/**
 * \brief Converting pixels between pixel types, taking color into account.
 *
 * We have three different pixel color traits classes, so there are 
 * essentially 9 different algorithms to convert pixel values. Conversions
 * to and from monochrome are always possible, and amount to just converting
 * pixel value types. Conversions between different instantiations  of RGB
 * or of YUYV likewise. Only conversions between the two color types 
 * have to handled differently, they only work on pairs.
 *
 * This is reflected by the hierarchy of types below. The convertPixelTyped
 * template by default uses the pixel type copy constructor, which works
 * for the conversions within a type (3 cases). The template detects the
 * color type from two additional template parameters containing the type
 * traits.  For the conversions to and from monochrome, two specializations
 * are provided that just copy luminance information. This gives us and
 * additional 4 conversion functions. The only functions missing are the
 * two conversions between RGB and YUYV.
 */
template<typename destPixel, typename srcPixel,
	typename desttraits, typename srctraits>
void	convertPixelTyped(destPixel& dest, const srcPixel& src,
		const desttraits&, const srctraits&) {
	convertPixelValue(dest, src);
}

/* monochrome -> RGB */
template<typename destPixel, typename srcPixel>
void	convertPixelTyped(destPixel& dest, const srcPixel& src,
		const rgb_color_tag&, const monochrome_color_tag&) {
	convertPixelValue(dest.R, src);
	dest.G = dest.R;
	dest.B = dest.R;
}

/* RGB -> monochrome */
template<typename destPixel, typename srcPixel>
void	convertPixelTyped(destPixel& dest, const srcPixel& src,
		const monochrome_color_tag&, const rgb_color_tag&) {
	typename srcPixel::value_type	y;
	y = src.luminance();
	convertPixelValue(dest, y);
}

/* YUYV -> monochrome */
template<typename destPixel, typename srcPixel>
void	convertPixelTyped(destPixel& dest, const srcPixel& src,
		const monochrome_color_tag&, const yuyv_color_tag&) {
	convertPixelValue(dest, src.y);
}

/*  monochrome -> YUYV */
template<typename destPixel, typename srcPixel>
void	convertPixelTyped(destPixel& dest, const srcPixel& src,
		const yuyv_color_tag&, const monochrome_color_tag&) {
	convertPixelValue(dest.y, src);
	dest.uv = destPixel::zero;
}

template<typename destPixel, typename srcPixel>
void	convertPixel(destPixel& dest, const srcPixel& src) {
	convertPixelTyped(dest, src,
		typename color_traits<destPixel>::color_category(),
		typename color_traits<srcPixel>::color_category());
}


template<typename P>
class Color {
public:
	typedef	P	value_type;
	static const value_type	pedestal;
	static const value_type	zero;
	static const value_type	limit;
	static P	clip(const double& value) {
		if (value < 0) { return 0; }
		if (value > limit) { return limit; }
		return value;
	}
#if 0
	virtual unsigned int	bytesPerValue() const {
		return sizeof(P);
	}
	virtual unsigned int	bitsPerValue() const {
		return std::numeric_limits<P>::digits;
	}
	virtual unsigned int	bytesPerPixel() const {
		return sizeof(P);
	}
	virtual unsigned int	bitsPerPixel() const {
		return 8 * bytesPerPixel();
	}
#endif

	static unsigned int	bytesPerValue() {
		return sizeof(P);
	}
	static unsigned int	bitesPerValue() {
		return std::numeric_limits<P>::digits;
	}
	static unsigned int	bytesPerPixel() {
		return sizeof(P);
	}
	static unsigned int	bitsPerPixel() {
		return 8 * bytesPerPixel();
	}
};

/**
 * \brief YUYV Pixels
 *
 * YUYV Colorspace encodes colors by providing a luminance (Y) value
 * in every pixel, and color (U, V) values in every other pixel.
 * So luminance can be reconstructed with full resolution, while 
 * color will be reconstructed with reduced fidelity. This usually
 * is not a problem, as the eye does not resolve color with the
 * same accuracy as luminance.
 *
 * Our images can use any primitive numeric type for the pixel values,
 * so YUYV, so we use a template class to create YUYV pixels. Note, however,
 * that there is no way to convert individual YUYV pixels to RGB or
 * back, as only pairs of pixels contain enough information.
 */
template<typename P>
class YUYV : public Color<P> {
public:
	P	y;
	P	uv;
	YUYV() : y(0), uv(0) { }
	YUYV(const P& _y, const P& _uv) : y(_y), uv(_uv) { }
	YUYV(const P& _y) : y(_y), uv(0) { }

	template<typename Q>
	YUYV(const Q& _y, const Q& _uv) {
		convertPixelValue(y, _y);
		convertPixelValue(uv, _uv);
	}
	virtual	~YUYV() { }

	/**
	 * \brief Copy constructor for YUYV pixels.
	 *
	 * Since the Y,U,V pixel values can be any integer type, which
	 * necessitates shifting them when converting from one type to
	 * another, we have to use the convertPixel functions when copying
	 * an YUYV pixel.
	 */
	template<typename Q>
	YUYV(const YUYV<Q>& q) {
		convertPixelValue(y, q.y);
		convertPixelValue(uv, q.uv);
	}

	bool	operator==(const YUYV<P>& other) const {
		return (y == other.y) && (uv == other.uv);
	}
	bool	operator!=(const YUYV<P>& other) const {
		return (y != other.y) || (uv != other.uv);
	}
	typedef yuyv_color_tag color_category;

	P	luminance() const {
		return y;
	}
	operator double() {
		return (double)luminance();
	}
#if 0
	virtual unsigned int	bytesPerPixel() const {
		return sizeof(*this);
	}
	virtual unsigned int	bitsPerPixel() const {
		return 8 * bytesPerPixel();
	}
#endif
	static unsigned int	bytesPerPixel() {
		return 2 * Color<P>::bytesPerValue();
	}
	static unsigned int	bitsPerPixel() {
		return 8 * bytesPerPixel();
	}
	static unsigned int	bytesPerValue() {
		return 2 * Color<P>::bytesPerValue();
	}
	static unsigned int	bitsPerValue() {
		return 8 * bytesPerPixel();
	}
};

/**
 * \brief YUV pixels
 *
 * In contrast to the YUYV pixels, this pixel type combines u and v in 
 * one pixel.
 */
template<typename P>
class YUV : public Color<P> {
public:
	P	y;
	P	u;
	P	v;
	YUV() { }
	YUV(const P& _y, const P& _u, const P& _v) : y(_y), u(_u), v(_v) { }
	YUV(const P& _y) : y(_y), u(0), v(0) { }
	virtual ~YUV() { }

	bool	operator==(const YUV<P>& other) const {
		return (y == other.y) && (u == other.u) && (v == other.v);
	}
	bool	operator!=(const YUV<P>& other) const {
		return (y != other.y) || (u != other.u) || (v != other.v);
	}
	typedef yuv_color_tag color_category;
	P	luminance() const {
		return y;
	}
	operator	double() {
		return (double)luminance();
	}
#if 0
	virtual unsigned int	bytesPerPixel() const {
		return sizeof(*this);
	}
	virtual unsigned int	bitsPerPixel() const {
		return 8 * bytesPerPixel();
	}
#endif
	static unsigned int	bytesPerPixel() {
		return 2 * Color<P>::bytesPerValue();
	}
	static unsigned int	bitsPerPixel() {
		return 8 * bytesPerPixel();
	}
};

/**
 * \brief Conversion class from HSL to RGB
 */
class HSLBase {
	double	_h;
	double	_s;
	double	_l;
public:
	double	h() const { return _h; }
	double	s() const { return _s; }
	double	l() const { return _l; }
private:
	double	_r, _g, _b;
public:
	HSLBase(double hue, double saturation, double luminance);
	double	r() const { return _r; };
	double	g() const { return _g; };
	double	b() const { return _b; };
};

/**
 * \brief Template class for HSL color with arbitrary pixel types
 */
template<typename P>
class HSL : public HSLBase {
public:
	HSL(double hue, double saturation, P luminance)
		: HSLBase(hue, saturation,
			luminance / std::numeric_limits<P>::max()) {
	}
	P	R() const { return std::numeric_limits<P>::max() * r(); }
	P	G() const { return std::numeric_limits<P>::max() * g(); }
	P	B() const { return std::numeric_limits<P>::max() * b(); }
};

template<>
HSL<double>::HSL(double hue, double saturation, double luminance);
template<>
double	HSL<double>::R() const;
template<>
double	HSL<double>::G() const;
template<>
double	HSL<double>::B() const;

template<>
HSL<float>::HSL(double hue, double saturation, float luminance);
template<>
float	HSL<float>::R() const;
template<>
float	HSL<float>::G() const;
template<>
float	HSL<float>::B() const;

/**
 * \brief RGB pixels of any type
 *
 * RGB pixels encode color with three independent color channels.
 */
template<typename P>
class RGB : public Color<P> {
public:
	P	R;
	P	G;
	P	B;
	RGB() : R(0), G(0), B(0) { }
	RGB(P w) : R(w), G(w), B(w) { }
	RGB(P r, P g, P b) : R(r), G(g), B(b) { }
	RGB(const HSL<P>& h) : R(h.R()), G(h.G()), B(h.B()) { }
	virtual ~RGB() { }

	template<typename Q>
	RGB(Q r, Q g, Q b) {
		convertPixel(R, r);
		convertPixel(G, g);
		convertPixel(B, b);
	}

	/**
	 * \brief Copy constructor for RGB pixels.
	 *
	 * Since the R,G,B pixel values can be any integer type, which
	 * necessitates shifting them when converting from one type to
	 * another, we have to use the convertPixel functions when copying
	 * an RGB pixel.
	 */
	template<typename Q>
	RGB(const RGB<Q>& q) {
		convertPixelValue(R, q.R);
		convertPixelValue(G, q.G);
		convertPixelValue(B, q.B);
	}

	bool	operator==(const RGB<P>& other) const {
		return (R == other.R) && (G == other.G) && (B == other.B);
	}
	bool	operator!=(const RGB<P>& other) const {
		return !(*this == other);
	}
	typedef rgb_color_tag color_category;

	// numeric operators on RGB pixels
	RGB<P>	operator+(const RGB<P>& other) const {
		RGB<P>	result;
		result.R = R + other.R;
		result.G = G + other.G;
		result.B = B + other.B;
		return result;
	}
	RGB<P>	operator-(const RGB<P>& other) const {
		RGB<P>	result;
		result.R = (R < other.R) ? 0 : (R - other.R);
		result.G = (G < other.G) ? 0 : (G - other.G);
		result.B = (B < other.B) ? 0 : (B - other.B);
		return result;
	}
	RGB<P>	operator*(const double value) const {
		RGB<P>	result;
		if ((R * value) > std::numeric_limits<P>::max()) {
			result.R = std::numeric_limits<P>::max();
		} else {
			result.R = R * value;
		}
		if ((G * value) > std::numeric_limits<P>::max()) {
			result.G = std::numeric_limits<P>::max();
		} else {
			result.G = G * value;
		}
		if ((B * value) > std::numeric_limits<P>::max()) {
			result.B = std::numeric_limits<P>::max();
		} else {
			result.B = B * value;
		}
		return result;
	}
	RGB<P>	operator*(const float value) const {
		return (*this) * (double)value;
	}

	RGB<P>	operator*(const RGB<P>& other) const {
		return RGB<P>(R * other.R, G * other.G, B * other.B);
	}

	RGB<P>	operator/(const RGB<P>& other) const {
		return RGB<P>(R / other.R, G / other.G, B / other.B);
	}

#if 0
	virtual unsigned int	bytesPerPixel() const {
		return sizeof(*this);
	}

	virtual unsigned int	bitsPerPixel() const {
		return 8 * bytesPerPixel();
	}
#endif

	static unsigned int	bytesPerPixel() {
		return 3 * Color<P>::bytesPerValue();
	}

	static unsigned int	bitsPerPixel() {
		return 8 * bytesPerPixel();
	}

	static unsigned int	bytesPerValue() {
		return Color<P>::bytesPerValue();
	}

	static unsigned int	bitsPerValue() {
		return 8 * bytesPerValue();
	}

	P	luminance() const {
		return 0.2126 * R + 0.7152 * G + 0.0722 * B;
	}

	operator double() {
		return (double)luminance();
	}

	RGB<P>	operator/(const P value) const {
		return RGB<P>(R / value, G / value, B / value);
	}

	P	max() const {
		P	result = R;
		if (G > result) { result = G; }
		if (B > result) { result = B; }
		return result;
	}

	P	min() const {
		P	result = R;
		if (G < result) { result = G; }
		if (B < result) { result = B; }
		return result;
	}

	P	sum() const {
		return R + G + B;
	}

	RGB<P>	inverse() const {
		return RGB<P>((P)(1. / R), (P)(1. / G), (P)(1. / B));
	}

	RGB<P>	normalize() const {
		double	l = ((double)R + (double)G + (double)B) / 3.;
		return (*this) * l;
	}

	RGB<P>	colorcomponents() const {
		double	l = luminance();
		return RGB<P>(R - l, G - l, B - l);
	}
};

/*
 * \brief Pixel classes with an arbitrary number of planes 
 *
 * Possible applications for this pixel class is stacking of images for
 * with the LRGB technique.
 */
template<typename P, int n>
class Multiplane : public Color<P> {
public:
	enum { planes = n};
	P	p[n];

	Multiplane() { }

	Multiplane(const P& v) {
		for (int i = 0; i < n; i++) {
			p[i] = v;
		}
	}

	template<typename Q>
	Multiplane(const Q& v) {
		for (int i = 0; i < n; i++) {
			p[i] = v;
		}
	}

	Multiplane(const RGB<P>& rgb) {
		int	i = 0;
		p[i++] = rgb.R;
		if (i >= n) return;
		p[i++] = rgb.G;
		if (i >= n) return;
		p[i++] = rgb.B;
		while (i < n) {
			p[i++] = 0;
		}
	}

	template<typename Q>
	Multiplane(const RGB<Q>& rgb) {
		int	i = 0;
		p[i++] = rgb.R;
		if (i >= n) return;
		p[i++] = rgb.G;
		if (i >= n) return;
		p[i++] = rgb.B;
		while (i < n) {
			p[i++] = 0;
		}
	}
	
	Multiplane(const Multiplane<P, n>& other) {
		for (int i = 0; i < n; i++) {
			p[i] = other.p[i];
		}
	}

	Multiplane<P,n>&	operator=(const Multiplane<P,n>& other) {
		for (int i = 0; i < n; i++) {
			p[i] = other.p[i];
		}
		return *this;
	}

#if 0
	virtual unsigned int	bytesPerPixel() const {
		return n * Color<P>::bytesPerValue();
	}

	virtual unsigned int	bitsPerPixel() const {
		return 8 * bytesPerPixel();
	}
#endif

	static unsigned int	bytesPerPixel() {
		return n * Color<P>::bytesPerValue();
	}

	static unsigned int	bitsPerPixel() {
		return 8 * bytesPerPixel();
	}

	static unsigned int	bytesPerValue() {
		return Color<P>::bytesPerValue();
	}

	static unsigned int	bitsPerValue() {
		return 8 * bytesPerValue();
	}

	bool	operator==(const Multiplane<P, n>& other) const {
		for (int i = 0; i < n; i++) {
			if (p[i] != other.p[i]) {
				return false;
			}
		}
		return true;
	}

	bool	operator!=(const Multiplane<P, n>& other) const {
		return !(*this == other);
	}

	typedef multiplane_color_tag color_category;

	// numeric operators on RGB pixels
	Multiplane<P, n>	operator+(const Multiplane<P, n>& other) const {
		Multiplane<P, n>	result;
		for (int i = 0; i < n; i++) {
			result.p[i] = p[i] + other.p[i];
		}
		return result;
	}

	Multiplane<P, n>	operator-(const Multiplane<P, n>& other) const {
		Multiplane<P, n>	result;
		for (int i = 0; i < n; i++) {
			result.p[i] = (p[i] < other.p[i])
					? 0
					: (p[i] - other.p[i]);
		}
		return result;
	}

	Multiplane<P, n>	operator*(const P value) const {
		Multiplane<P, n>	result;
		for (int i = 0; i < n; i++) {
			if ((p[i] * (double)value) > std::numeric_limits<P>::max()) {
				result.p[i] = std::numeric_limits<P>::max();
			} else {
				result.p[i] = p[i] * value;
			}
		}
		return result;
	}

	P	luminance() const {
		return p[0];
	}
};

/**
 * \brief Convert a pair of pixels
 *
 * This template converts between pairs of pixels. Its default implementation
 * calls the typed conversion template for each pixel. Only for YUYV-RGB
 * conversions, two specializations that do "the right thing" are
 * provided. For efficience reasons, there are further specializations
 * just for the important case of unsigned char integral type values.
 *
 * The color space conversion formulae used below are documented here:
 * http://msdn.microsoft.com/en-us/library/windows/desktop/dd206750(v=vs.85).aspx
 */
template<typename destPixel, typename srcPixel,
	typename desttraits, typename srctraits>
void	convertPixelPairTyped(destPixel *dest, const srcPixel *src,
		const desttraits&, const srctraits&) {
	convertPixel(*dest, *src);
	dest++; src++;
	convertPixel(*dest, *src);
}

/* conversion RGB --> YUYV */
template<typename destPixel, typename srcPixel>
void	convertPixelPairTyped(destPixel *dest, const srcPixel *src,
		const yuyv_color_tag&, const rgb_color_tag&) {
	typename srcPixel::value_type	Y, U, V;
	Y = round( 0.256788 * src[0].R
		 + 0.504129 * src[0].G
		 + 0.097906 * src[0].B) +  srcPixel::pedestal;
	convertPixelValue(dest[0].y, Y);
	U = round(- 0.148223 * src[0].R
		  - 0.290993 * src[0].G
		  + 0.439216 * src[0].B) + srcPixel::zero;
	convertPixelValue(dest[0].uv, U);
	Y = round( 0.256788 * src[1].R
		 + 0.504129 * src[1].G
		 + 0.097906 * src[1].B) +  srcPixel::pedestal;
	convertPixelValue(dest[1].y, Y);
	V = round( 0.439216 * src[1].R
		 - 0.367788 * src[1].G
		 - 0.071427 * src[1].B) + srcPixel::zero;
	convertPixelValue(dest[1].uv, V);
}

template<>
void	convertPixelPairTyped(YUYV<unsigned char> *dest,
		const RGB<unsigned char> *src,
		const yuyv_color_tag&, const rgb_color_tag&);

/* conversion YUYV --> RGB */
template<typename destPixel, typename srcPixel>
void	convertPixelPairTyped(destPixel *dest, const srcPixel *src,
		const rgb_color_tag&, const yuyv_color_tag&) {
	double	C, D, E;
	typename srcPixel::value_type	R, G, B;
	typename srcPixel::value_type	lower;
	convertPixelValue(lower, (unsigned char)16);
	typename srcPixel::value_type	middle;
	convertPixelValue(middle, (unsigned char)128);
	C = src[0].y - lower;
	D = src[0].uv - middle;
	E = src[1].uv - middle;
	R = srcPixel::clip(
		round( 1.164383 * C                   + 1.596027 * E  ) );
	G = srcPixel::clip(
		round( 1.164383 * C - (0.391762 * D) - (0.812968 * E) ) );
	B = srcPixel::clip(
		round( 1.164383 * C +  2.017232 * D                   ) );
	convertPixelValue(dest[0].R, R);
	convertPixelValue(dest[0].G, G);
	convertPixelValue(dest[0].B, B);
	C = src[1].y - lower;
	R = srcPixel::clip(
		round( 1.164383 * C                   + 1.596027 * E  ) );
	G = srcPixel::clip(
		round( 1.164383 * C - (0.391762 * D) - (0.812968 * E) ) );
	B = srcPixel::clip(
		round( 1.164383 * C +  2.017232 * D                   ) );
	convertPixelValue(dest[1].R, R);
	convertPixelValue(dest[1].G, G);
	convertPixelValue(dest[1].B, B);
}

template<>
void	convertPixelPairTyped(RGB<unsigned char> *dest,
		const YUYV<unsigned char> *src,
		const rgb_color_tag&, const yuyv_color_tag&);

template<typename destPixel, typename srcPixel>
void	convertPixelPair(destPixel *dest, const srcPixel *src) {
	convertPixelPairTyped(dest, src,
		typename color_traits<destPixel>::color_category(),
		typename color_traits<srcPixel>::color_category());
}

/**
 * \brief Convert an array of Pixels.
 *
 * This template uses the converPixelPair template function to convert
 * between pixel types. Applications should only call this template to
 * convert color images.
 */
template<typename destPixel, typename srcPixel>
void	convertPixelArray(destPixel *dest, const srcPixel *src, int length) {
	for (int i = 0; i < length; i += 2) {
		convertPixelPair(&dest[i], &src[i]);
	}
}

/**
 * \brief template with specialisations for the Planes function
 */
template<typename P>
unsigned int	planes(P) {
	return 1;
}

template<typename P>
unsigned int	planes(YUYV<P>) {
	return 2;
}

template<typename P>
unsigned int	planes(RGB<P>) {
	return 3;
}

template<typename P, int n>
unsigned int	planes(Multiplane<P, n>) {
	return n;
}

/**
 * \brief template with specializations for the bitsPerPixel function
 */
template<typename P>
unsigned int	bitsPerPixel(P) {
	return std::numeric_limits<P>::digits;
}

template<typename P>
unsigned int	bitsPerPixel(YUYV<P>) {
	return YUYV<P>::bitsPerPixel();
}

template<typename P>
unsigned int	bitsPerPixel(YUV<P>) {
	return YUV<P>::bitsPerPixel();
}

template<typename P>
unsigned int	bitsPerPixel(RGB<P>) {
	return RGB<P>::bitsPerPixel();
}

template<typename P, int n>
unsigned int	bitsPerPixel(Multiplane<P, n>) {
	return Multiplane<P,n>::bitsPerPixel();
}

/**
 * \brief bytesPerPixel template function
 */
template<typename P>
unsigned int	bytesPerPixel(P) {
	return sizeof(P);
}

template<typename P>
unsigned int	bytesPerPixel(YUYV<P>) {
	return YUYV<P>::bytesPerPixel();
}

template<typename P>
unsigned int	bytesPerPixel(YUV<P>) {
	return YUV<P>::bytesPerPixel();
}

template<typename P>
unsigned int	bytesPerPixel(RGB<P>) {
	return RGB<P>::bytesPerPixel();
}

template<typename P, int n>
unsigned int	bytesPerPixel(Multiplane<P,n>) {
	return Multiplane<P,n>::bytesPerPixel();
}

/**
 * \brief template with specializations for the bitsPerValue function
 */
template<typename P>
unsigned int	bitsPerValue(P) {
	return std::numeric_limits<P>::digits;
}

template<typename P>
unsigned int	bitsPerValue(YUYV<P>) {
	return YUYV<P>::bitsPerValue();
}

template<typename P>
unsigned int	bitsPerValue(YUV<P>) {
	return YUV<P>::bitsPerValue();
}

template<typename P>
unsigned int	bitsPerValue(RGB<P>) {
	return RGB<P>::bitsPerValue();
}

template<typename P, int n>
unsigned int	bitsPerValue(Multiplane<P, n>) {
	return Multiplane<P,n>::bitsPerValue();
}

/**
 * \brief bytesPerValue template function
 */
template<typename P>
unsigned int	bytesPerValue(P) {
	return sizeof(P);
}

template<typename P>
unsigned int	bytesPerValue(YUYV<P>) {
	return YUYV<P>::bytesPerValue();
}

template<typename P>
unsigned int	bytesPerValue(YUV<P>) {
	return YUV<P>::bytesPerValue();
}

template<typename P>
unsigned int	bytesPerValue(RGB<P>) {
	return RGB<P>::bytesPerValue();
}

template<typename P, int n>
unsigned int	bytesPerValue(Multiplane<P,n>) {
	return Multiplane<P,n>::bytesPerValue();
}


/**
 * \bits Weighted sum of Pixels
 *
 * The most important function when performing Image transformations is
 * the ability to compute a weighted sum of pixels. The problem is that
 * information is lost when this is done in the pixel type arithemtic,
 * especially for the very small types like unsigned char. Therefore 
 * we create this template function with suitable specialisations so
 * that Weighted averags can be computed for every type of pixel
 */
template<typename Pixel>
Pixel	weighted_sum_typed(unsigned int number_of_terms, const double *weights,
		const Pixel *pixels, const monochrome_color_tag&) {
	double	result = 0;
	double	weightsum = 0;
	for (unsigned int i = 0; i < number_of_terms; i++) {
		result = result + pixels[i] * weights[i];
		weightsum += weights[i];
	}
	return result * (1./weightsum);
}

template<typename Pixel>
Pixel	weighted_sum_typed(unsigned int number_of_terms,
			const double *weights, const Pixel *pixels,
			const rgb_color_tag&) {
	RGB<double>	result = 0;
	double	weightsum = 0;
	for (unsigned int i = 0; i < number_of_terms; i++) {
		RGB<double>	summand(pixels[i].R, pixels[i].G, pixels[i].B);
		result = result + summand * weights[i];
		weightsum += weights[i];
	}
	return result * (1./weightsum);
	return Pixel(result.R, result.G, result.B);
}

template<typename Pixel, int n>
Multiplane<Pixel, n>	weighted_sum_typed_n(unsigned int number_of_terms,
				const double *weights,
				const Multiplane<Pixel, n> *pixels,
				const multiplane_color_tag&) {
	Multiplane<double, n>	result = 0;
	double	weightsum = 0;
	for (unsigned int i = 0; i < number_of_terms; i++) {
		Multiplane<double, n>	summand(pixels[i]);
		result = result + summand * weights[i];
		weightsum += weights[i];
	}
	return result * (1./weightsum);
	return Pixel(result);
}


template<typename Pixel>
Pixel	weighted_sum_typed(unsigned int number_of_terms,
			const double *weights, const Pixel *pixels,
			const multiplane_color_tag&) {
	return weighted_sum_typed_n<Pixel::value_type, Pixel::planes>(
		number_of_terms, weights, pixels, multiplane_color_tag());
}

template<typename Pixel>
Pixel	weighted_sum(unsigned int number_of_terms,
		const double *weights, const Pixel *pixels) {
	return weighted_sum_typed(number_of_terms, weights, pixels,
		typename color_traits<Pixel>::color_category());
}

/**
 * \brief Luminance operators
 */
template<typename Pixel>
class Luminance {
public:
	double	operator()(const RGB<Pixel>&) = 0;
};

template<typename Pixel>
class StandardLuminance : public Luminance<Pixel> {
public:
	double	operator()(const RGB<Pixel>& p) {
		return 0.2126 * p.R + 0.7152 * p.G + 0.0722 * p.B;
	}
};

template<typename Pixel>
class GreenLuminance : public Luminance<Pixel> {
public:
	double	operator()(const RGB<Pixel>& p) {
		return p.G;
	}
};

template<typename Pixel>
class CCIRLuminance : public Luminance<Pixel> {
public:
	double	operator()(const RGB<Pixel>& p) {
		return 0.299 * p.R + 0.587 * p.G + 0.114 * p.B;
	}
};

template<typename Pixel>
class SqrtLuminance : public Luminance<Pixel> {
public:
	double	operator()(const RGB<Pixel>& p) {
		return sqrt(0.241 * p.R * p.R + 0.691 * p.G * p.G
				+ 0.068 * p.B * p.B);
	}
};

/**
 * \brief Luminance function
 *
 * Retrieve luminance information from a pixel, independent of time.
 */
template<typename Pixel>
double	luminance_typed(const Pixel& pixel, const monochrome_color_tag&) {
	return pixel;
}

template<typename Pixel>
double	luminance_typed(const Pixel& pixel, const multiplane_color_tag&) {
	return pixel.luminance();
}

template<typename Pixel>
double	luminance_typed(const Pixel& pixel, const rgb_color_tag&) {
	return pixel.luminance();
}

template<typename Pixel>
double	luminance_typed(const Pixel& pixel, const yuyv_color_tag&) {
	return pixel.luminance();
}

template<typename Pixel>
double	luminance(const Pixel& pixel) {
	return luminance_typed(pixel,
		typename color_traits<Pixel>::color_category());
}

/**
 * \brief Find the maximum possible value for a pixel type
 */
template<typename Pixel>
double	maximum_typed(const Pixel&, const monochrome_color_tag&) {
	return std::numeric_limits<Pixel>::max();
}

template<typename Pixel>
double	maximum_typed(const Pixel&, const multiplane_color_tag&) {
	return std::numeric_limits<typename Pixel::value_type>::max();
}

template<typename Pixel>
double	maximum_typed(const Pixel&, const rgb_color_tag&) {
	return std::numeric_limits<typename Pixel::value_type>::max();
}

template<typename Pixel>
double	maximum_typed(const Pixel&, const yuyv_color_tag&) {
	return std::numeric_limits<typename Pixel::value_type>::max();
}

template<typename Pixel>
double pixel_maximum() {
	return maximum_typed(Pixel(),
			typename color_traits<Pixel>::color_category());
}

/**
 * \brief Find the red/green/blue values
 */
template<typename Pixel>
double	red_typed(const Pixel& pixel, const monochrome_color_tag&) {
	return pixel;
}

template<typename Pixel>
double	red_typed(const Pixel& pixel, const rgb_color_tag&) {
	return pixel.R();
}

template<typename Pixel>
double	red(const Pixel& pixel) {
	return red_typed(pixel, typename color_traits<Pixel>::color_category());
}

template<typename Pixel>
double	green_typed(const Pixel& pixel, const monochrome_color_tag&) {
	return pixel;
}

template<typename Pixel>
double	green_typed(const Pixel& pixel, const rgb_color_tag&) {
	return pixel.G();
}

template<typename Pixel>
double	green(const Pixel& pixel) {
	return green_typed(pixel, typename color_traits<Pixel>::color_category());
}

template<typename Pixel>
double	blue_typed(const Pixel& pixel, const monochrome_color_tag&) {
	return pixel;
}

template<typename Pixel>
double	blue_typed(const Pixel& pixel, const rgb_color_tag&) {
	return pixel.G();
}

template<typename Pixel>
double	blue(const Pixel& pixel) {
	return blue_typed(pixel, typename color_traits<Pixel>::color_category());
}

} // namespace image
} // namespace astro

#endif /* _AstroPixel_h */
