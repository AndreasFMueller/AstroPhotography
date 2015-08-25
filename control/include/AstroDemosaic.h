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
	ImagePoint	r;
	ImagePoint	b;
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
	Image<RGB<T> >	*result = new Image<RGB<T> >(image.size());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "result RGB image %s created",
		result->size().toString().c_str());
	r = image.getMosaicType().red();
	b = image.getMosaicType().blue();

	// set the image to black
	for (int x = 0; x < image.size().width(); x++) {
		for (int y = 0; y < image.size().height(); y++) {
			result->pixel(x, y).R = 0;
			result->pixel(x, y).G = 0;
			result->pixel(x, y).B = 0;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image initialized to black");

	// now set the pixels from the mosaic
	for (int x = 0; x < image.size().width(); x += 2) {
		for (int y = 0; y < image.size().height(); y += 2) {
			result->pixel(x + r.x(), y + r.y()).R
				= image.pixel(x + r.x(), y + r.y());
			result->pixel(x + b.x(), y + b.y()).B
				= image.pixel(x + b.x(), y + b.y());
			result->pixel(x + r.x(), y + b.y()).G
				= image.pixel(x + r.x(), y + b.y());
			result->pixel(x + b.x(), y + r.y()).G
				= image.pixel(x + b.x(), y + r.y());
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
T	DemosaicBilinear<T>::quadt(int x, int y,
		const Image<T>& image) {
	double	result = 0;
	int	n = 0;
	if (x > 0) {
		result += image.pixel(x - 1, y); n++;
	}
	if (x < image.size().width() - 1) {
		result += image.pixel(x + 1, y); n++;
	}
	if (y > 0) {
		result += image.pixel(x, y - 1); n++;
	}
	if (y < image.size().height() - 1) {
		result += image.pixel(x, y + 1); n++;
	}
	result /= n;
	return (T)result;
}

template<typename T>
T	DemosaicBilinear<T>::quadx(int x, int y,
		const Image<T>& image) {
	double	result = 0;
	int	n = 0;
	if ((x > 0) && (y > 0)) {
		result += image.pixel(x - 1, y - 1); n++;
	}
	if ((x > 0) && (y < image.size().height() - 1)) {
		result += image.pixel(x - 1, y + 1); n++;
	}
	if ((x < image.size().width() - 1) && (y > 0)) {
		result += image.pixel(x + 1, y - 1); n++;
	} 
	if ((x < image.size().width() - 1) && (y < image.size().height() - 1)) {
		result += image.pixel(x + 1, y + 1); n++;
	}
	result /= n;
	return (T)result;
}

template<typename T>
T	DemosaicBilinear<T>::pairh(int x, int y,
		const Image<T>& image) {
	double	result = 0;
	int	n = 0;
	if (x > 0) {
		result += image.pixel(x - 1, y); n++;
	}
	if (x < image.size().width() - 1) {
		result += image.pixel(x + 1, y); n++;
	}
	result /= n;
	return (T)result;
}

template<typename T>
T	DemosaicBilinear<T>::pairv(int x, int y,
		const Image<T>& image) {
	double	result = 0;
	int	n = 0;
	if (y > 0) {
		result += image.pixel(x, y - 1); n++;
	}
	if (y < image.size().height() - 1) {
		result += image.pixel(x, y + 1); n++;
	}
	result /= n;
	return (T)result;
}

#define	r	Demosaic<T>::r
#define	b	Demosaic<T>::b

template<typename T>
void	DemosaicBilinear<T>::green(Image<RGB<T> > *result,
		const Image<T>& image) {
	for (int x = 0; x < image.size().width(); x += 2) {
		for (int y = 0; y < image.size().height(); y += 2) {
			result->pixel(x + r.x(), y + r.y()).G
				= quadt(x + r.x(), y + r.y(), image);
			result->pixel(x + b.x(), y + b.y()).G
				= quadt(x + b.x(), y + b.y(), image);
		}
	}
}

template<typename T>
void	DemosaicBilinear<T>::red(Image<RGB<T> > *result,
		const Image<T>& image) {
	for (int x = 0; x < image.size().width(); x += 2) {
		for (int y = 0; y < image.size().height(); y += 2) {
			result->pixel(x + b.x(), y + b.y()).R
				= quadx(x + b.x(), y + b.y(), image);
			result->pixel(x + r.x(), y + b.y()).R
				= pairv(x + r.x(), y + b.y(), image);
			result->pixel(x + b.x(), y + r.y()).R
				= pairh(x + b.x(), y + r.y(), image);
		}
	}
}

template<typename T>
void	DemosaicBilinear<T>::blue(Image<RGB<T> > *result,
		const Image<T>& image) {
	for (int x = 0; x < image.size().width(); x += 2) {
		for (int y = 0; y < image.size().height(); y += 2) {
			result->pixel(x + r.x(), y + r.y()).B
				= quadx(x + r.x(), y + r.y(), image);
			result->pixel(x + r.x(), y + b.y()).B
				= pairh(x + r.x(), y + b.y(), image);
			result->pixel(x + b.x(), y + r.y()).B
				= pairv(x + b.x(), y +r.y(), image);
		}
	}
}

#undef r
#undef b

template<typename T>
Image<RGB<T> >	*DemosaicBilinear<T>::operator()(const Image<T>& image) {
	Image<RGB<T> >	*result = this->separate(image);

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
