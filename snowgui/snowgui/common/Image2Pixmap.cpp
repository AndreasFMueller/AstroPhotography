/*
 * Image2Pixmap.cpp -- implementation of image to pixmap conversion
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "Image2Pixmap.h"
#include <AstroDebug.h>

using namespace astro::image;

namespace snowgui {

/**
 * \brief Convert an unsigned char value to a monochrome pixel
 */
static unsigned long	convert(unsigned char v) {
	return 0xff000000 | (v << 16) | (v << 8) | v;
}

/*
 * \brief Convert an RGB<unsigned char> pixel to a Qt RGB Pixel
 */
static unsigned long	convert(const RGB<unsigned char>& v) {
	return 0xff000000 | (v.R << 16) | (v.G << 8) | v.B;
}

/**
 * \brief class to handle the gain/brightness settings
 *
 * This is a mixin class for the Basic gain adapters.
 */
class GainSettings {
protected:
	double	_gain;
	double	_brightness;
public:
	GainSettings() : _gain(1), _brightness(0) { }
	GainSettings(double gain, double brightness)
		: _gain(gain), _brightness(brightness) { }
	void	gain(double g) { _gain = g; }
	void	brightness(double b) { _brightness = b; }
};

/**
 * \brief Base class for the gain adapters
 *
 * The base class adds the gain and brightness settings to the image
 * adapter
 */
class BasicGainAdapter : public ConstImageAdapter<unsigned char>,
			public GainSettings {
public:
	BasicGainAdapter(const ImageSize& imagesize)
		: ConstImageAdapter<unsigned char>(imagesize) {
	}
	BasicGainAdapter(const ImageSize& imagesize,
		double gain, double brightness)
		: ConstImageAdapter<unsigned char>(imagesize),
		  GainSettings(gain, brightness) {
	}
};

/**
 * \brief Gain adapter to convert a monochrome image
 *
 * This adapter expands pixel values according to the settings in the
 * gain and brightness attributes, and limits the values to 0-255.
 */
template<typename Pixel>
class GainAdapter : public BasicGainAdapter {
	const ConstImageAdapter<Pixel>&	_image;
public:
	GainAdapter(const ConstImageAdapter<Pixel>& image)
		: BasicGainAdapter(image.getSize()), _image(image) {
	}
	GainAdapter(const ConstImageAdapter<Pixel>& image, double gain,
		double brightness)
		: BasicGainAdapter(image.getSize(), gain, brightness),
		  _image(image) {
	}
	virtual unsigned char	pixel(int x, int y) const {
		double	value = _image.pixel(x, y) * _gain + _brightness;
		if (value > 255) {
			value = 255;
		}
		if (value < 0) {
			value = 0;
		}
		unsigned char	result = value;
		return result;
	}
};

#define	ga(gainadapter, Pixel, image)					\
	if (gainadapter == NULL) {					\
		Image<Pixel>	*img = dynamic_cast<Image<Pixel> *>(&*image);\
		if (NULL != img) {					\
			gainadapter = new GainAdapter<Pixel>(*img);	\
		}							\
	}

/**
 * \brief Monochrome image conversion
 *
 * This method converts a monochrome image to a QImage. It can work with
 * mono images of arbitrary pixel types. The values of gain and brightness
 * must be set to reasonable values or most pixel values may lie outside
 * the displayable range.
 */
QImage	*Image2Pixmap::convertMono(ImagePtr image) {
	ImageSize	size = image->size();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "converting Mono image of size %s",
		size.toString().c_str());

	// get a gain adaapter
	BasicGainAdapter	*gainadapter = NULL;
	ga(gainadapter, unsigned char, image);
	ga(gainadapter, unsigned short, image);
	ga(gainadapter, unsigned long, image);
	ga(gainadapter, float, image);
	ga(gainadapter, double, image);
	if (NULL == gainadapter) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no suitable adapter found");
		return NULL;
	}
	gainadapter->gain(_gain);
	gainadapter->brightness(_brightness);

	// prepare the result
	QImage	*qimage = new QImage(size.width(), size.height(),
				QImage::Format_RGB32);

	// fill the image into the result
	int	w = size.width();
	int	h = size.height();
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			unsigned long	value
				= convert(gainadapter->pixel(x, y));
			qimage->setPixel(x, h - 1 - y, value);
		}
	}

	// cleanup
	delete gainadapter;
	
	return qimage;
}

