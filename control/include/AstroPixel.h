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

namespace astro {
namespace image {

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
 
template<typename destValue, typename srcValue>
void	convertPixelInteger(destValue& dest, const srcValue& src,
		Int2Type<7> x) {
	dest = destValue(src) << 56;
}

template<typename destValue, typename srcValue>
void	convertPixelInteger(destValue& dest, const srcValue& src,
		Int2Type<6> x) {
	dest = destValue(src) << 48;
}

template<typename destValue, typename srcValue>
void	convertPixelInteger(destValue& dest, const srcValue& src,
		Int2Type<5> x) {
	dest = destValue(src) << 40;
}

template<typename destValue, typename srcValue>
void	convertPixelInteger(destValue& dest, const srcValue& src,
		Int2Type<4> x) {
	dest = destValue(src) << 32;
}

template<typename destValue, typename srcValue>
void	convertPixelInteger(destValue& dest, const srcValue& src,
		Int2Type<3> x) {
	dest = destValue(src) << 24;
}

template<typename destValue, typename srcValue>
void	convertPixelInteger(destValue& dest, const srcValue& src,
		Int2Type<2> x) {
	dest = destValue(src) << 16;
}

template<typename destValue, typename srcValue>
void	convertPixelInteger(destValue& dest, const srcValue& src,
		Int2Type<1> x) {
	dest = destValue(src) << 8;
}

template<typename destValue, typename srcValue>
void	convertPixelInteger(destValue& dest, const srcValue& src,
		Int2Type<-1> x) {
	dest = destValue(src >> 8);
}

template<typename destValue, typename srcValue>
void	convertPixelInteger(destValue& dest, const srcValue& src,
		Int2Type<-2> x) {
	dest = destValue(src >> 16);
}

template<typename destValue, typename srcValue>
void	convertPixelInteger(destValue& dest, const srcValue& src,
		Int2Type<-3> x) {
	dest = destValue(src >> 24);
}

template<typename destValue, typename srcValue>
void	convertPixelInteger(destValue& dest, const srcValue& src,
		Int2Type<-4> x) {
	dest = destValue(src >> 32);
}

template<typename destValue, typename srcValue>
void	convertPixelInteger(destValue& dest, const srcValue& src,
		Int2Type<-5> x) {
	dest = destValue(src >> 40);
}

template<typename destValue, typename srcValue>
void	convertPixelInteger(destValue& dest, const srcValue& src,
		Int2Type<-6> x) {
	dest = destValue(src >> 48);
}

template<typename destValue, typename srcValue>
void	convertPixelInteger(destValue& dest, const srcValue& src,
		Int2Type<-7> x) {
	dest = destValue(src >> 56);
}

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
	typename isintegraldest, typename isintraldest>
void	convertPixelValueX(destValue& dest, const srcValue& src,
		isintegralsrc, isintegraldeset) {
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
class YUYV {
public:
	P	y;
	P	uv;
	bool	operator==(const YUYV<P>& other) const {
		return (y == other.y) && (uv == other.uv);
	}
	bool	operator!=(const YUYV<P>& other) const {
		return (y != other.y) || (uv != other.uv);
	}
	typedef yuyv_color_tag color_category;
	typedef	P	value_type;
};

/**
 * \brief RGB pixels of any type
 *
 * RGB pixels encode color with three independent color channels.
 */
template<typename P>
class RGB {
public:
	P	R;
	P	G;
	P	B;
	bool	operator==(const RGB<P>& other) const {
		return (R == other.R) && (G == other.G) && (B == other.B);
	}
	bool	operator!=(const RGB<P>& other) const {
		return !(*this == other);
	}
	typedef rgb_color_tag color_category;
	typedef	P	value_type;
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
struct  color_traits<char> {
	typedef monochrome_color_tag color_category;
};
template<>
struct  color_traits<unsigned short> {
	typedef monochrome_color_tag color_category;
};
template<>
struct  color_traits<short> {
	typedef monochrome_color_tag color_category;
};
template<>
struct  color_traits<unsigned int> {
	typedef monochrome_color_tag color_category;
};
template<>
struct  color_traits<int> {
	typedef monochrome_color_tag color_category;
};
template<>
struct  color_traits<unsigned long> {
	typedef monochrome_color_tag color_category;
};
template<>
struct  color_traits<long> {
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
	dest = destPixel(src);
}

template<typename destPixel, typename srcPixel>
void	convertPixelTyped(destPixel& dest, const srcPixel& src,
		const rgb_color_tag& dt, const monochrome_color_tag& ds) {
std::cerr << "rgb <- mono" << std::endl;
	convertPixelValue(dest.R, src);
	convertPixelValue(dest.G, src);
	convertPixelValue(dest.B, src);
}

template<typename destPixel, typename srcPixel>
void	convertPixelTyped(destPixel& dest, const srcPixel& src,
		const monochrome_color_tag& dt, const rgb_color_tag& ds) {
	typename srcPixel::value_type	y;
	y = 0.299 * src.R + 0.587 * src.G + 0.114 * src.B;
	convertPixelValue(dest, y);
}

template<typename destPixel, typename srcPixel>
void	convertPixelTyped(destPixel& dest, const srcPixel& src,
		const monochrome_color_tag& dt, const yuyv_color_tag& ds) {
	convertPixelValue(dest, src.y);
}

template<typename destPixel, typename srcPixel>
void	convertPixelTyped(destPixel& dest, const srcPixel& src,
		const yuyv_color_tag& dt, const monochrome_color_tag& ds) {
	convertPixelValue(dest.y, src);
	unsigned char	x = 128;
	convertPixelValue(dest.uv, x);
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
 */
template<typename destPixel, typename srcPixel,
	typename desttraits, typename srctraits>
void	convertPixelPairTyped(destPixel *dest, const srcPixel *src,
		const desttraits& dt, const srctraits& ds) {
	*dest++ = destPixel(src++);
	*dest = destPixel(src);
}

template<typename destPixel, typename srcPixel>
void	convertPixelPairTyped(destPixel *dest, const srcPixel *src,
		const yuyv_color_tag& dt, const rgb_color_tag& ds) {
	std::cerr << "rgb -> yuyv conversion" << std::endl;
}

template<typename destPixel, typename srcPixel>
void	convertPixelPairTyped(destPixel *dest, const srcPixel *src,
		const rgb_color_tag& dt, const yuyv_color_tag& ds) {
	std::cerr << "yuyv -> rgb conversion" << std::endl;
}

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
