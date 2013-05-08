/*
 * AstroPixel.h -- types for pixels of various types
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#ifndef _AstroPixel_h
#define _AstroPixel_h

#include <stdexcept>
#include <tr1/memory>
#include <tr1/type_traits>
#include <algorithm>
#include <iostream>
#include <math.h>

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
	static Pixel	max_value;
};
template<>
struct pixel_value_type<unsigned char> {
	typedef unsigned char	value_type;
	static unsigned char	max_value;
};
template<>
struct pixel_value_type<unsigned short> {
	typedef unsigned short	value_type;
	static unsigned short	max_value;
};
template<>
struct pixel_value_type<unsigned int> {
	typedef unsigned int	value_type;
	static unsigned int	max_value;
};
template<>
struct pixel_value_type<unsigned long> {
	typedef unsigned long	value_type;
	static unsigned long	max_value;
};
template<>
struct pixel_value_type<float> {
	typedef float	value_type;
	static float	max_value;
};
template<>
struct pixel_value_type<double> {
	typedef double	value_type;
	static double	max_value;
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
template<int v>
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
 * two instantiations of std::tr1:is_integral to identify to the compiler
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
	sizedifference x) {
	dest = destValue(src);
}

/**
 * \brief Specializations for various size differences
 */
#define CONVERT_PIXEL_INTEGER_SHIFT_LEFT(p)				\
template<typename destValue, typename srcValue>				\
void	convertPixelInteger(destValue& dest, const srcValue& src,	\
		Int2Type<p> x) {					\
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
		Int2Type<-p> x) {					\
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
	dest = destValue(src);
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
		std::tr1::true_type, std::tr1::true_type) {
	convertPixelInteger(dest, src,
		Int2Type<sizeof(destValue) - sizeof(srcValue)>());
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
		typename std::tr1::is_integral<destValue>::type(),
		typename std::tr1::is_integral<srcValue>::type());
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
struct yuyv_color_tag { };
struct rgb_color_tag { };
struct monochrome_color_tag { };

template<typename P>
struct color_traits {
	typedef typename P::color_category color_category;
};

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
	YUYV() { }
	YUYV(const P& _y, const P& _uv) : y(_y), uv(_uv) { }

	template<typename Q>
	YUYV(const Q& _y, const Q& _uv) {
		convertPixelValue(y, _y);
		convertPixelValue(uv, _uv);
	}

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
};

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
	RGB() { }
	RGB(P r, P g, P b) : R(r), G(g), B(b) { }

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
		const desttraits& dt, const srctraits& ds) {
	convertPixelValue(dest, src);
}

/* monochrome -> RGB */
template<typename destPixel, typename srcPixel>
void	convertPixelTyped(destPixel& dest, const srcPixel& src,
		const rgb_color_tag& dt, const monochrome_color_tag& ds) {
	convertPixelValue(dest.R, src);
	dest.G = dest.R;
	dest.B = dest.R;
}

/* RGB -> monochrome */
template<typename destPixel, typename srcPixel>
void	convertPixelTyped(destPixel& dest, const srcPixel& src,
		const monochrome_color_tag& dt, const rgb_color_tag& ds) {
	typename srcPixel::value_type	y;
	y = 0.299 * src.R + 0.587 * src.G + 0.114 * src.B;
	convertPixelValue(dest, y);
}

/* YUYV -> monochrome */
template<typename destPixel, typename srcPixel>
void	convertPixelTyped(destPixel& dest, const srcPixel& src,
		const monochrome_color_tag& dt, const yuyv_color_tag& ds) {
	convertPixelValue(dest, src.y);
}

/*  monochrome -> YUYV */
template<typename destPixel, typename srcPixel>
void	convertPixelTyped(destPixel& dest, const srcPixel& src,
		const yuyv_color_tag& dt, const monochrome_color_tag& ds) {
	convertPixelValue(dest.y, src);
	dest.uv = destPixel::zero;
}

template<typename destPixel, typename srcPixel>
void	convertPixel(destPixel& dest, const srcPixel& src) {
	convertPixelTyped(dest, src,
		typename color_traits<destPixel>::color_category(),
		typename color_traits<srcPixel>::color_category());
}

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
		const desttraits& dt, const srctraits& ds) {
	convertPixel(*dest, *src);
	dest++; src++;
	convertPixel(*dest, *src);
}

/* conversion RGB --> YUYV */
template<typename destPixel, typename srcPixel>
void	convertPixelPairTyped(destPixel *dest, const srcPixel *src,
		const yuyv_color_tag& dt, const rgb_color_tag& ds) {
	typename srcPixel::value_type	Y, U, V;
	Y = round( 0.256788 * src[0].R
		 + 0.504129 * src[0].G
		 + 0.097906 * src[0].B) +  srcPixel::pedestal;
	convertPixelValue(dest[0].y, Y);
	U = round(-0.148223 * src[0].R
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
		const yuyv_color_tag& dt, const rgb_color_tag& ds);

/* conversion YUYV --> RGB */
template<typename destPixel, typename srcPixel>
void	convertPixelPairTyped(destPixel *dest, const srcPixel *src,
		const rgb_color_tag& dt, const yuyv_color_tag& ds) {
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
		const rgb_color_tag& dt, const yuyv_color_tag& ds);

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

} // namespace image
} // namespace astro

#endif /* _AstroPixel_h */
