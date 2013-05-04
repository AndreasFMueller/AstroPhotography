/*
 * AstroDemosaic.h -- demosaicing methods
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroDemosaic_h
#define _AstroDemosaic_h

#include <AstroImage.h>

namespace astro {
namespace image {

template<typename T>
class Demosaic {
	Image<RGB<T> >	*separate(const Image<T>& image);
public:
	Demosaic() { }
	Image<RGB<T> >	*operator()(const Image<T>& image);
};


template<typename T>
Image<RGB<T> >	*Demosaic<T>::separate(const Image<T>& image) {
	Image<RGB<T> >	*result = new Image<RGB<T> >(image.size);
	int	redx =  image.mosaic       & 0x1;
	int	redy = (image.mosaic >> 1) & 0x1;
	int	bluex = 0x1 ^ redx;
	int	bluey = 0x1 ^ redy;

	// set the image to black
	for (int x = 0; x < image.size.width; x++) {
		for (int y = 0; y < image.size.height; y++) {
			result->pixel(x, y).R = 0;
			result->pixel(x, y).G = 0;
			result->pixel(x, y).B = 0;
		}
	}

	// now set the pixels from the mosaic
	for (int x = 0; x < image.size.width; x += 2) {
		for (int y = 0; y < image.size.height; y += 2) {
			result->pixel(x + redx, y + redy).R
				= image.pixel(x + redx, y + redy);
			result->pixel(x + bluex, y + bluey).B
				= image.pixel(x + bluex, y + bluey);
			result->pixel(x + redx, y + bluey).G
				= image.pixel(x + redx, y + bluey);
			result->pixel(x + bluex, y + redy).G
				= image.pixel(x + bluex, y + redy);
		}
	}
	return result;
}

template<typename T>
Image<RGB<T> >	*Demosaic<T>::operator()(const Image<T>& image) {
	return separate(image);
}

} // namespace image
} // namespace astro

#endif /* _AstroDemosaic_h */
