/*
 * AstroDemosaic.h -- demosaicing methods
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroDemosaic_h
#define _AstroDemosaic_h

#include <AstroImage.h>
#include <AstroDebug.h>

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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "result RGB image %s created",
		result->size.toString().c_str());
	unsigned int	redx =  image.getMosaicType()       & 0x1;
	unsigned int	redy = (image.getMosaicType() >> 1) & 0x1;
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image initialized to black");

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

	debug(LOG_DEBUG, DEBUG_LOG, 0, "color planes separated");
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

	T	quadt(unsigned int x, unsigned int y, const Image<T>& image);
	T	quadx(unsigned int x, unsigned int y, const Image<T>& image);
	T	pairh(unsigned int x, unsigned int y, const Image<T>& image);
	T	pairv(unsigned int x, unsigned int y, const Image<T>& image);
public:
	DemosaicBilinear() { }
	Image<RGB<T> >	*operator()(const Image<T>& image);
};

template<typename T>
T	DemosaicBilinear<T>::quadt(unsigned int x, unsigned int y,
		const Image<T>& image) {
	double	result = 0;
	int	n = 0;
	if (x > 0) {
		result += image.pixel(x - 1, y); n++;
	}
	if (x < image.size.width - 1) {
		result += image.pixel(x + 1, y); n++;
	}
	if (y > 0) {
		result += image.pixel(x, y - 1); n++;
	}
	if (y < image.size.height - 1) {
		result += image.pixel(x, y + 1); n++;
	}
	result /= n;
	return (T)result;
}

template<typename T>
T	DemosaicBilinear<T>::quadx(unsigned int x, unsigned int y,
		const Image<T>& image) {
	double	result = 0;
	int	n = 0;
	if ((x > 0) && (y > 0)) {
		result += image.pixel(x - 1, y - 1); n++;
	}
	if ((x > 0) && (y < image.size.height - 1)) {
		result += image.pixel(x - 1, y + 1); n++;
	}
	if ((x < image.size.width - 1) && (y > 0)) {
		result += image.pixel(x + 1, y - 1); n++;
	} 
	if ((x < image.size.width - 1) && (y < image.size.height - 1)) {
		result += image.pixel(x + 1, y + 1); n++;
	}
	result /= n;
	return (T)result;
}

template<typename T>
T	DemosaicBilinear<T>::pairh(unsigned int x, unsigned int y,
		const Image<T>& image) {
	double	result = 0;
	int	n = 0;
	if (x > 0) {
		result += image.pixel(x - 1, y); n++;
	}
	if (x < image.size.width - 1) {
		result += image.pixel(x + 1, y); n++;
	}
	result /= n;
	return (T)result;
}

template<typename T>
T	DemosaicBilinear<T>::pairv(unsigned int x, unsigned int y,
		const Image<T>& image) {
	double	result = 0;
	int	n = 0;
	if (y > 0) {
		result += image.pixel(x, y - 1); n++;
	}
	if (y < image.size.height - 1) {
		result += image.pixel(x, y + 1); n++;
	}
	result /= n;
	return (T)result;
}

template<typename T>
void	DemosaicBilinear<T>::green(Image<RGB<T> > *result,
		const Image<T>& image) {
	for (unsigned int x = 0; x < image.size.width; x += 2) {
		for (unsigned int y = 0; y < image.size.height; y += 2) {
			result->pixel(x + redx, y + redy).G
				= quadt(x + redx, y + redy, image);
			result->pixel(x + bluex, y + bluey).G
				= quadt(x + bluex, y + bluey, image);
		}
	}
}

template<typename T>
void	DemosaicBilinear<T>::red(Image<RGB<T> > *result,
		const Image<T>& image) {
	for (unsigned int x = 0; x < image.size.width; x += 2) {
		for (unsigned int y = 0; y < image.size.height; y += 2) {
			result->pixel(x + bluex, y + bluey).R
				= quadx(x + bluex, y + bluey, image);
			result->pixel(x + redx, y + bluey).R
				= pairv(x + redx, y + bluey, image);
			result->pixel(x + bluex, y + redy).R
				= pairh(x + bluex, y + redy, image);
		}
	}
}

template<typename T>
void	DemosaicBilinear<T>::blue(Image<RGB<T> > *result,
		const Image<T>& image) {
	for (unsigned int x = 0; x < image.size.width; x += 2) {
		for (unsigned int y = 0; y < image.size.height; y += 2) {
			result->pixel(x + redx, y + redy).B
				= quadx(x + redx, y + redy, image);
			result->pixel(x + redx, y + bluey).B
				= pairh(x + redx, y + bluey, image);
			result->pixel(x + bluex, y + redy).B
				= pairv(x + bluex, y +redy, image);
		}
	}
}

template<typename T>
Image<RGB<T> >	*DemosaicBilinear<T>::operator()(const Image<T>& image) {
	Image<RGB<T> >	*result = this->separate(image);

	// we don't want to call the isR functions all the time
	redx =  image.getMosaicType()       & 0x1;
	redy = (image.getMosaicType() >> 1) & 0x1;
	bluex = 0x1 ^ redx;
	bluey = 0x1 ^ redy;
	
	// fill in the green pixels
	debug(LOG_DEBUG, DEBUG_LOG, 0, "interpolate green pixels");
	green(result, image);

	// fill in the red pixels
	debug(LOG_DEBUG, DEBUG_LOG, 0, "interpolate red pixels");
	red(result, image);

	// fill in the blue pixels
	debug(LOG_DEBUG, DEBUG_LOG, 0, "interpolate blue pixels");
	blue(result, image);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "interpolation complete");
	return result;
}

ImagePtr	demosaic_bilinear(const ImagePtr& image);

} // namespace image
} // namespace astro

#endif /* _AstroDemosaic_h */
