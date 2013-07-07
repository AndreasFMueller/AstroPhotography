/**
 * PixelValue.h -- compute calibration frames
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _PixelValue_h
#define _PixelValue_h

#include <AstroImage.h>
#include <limits>
#include <debug.h>
#include <stdexcept>

using namespace astro::image;

namespace astro {
namespace image {

/**
 * \brief Adapter class for picture values of an ImagePtr
 *
 * This class allows access to the pixels of an image with primitive
 * pixel types, and performs an implicit type conversion to the type
 * we want to use in the calibration image computation.
 */
template<typename T>
class ConstPixelValue {
	const Image<unsigned char>	*byteimage;
	const Image<unsigned short>	*shortimage;
	const Image<unsigned int>	*intimage;
	const Image<unsigned long>	*longimage;
	const Image<float>	*floatimage;
	const Image<double>	*doubleimage;
public:
	ConstPixelValue(const ImagePtr& image);
	T	pixelvalue(unsigned int x, unsigned int y) const;
};

/**
 * \brief Constructor
 */
template<typename T>
ConstPixelValue<T>::ConstPixelValue(const ImagePtr& image) {
	byteimage = dynamic_cast<Image<unsigned char> *>(&*image);
	shortimage = dynamic_cast<Image<unsigned short> *>(&*image);
	intimage = dynamic_cast<Image<unsigned int> *>(&*image);
	longimage = dynamic_cast<Image<unsigned long> *>(&*image);
	floatimage = dynamic_cast<Image<float> *>(&*image);
	doubleimage = dynamic_cast<Image<double> *>(&*image);
	if ((NULL == byteimage) &&
	    (NULL == shortimage) &&
	    (NULL == intimage) &&
	    (NULL == longimage) &&
	    (NULL == floatimage) &&
	    (NULL == doubleimage)) {
		throw std::runtime_error("pixel type not primitive");
	}
}

/**
 * \brief Accessor to pixel value with implicit type conversion
 *
 * This method retrieves the pixel at point (x,y) and converts its
 * value to type T. If the conversion is not possible, a NaN is returned,
 * if available, or an exception thrown otherwise. Usually, calibration
 * images will be created with float or double types, so the exception
 * is not an issue.
 * \param x	x-coordinate of pixel
 * \param y	y-coordinate of pixel
 */
template<typename T>
T	ConstPixelValue<T>::pixelvalue(unsigned int x, unsigned int y) const {
	if (byteimage) {   return byteimage->pixelvalue<T>(x, y);   }
	if (shortimage) {  return shortimage->pixelvalue<T>(x, y);  }
	if (intimage) {    return intimage->pixelvalue<T>(x, y);    }
	if (longimage) {   return longimage->pixelvalue<T>(x, y);   }
	if (floatimage) {  return floatimage->pixelvalue<T>(x, y);  }
	if (doubleimage) { return doubleimage->pixelvalue<T>(x, y); }
	if (std::numeric_limits<T>::has_quiet_NaN) {
		return std::numeric_limits<T>::quiet_NaN();
	}
	throw std::runtime_error("NaN not available");
}

/**
 * \brief Access to pixel values
 *
 * This
 */
template<typename T>
class PixelValue {
	Image<unsigned char>	*byteimage;
	Image<unsigned short>	*shortimage;
	Image<unsigned int>	*intimage;
	Image<unsigned long>	*longimage;
	Image<float>	*floatimage;
	Image<double>	*doubleimage;
public:
	PixelValue(ImagePtr& image);
	T	pixelvalue(unsigned int x, unsigned int y);
};

template<typename T>
PixelValue<T>::PixelValue(ImagePtr& image) {
	byteimage = dynamic_cast<Image<unsigned char> *>(&*image);
	shortimage = dynamic_cast<Image<unsigned short> *>(&*image);
	intimage = dynamic_cast<Image<unsigned int> *>(&*image);
	longimage = dynamic_cast<Image<unsigned long> *>(&*image);
	floatimage = dynamic_cast<Image<float> *>(&*image);
	doubleimage = dynamic_cast<Image<double> *>(&*image);
	if ((NULL == byteimage) &&
	    (NULL == shortimage) &&
	    (NULL == intimage) &&
	    (NULL == longimage) &&
	    (NULL == floatimage) &&
	    (NULL == doubleimage)) {
		throw std::runtime_error("pixel type not primitive");
	}
}

template<typename T>
T	PixelValue<T>::pixelvalue(unsigned int x, unsigned int y) {
	if (byteimage) {   return byteimage->pixelvalue<T>(x, y);   }
	if (shortimage) {  return shortimage->pixelvalue<T>(x, y);  }
	if (intimage) {    return intimage->pixelvalue<T>(x, y);    }
	if (longimage) {   return longimage->pixelvalue<T>(x, y);   }
	if (floatimage) {  return floatimage->pixelvalue<T>(x, y);  }
	if (doubleimage) { return doubleimage->pixelvalue<T>(x, y); }
	if (std::numeric_limits<T>::has_quiet_NaN) {
		return std::numeric_limits<T>::quiet_NaN();
	}
	throw std::runtime_error("NaN not available");
}

} // image
} // astro

#endif /* _PixelValue_h */
