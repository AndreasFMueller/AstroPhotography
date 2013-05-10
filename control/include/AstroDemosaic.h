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

/**
 * \brief The demosaicer base class.
 *
 * In the base class we have some common functions that potentially all
 * demosaicers will use.
 */
template<typename T>
class Demosaic {
protected:
	Image<RGB<T> >	*separate(const Image<T>& image);
public:
	Demosaic() { }
	Image<RGB<T> >	*operator()(const Image<T>& image);
};

/**
 * \brief Color separation
 *
 * This method just separates the color pixels into the color planes. 
 * Pixels about which we have no color information are left plack in
 * their color plane.
 */
template<typename T>
Image<RGB<T> >	*Demosaic<T>::separate(const Image<T>& image) {
	Image<RGB<T> >	*result = new Image<RGB<T> >(image.size);
	unsigned int	redx =  image.mosaic       & 0x1;
	unsigned int	redy = (image.mosaic >> 1) & 0x1;
	unsigned int	bluex = 0x1 ^ redx;
	unsigned int	bluey = 0x1 ^ redy;

	// set the image to black
	for (unsigned int x = 0; x < image.size.width; x++) {
		for (unsigned int y = 0; y < image.size.height; y++) {
			result->pixel(x, y).R = 0;
			result->pixel(x, y).G = 0;
			result->pixel(x, y).B = 0;
		}
	}

	// now set the pixels from the mosaic
	for (unsigned int x = 0; x < image.size.width; x += 2) {
		for (unsigned int y = 0; y < image.size.height; y += 2) {
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

/**
 * \brief Basic demosaicing function
 *
 * This is not really a demosaicer, as it just separates the color pixels
 * int the color planes.
 */
template<typename T>
Image<RGB<T> >	*Demosaic<T>::operator()(const Image<T>& image) {
	return separate(image);
}

/**
 * \brief The bilinear demosaicer.
 *
 * The "bilinear" demosaicer is a complete misnomer. What they mean when
 * they call it bilinear is that it behaves linearly in both directions.
 * But that is nothing but a linear function of the neighboring pixels,
 * so calling it linear would be more appropriate. 
 */
template<typename T>
class DemosaicBilinear : public Demosaic<T> {
	int	redx, redy;
	int	bluex, bluey;
	void	green(Image<RGB<T> > *result, const Image<T>& image);
	void	red(Image<RGB<T> > *result, const Image<T>& image);
	void	blue(Image<RGB<T> > *result, const Image<T>& image);

	T	quadt(int x, int y, const Image<T>& image);
	T	quadx(int x, int y, const Image<T>& image);
	T	pairh(int x, int y, const Image<T>& image);
	T	pairv(int x, int y, const Image<T>& image);
public:
	DemosaicBilinear() { }
	Image<RGB<T> >	*operator()(const Image<T>& image);
};

template<typename T>
void	DemosaicBilinear<T>::green(Image<RGB<T> > *result, const Image<T>& image) {
	for (unsigned int x = 0; x < image.size.width; x += 2) {
		for (unsigned int y = 0; y < image.size.height; y += 2) {
#if 0
			result->pixel(x + redx, y + redy).G
				= quadt(x + redx, y + redy, image);
			result->pixel(x + bluex, y + bluey).G
				= quadt(x + bluex, y + bluey, image);
#endif
		}
	}
}

template<typename T>
void	DemosaicBilinear<T>::red(Image<RGB<T> > *result, const Image<T>& image) {
	for (unsigned int x = 0; x < image.size.width; x += 2) {
		for (unsigned int y = 0; y < image.size.height; y += 2) {
#if 0
			result->pixel(x + bluex, y + bluey).R
				= quadx(x + bluex, y + bluey, image);
			result->pixel(x + redx, y + bluey).R
				= pairv(x + redx, y + bluey, image);
			result->pixel(x + bluex, y + redy).R
				= pairh(x + bluex, y + redy, image);
#endif
		}
	}
}

template<typename T>
void	DemosaicBilinear<T>::blue(Image<RGB<T> > *result, const Image<T>& image) {
	for (unsigned int x = 0; x < image.size.width; x += 2) {
		for (unsigned int y = 0; y < image.size.height; y += 2) {
#if 0
			result->pixel(x + redx, y + redy).B
				= quadx(x + redx, y + redy, image);
			result->pixel(x + redx, y + blue).B
				= pairh(x + redx, y + blue, image);
			result->pixel(x + bluex, y + redy).B
				= pairv(x + bluex, y +redy, image);
#endif
		}
	}
}

template<typename T>
Image<RGB<T> >	*DemosaicBilinear<T>::operator()(const Image<T>& image) {
	Image<RGB<T> >	*result = separate(image);

	// we don't want to call the isR functions all the time
	redx =  image.mosaic       & 0x1;
	redy = (image.mosaic >> 1) & 0x1;
	bluex = 0x1 ^ redx;
	bluey = 0x1 ^ redy;
	
	// fill in the green pixels
	green(result, image);

	// fill in the red pixels
	red(result, image);

	// fill in the blue pixels
	blue(result, image);

	return result;
}

} // namespace image
} // namespace astro

#endif /* _AstroDemosaic_h */
