/*
 * AstroMosaic.h -- Templates to convert 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroMosaic_h
#define _AstroMosaic_h

#include <AstroImage.h>

namespace astro {
namespace image {

/**
 * \brief Mosaicing class.
 *
 * This functor class convertes RGB images into Bayer mosaiced single
 * plane image. This is mainly used for testing the demosaicing classes.
 */
template<typename T>
class Mosaic {
	ImageBase::mosaic_type	mosaic;
public:
	Mosaic(ImageBase::mosaic_type _mosaic);

	Image<T>	*operator()(const Image<RGB<T> >& image) const;
};

/**
 * \brief Constructor for the mosaicer.
 */
template<typename T>
Mosaic<T>::Mosaic(ImageBase::mosaic_type _mosaic) : mosaic(_mosaic) {
}

/**
 * \brief Mosaicing method
 * 
 * \param image    an RGB image to be reduced to a mosaic image.
 */
template<typename T>
Image<T>	*Mosaic<T>::operator()(const Image<RGB<T> >& image) const {
	Image<T>	*result = new Image<T>(image.size);
	result->mosaic = mosaic;
	int	redx = mosaic & 0x1;
	int	redy = (mosaic >> 1) & 0x1;
	int	bluex = 0x1 ^ redx;
	int	bluey = 0x1 ^ redy;
	for (unsigned int x = 0; x < image.size.width; x += 2) {
		for (unsigned int y = 0; y < image.size.height; y += 2) {
			// red pixels
			result->pixel(x + redx, y + redy)
				= image.pixel(x + redx, y + redy).R;
			// blue pixels
			result->pixel(x + bluex, y + bluey)
				= image.pixel(x + redx, y + bluey).B;
			// green pixels
			result->pixel(x + redx, y + bluey)
				= image.pixel(x + redx, y + bluey).G;
			result->pixel(x + bluex, y + redy)
				= image.pixel(x + bluex, y + redy).G;
		}
	}
	return result;
}

} // namespace astr
} // namespace image

#endif /* _AstroMosaic_h */
