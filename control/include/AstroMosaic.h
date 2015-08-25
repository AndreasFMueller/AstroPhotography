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
	MosaicType	mosaic;
public:
	Mosaic(MosaicType::mosaic_type _mosaic);

	Image<T>	*operator()(const Image<RGB<T> >& image) const;
};

/**
 * \brief Constructor for the mosaicer.
 */
template<typename T>
Mosaic<T>::Mosaic(MosaicType::mosaic_type _mosaic) : mosaic(_mosaic) {
}

/**
 * \brief Mosaicing method
 * 
 * \param image    an RGB image to be reduced to a mosaic image.
 */
template<typename T>
Image<T>	*Mosaic<T>::operator()(const Image<RGB<T> >& image) const {
	Image<T>	*result = new Image<T>(image.getFrame().size());
	result->setMosaicType(mosaic.getMosaicType());
	ImagePoint	r = mosaic.red();
	ImagePoint	b = mosaic.blue();
	for (int x = 0; x < image.getFrame().size().width(); x += 2) {
		for (int y = 0; y < image.getFrame().size().height(); y += 2) {
			// red pixels
			result->pixel(x + r.x(), y + r.y())
				= image.pixel(x + r.x(), y + r.y()).R;
			// blue pixels
			result->pixel(x + b.x(), y + b.y())
				= image.pixel(x + r.x(), y + b.y()).B;
			// green pixels
			result->pixel(x + r.x(), y + b.y())
				= image.pixel(x + r.x(), y + b.y()).G;
			result->pixel(x + b.x(), y + r.y())
				= image.pixel(x + b.x(), y + r.y()).G;
		}
	}
	return result;
}

} // namespace astr
} // namespace image

#endif /* _AstroMosaic_h */
