/**
 * Interpolation.cpp -- interpolate images
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroInterpolation.h>
#include <limits>
#include <AstroDebug.h>
#include <stdexcept>

using namespace astro::image;

namespace astro {
namespace interpolation {

//////////////////////////////////////////////////////////////////////
// TypedInterpolator implementation
//////////////////////////////////////////////////////////////////////
template<typename DarkPixelType, typename Pixel>
class TypedInterpolator {
protected:
	const Image<DarkPixelType>&	dark;
	virtual DarkPixelType	darkpixel(unsigned int x, unsigned int y) const;
	virtual void	interpolatePixel(unsigned int x, unsigned int y,
				ImageAdapter<Pixel>& image) = 0;
	DarkPixelType	nan;
public:
	TypedInterpolator(const Image<DarkPixelType>& _dark);
	void	interpolate(ImageAdapter<Pixel>& image);
};


template<typename DarkPixelType, typename Pixel>
DarkPixelType TypedInterpolator<DarkPixelType, Pixel>::darkpixel(
	unsigned int x, unsigned int y) const {
	return dark.pixel(x, y);
}

template<typename DarkPixelType, typename Pixel>
TypedInterpolator<DarkPixelType, Pixel>::TypedInterpolator(
	const Image<DarkPixelType>& _dark) : dark(_dark) {
	nan = std::numeric_limits<DarkPixelType>::quiet_NaN();
}

template<typename DarkPixelType, typename Pixel>
void	TypedInterpolator<DarkPixelType, Pixel>::interpolate(
		ImageAdapter<Pixel>& image) {
	// make sure the image sizes match
	if (image.getSize() != dark.size()) {
		throw std::range_error("image sizes don't match");
	}
	for (unsigned int x = 0; x < dark.size().width(); x++) {
		for (unsigned int y = 0; y < dark.size().height(); y++) {
			DarkPixelType	darkpixel = dark.pixel(x, y);
			if (darkpixel != darkpixel) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"interpolating pixel (%u,%u)", x, y);
				this->interpolatePixel(x, y, image);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Monochrome interpolator
//////////////////////////////////////////////////////////////////////
template<typename DarkPixelType, typename Pixel>
class MonochromeInterpolator : public TypedInterpolator<DarkPixelType, Pixel> {
protected:
	virtual DarkPixelType	darkpixel(unsigned int x, unsigned int y) const;
	virtual void	interpolatePixel(unsigned int x, unsigned int y,
				ImageAdapter<Pixel>& image);
public:
	MonochromeInterpolator(const Image<DarkPixelType>& _dark);
};

template<typename DarkPixelType, typename Pixel>
DarkPixelType	MonochromeInterpolator<DarkPixelType, Pixel>::darkpixel(
	unsigned int x, unsigned int y) const {
	return TypedInterpolator<DarkPixelType, Pixel>::darkpixel(x, y);
}

template<typename DarkPixelType, typename Pixel>
MonochromeInterpolator<DarkPixelType, Pixel>::MonochromeInterpolator(
	const Image<DarkPixelType>& _dark)
	: TypedInterpolator<DarkPixelType, Pixel>(_dark) {
}

/**
 * \brief Interpolation function
 *
 * Interpolation is actually two quite different cases: monochrome and
 * Bayer matrix. Monochrome interpolation interpolates the four nearest
 * neighbor points. Bayer Matrix interpolation depends on the color.
 * For green pixel, the four diagonal neighbors are interpolated. For
 * red and blue, the for nearest neighbours of the same color are interpolated.
 *
 */
template<typename DarkPixelType, typename Pixel>
void	MonochromeInterpolator<DarkPixelType, Pixel>::interpolatePixel(
	unsigned int x, unsigned int y, ImageAdapter<Pixel>& image) {
	unsigned int	counter = 0;
	double	sum = 0;
	// for each image, we have to make sure the pixels are inside the
	// image
	if (x > 0) {
		DarkPixelType	darkneighbor = darkpixel(x - 1, y);
		if (darkneighbor == darkneighbor) {
			sum += image.pixel(x - 1, y);
			counter++;
		}
	}
	if (x < image.getSize().width() - 1) {
		DarkPixelType	darkneighbor = darkpixel(x + 1, y);
		if (darkneighbor == darkneighbor) {
			sum += image.pixel(x + 1, y);
			counter++;
		}
	}
	if (y > 0) {
		DarkPixelType	darkneighbor = darkpixel(x, y - 1);
		if (darkneighbor == darkneighbor) {
			sum += image.pixel(x, y - 1);
			counter++;
		}
	}
	if (y < image.getSize().height() - 1) {
		DarkPixelType	darkneighbor = darkpixel(x, y + 1);
		if (darkneighbor == darkneighbor) {
			sum += image.pixel(x, y + 1);
			counter++;
		}
	}
	Pixel	result = sum / counter;
	image.pixel(x, y) = result;
}