/**
 * \brief Base class for color gain adapters
 *
 * By mixing in the GainSettings, this class adds gain information to the
 * basic image adapter
 */
class BasicGainRGBAdapter : public ConstImageAdapter<RGB<unsigned char> >, public GainSettings {
public:
	BasicGainRGBAdapter(const ImageSize& imagesize)
		: ConstImageAdapter<RGB<unsigned char> >(imagesize) {
	}
	BasicGainRGBAdapter(const ImageSize& imagesize,
		double gain, double brightness)
		: ConstImageAdapter<RGB<unsigned char> >(imagesize),
		  GainSettings(gain, brightness) {
	}
};

/**
 * \brief Template class to adapt gain in color images
 */
template<typename Pixel>
class GainRGBAdapter : public BasicGainRGBAdapter {
	const ConstImageAdapter<RGB<Pixel> >&	_image;
	unsigned char	rescale(Pixel i) const {
		double	v = i * _gain + _brightness;
		if (v > 255) {
			v = 255;
		}
		if (v < 0) {
			v = 0;
		}
		return (unsigned char)v;
	}
	RGB<unsigned char>	rescale(const RGB<Pixel> i) const {
		return RGB<unsigned char>(rescale(i.R), rescale(i.G),
			rescale(i.B));
	}
public:
	GainRGBAdapter(const ConstImageAdapter<RGB<Pixel> >& image)
		: BasicGainRGBAdapter(image.getSize()), _image(image) {
	}
	GainRGBAdapter(const ConstImageAdapter<RGB<Pixel> >& image, double gain,
		double brightness)
		: BasicGainRGBAdapter(image.getSize(), gain, brightness),
		  _image(image) {
	}
	virtual RGB<unsigned char>	pixel(int x, int y) const {
		return rescale(_image.pixel(x, y));
	}
	void	gain(double g) { _gain = g; }
	void	brightness(double b) { _brightness = b; }
};

#define	gargb(gainadapter, Pixel, image)				\
	if (gainadapter == NULL) {					\
		Image<RGB<Pixel> >	*img				\
			= dynamic_cast<Image<RGB<Pixel> > *>(&*image);	\
		if (NULL != img) {					\
			gainadapter = new GainRGBAdapter<Pixel>(*img);	\
		}							\
	}


/**
 * \brief Convert a RGB astro::image::ImagePtr to a QImage
 */
QImage	*Image2Pixmap::convertRGB(ImagePtr image) {
	ImageSize	size = image->size();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "converting RGB image of size %s",
		size.toString().c_str());
	
	BasicGainRGBAdapter	*gainadapter = NULL;
	gargb(gainadapter, unsigned char, image);
	gargb(gainadapter, unsigned short, image);
	gargb(gainadapter, unsigned long, image);
	gargb(gainadapter, double, image);
	gargb(gainadapter, float, image);
	if (NULL == gainadapter) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no gain adapter found");
		return NULL;
	}
	gainadapter->gain(_gain);
	gainadapter->brightness(_brightness);

	// prepare result structuren
	QImage	*qimage = new QImage(size.width(), size.height(),
				QImage::Format_RGB32);

	int	w = size.width();
	int	h = size.height();
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			unsigned long	value
				= convert(gainadapter->pixel(x, y));
			qimage->setPixel(x, h - 1 - y, value);
		}
	}

	return qimage;
}

QPixmap	*Image2Pixmap::operator()(ImagePtr image) {
	// find the image size and allocate a buffer of appropriate size
	ImageSize	size = image->size();
	QImage	*qimage = NULL;
	switch (image->planes()) {
	case 3:
		qimage = convertRGB(image);
		break;
	case 1:
		qimage = convertMono(image);
		break;
	}
	QPixmap	*result = new QPixmap(size.width(), size.height());
	result->convertFromImage(*qimage);

	return result;;
}

} // namespace snowgui