//////////////////////////////////////////////////////////////////////
// Mosaic interpolator
//////////////////////////////////////////////////////////////////////
template<typename DarkPixelType, typename Pixel>
class MosaicInterpolator : public TypedInterpolator<DarkPixelType, Pixel> {
protected:
	virtual DarkPixelType	darkpixel(unsigned int x, unsigned int y) const;
	virtual void	interpolatePixel(unsigned int x, unsigned int y,
				ImageAdapter<Pixel>& image);
	virtual void	interpolateGreen(unsigned int x, unsigned int y,
				ImageAdapter<Pixel>& image);
	virtual void	interpolateRedBlue(unsigned int x, unsigned int y,
				ImageAdapter<Pixel>& image);
	MosaicType	mosaic;
public:
	MosaicInterpolator(const Image<DarkPixelType>& _dark);
	void	setMosaic(const MosaicType& _mosaic) { mosaic = _mosaic; }
};

template<typename DarkPixelType, typename Pixel>
DarkPixelType	MosaicInterpolator<DarkPixelType, Pixel>::darkpixel(
	unsigned int x, unsigned int y) const {
	return TypedInterpolator<DarkPixelType, Pixel>::darkpixel(x, y);
}

template<typename DarkPixelType, typename Pixel>
MosaicInterpolator<DarkPixelType, Pixel>::MosaicInterpolator(
	const Image<DarkPixelType>& _dark)
	: TypedInterpolator<DarkPixelType, Pixel>(_dark) {
}

/**
 * \brief Interpolation function
 *
 * Interpolation is actually two quite different cases: monochrome and
 * Bayer matrix. Mosaic interpolation interpolates the four nearest
 * neighbor points. Bayer Matrix interpolation depends on the color.
 * For green pixel, the four diagonal neighbors are interpolated. For
 * red and blue, the for nearest neighbours of the same color are interpolated.
 *
 */
template<typename DarkPixelType, typename Pixel>
void	MosaicInterpolator<DarkPixelType, Pixel>::interpolateGreen(
	unsigned int x, unsigned int y, ImageAdapter<Pixel>& image) {
	unsigned int	counter = 0;
	double	sum = 0;

	// for each image, we have to make sure the pixels are inside the
	// image
	if ((x > 0) && (y > 0)) {
		DarkPixelType	darkneighbor = darkpixel(x - 1, y - 1);
		if (darkneighbor == darkneighbor) {
			sum += image.pixel(x - 1, y - 1);
			counter++;
		}
	}
	if ((x > 0) && (y < image.getSize().height() - 1)) {
		DarkPixelType	darkneighbor = darkpixel(x - 1, y + 1);
		if (darkneighbor == darkneighbor) {
			sum += image.pixel(x - 1, y + 1);
			counter++;
		}
	}
	if ((x < image.getSize().width() - 1) && (y > 0)) {
		DarkPixelType	darkneighbor = darkpixel(x + 1, y - 1);
		if (darkneighbor == darkneighbor) {
			sum += image.pixel(x + 1, y - 1);
			counter++;
		}
	}
	if ((x < image.getSize().width() - 1)
		&& (y < image.getSize().height() - 1)) {
		DarkPixelType	darkneighbor = darkpixel(x + 1, y + 1);
		if (darkneighbor == darkneighbor) {
			sum += image.pixel(x + 1, y + 1);
			counter++;
		}
	}
	Pixel	result = sum / counter;
	image.pixel(x, y) = result;
}

template<typename DarkPixelType, typename Pixel>
void	MosaicInterpolator<DarkPixelType, Pixel>::interpolateRedBlue(
	unsigned int x, unsigned int y, ImageAdapter<Pixel>& image) {
	unsigned int	counter = 0;
	double	sum = 0;
	// for each image, we have to make sure the pixels are inside the
	// image
	if (x > 1) {
		DarkPixelType	darkneighbor = darkpixel(x - 2, y);
		if (darkneighbor == darkneighbor) {
			sum += image.pixel(x - 2, y);
			counter++;
		}
	}
	if (x < image.getSize().width() - 2) {
		DarkPixelType	darkneighbor = darkpixel(x + 2, y);
		if (darkneighbor == darkneighbor) {
			sum += image.pixel(x + 2, y);
			counter++;
		}
	}
	if (y > 1) {
		DarkPixelType	darkneighbor = darkpixel(x, y - 2);
		if (darkneighbor == darkneighbor) {
			sum += image.pixel(x, y - 2);
			counter++;
		}
	}
	if (y < image.getSize().height() - 2) {
		DarkPixelType	darkneighbor = darkpixel(x, y + 2);
		if (darkneighbor == darkneighbor) {
			sum += image.pixel(x, y + 2);
			counter++;
		}
	}
	Pixel	result = sum / counter;
	image.pixel(x, y) = result;
}

template<typename DarkPixelType, typename Pixel>
void	MosaicInterpolator<DarkPixelType, Pixel>::interpolatePixel(
	unsigned int x, unsigned int y, ImageAdapter<Pixel>& image) {
	if (mosaic.isG(x, y)) {
		interpolateGreen(x, y, image);
	} else {
		interpolateRedBlue(x, y, image);
	}
}


//////////////////////////////////////////////////////////////////////
// Interpolator implementation
//////////////////////////////////////////////////////////////////////
Interpolator::Interpolator(const ImagePtr& _dark) : dark(_dark) {
	floatdark = dynamic_cast<Image<float> *>(&*dark);
	doubledark = dynamic_cast<Image<double> *>(&*dark);
	if ((NULL == floatdark) && (NULL == doubledark)) {
		throw std::runtime_error("only float or double images are "
			"suitable as darks");
	}
}

#define interpolate_mono(darkpixeltype, pixel, darkp, image)		\
{									\
	Image<pixel>	*imagep						\
		= dynamic_cast<Image<pixel > *>(&*image);		\
	if (NULL != imagep) {						\
		MonochromeInterpolator<darkpixeltype, pixel>	tint(*darkp);	\
		tint.interpolate(*imagep);				\
		return;							\
	}								\
}

void	Interpolator::interpolateMonochrome(ImagePtr& image) {
	if (floatdark) {
		interpolate_mono(float, unsigned char, floatdark, image);
		interpolate_mono(float, unsigned short, floatdark, image);
		interpolate_mono(float, unsigned int, floatdark, image);
		interpolate_mono(float, unsigned long, floatdark, image);
		interpolate_mono(float, float, floatdark, image);
		interpolate_mono(float, double, floatdark, image);
	}
	if (doubledark) {
		interpolate_mono(double, unsigned char, floatdark, image);
		interpolate_mono(double, unsigned short, floatdark, image);
		interpolate_mono(double, unsigned int, floatdark, image);
		interpolate_mono(double, unsigned long, floatdark, image);
		interpolate_mono(double, float, floatdark, image);
		interpolate_mono(double, double, floatdark, image);
	}
	throw std::runtime_error("cannot interpolate this image type");
}

#define interpolate_mosaic(darkpixeltype, pixel, darkp, image)		\
{									\
	Image<pixel>	*imagep						\
		= dynamic_cast<Image<pixel > *>(&*image);		\
	if (NULL != imagep) {						\
		MosaicInterpolator<darkpixeltype, pixel>	tint(*darkp);	\
		tint.setMosaic(image->getMosaicType());			\
		tint.interpolate(*imagep);				\
		return;							\
	}								\
}

void	Interpolator::operator()(ImagePtr& image) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Mosaic interpolation");
	if (floatdark) {
		interpolate_mosaic(float, unsigned char, floatdark, image);
		interpolate_mosaic(float, unsigned short, floatdark, image);
		interpolate_mosaic(float, unsigned int, floatdark, image);
		interpolate_mosaic(float, unsigned long, floatdark, image);
		interpolate_mosaic(float, float, floatdark, image);
		interpolate_mosaic(float, double, floatdark, image);
	}
	if (doubledark) {
		interpolate_mosaic(double, unsigned char, floatdark, image);
		interpolate_mosaic(double, unsigned short, floatdark, image);
		interpolate_mosaic(double, unsigned int, floatdark, image);
		interpolate_mosaic(double, unsigned long, floatdark, image);
		interpolate_mosaic(double, float, floatdark, image);
		interpolate_mosaic(double, double, floatdark, image);
	}
	return;
}

} // interpolation
} // astro
