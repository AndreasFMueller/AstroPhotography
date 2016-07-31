/*
 * Image2Pixmap.cpp -- implementation of image to pixmap conversion
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "Image2Pixmap.h"
#include <AstroDebug.h>
#include <AstroAdapter.h>

using namespace astro::image;
using namespace astro::adapter;

namespace snowgui {

Image2Pixmap::Image2Pixmap() {
	_gain = 1;
	_brightness = 0;
	_scale = 0;
	_histogram = NULL;
}

Image2Pixmap::~Image2Pixmap() {
	if (_histogram != NULL) {
		delete _histogram;
	}
}

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

static ImageSize	scaledSize(int scale, const ImageSize& origsize) {
	if (scale > 0) {
		return ImageSize(origsize.width() << scale,
				origsize.height() << scale);
	}
	if (scale < 0) {
		return ImageSize(origsize.width() >> -scale,
			origsize.height() >> -scale);
	}
	return origsize;
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
	int	_scale;
public:
	GainSettings() : _gain(1), _brightness(0), _scale(0) { }
	GainSettings(int scale) : _gain(1), _brightness(0), _scale(scale) { }
	GainSettings(double gain, double brightness, int scale)
		: _gain(gain), _brightness(brightness), _scale(scale) { }
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
	BasicGainAdapter(const ImageSize& imagesize, int scale)
		: ConstImageAdapter<unsigned char>(imagesize),
		  GainSettings(scale) {
	}
	BasicGainAdapter(const ImageSize& imagesize,
		double gain, double brightness, int scale)
		: ConstImageAdapter<unsigned char>(imagesize),
		  GainSettings(gain, brightness, scale) {
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
	unsigned char	rescale(double value) const {
		if (value > 255) {
			value = 255;
		}
		if (value < 0) {
			value = 0;
		}
		unsigned char	result = value;
		return result;
	}
	unsigned char	normalPixel(int x, int y) const {
		double	value = _image.pixel(x, y) * _gain + _brightness;
		return rescale(value);
	}
	unsigned char	downscalePixel(int x, int y) const {
		int	startx = x << -_scale;
		int	starty = y << -_scale;
		int	endx = startx + (1 << -_scale);
		int	endy = starty + (1 << -_scale);
		double	s = 0;
		int	counter = 0;
		for (int x = startx; x < endx; x++) {
			for (int y = starty; y < endy; y++) {
				s += _image.pixel(x, y) * _gain + _brightness;
				counter++;
			}
		}
		return rescale(s / counter);
	}
	unsigned char	upscalePixel(int x, int y) const {
		return normalPixel(x >> _scale, y >> _scale);
	}
public:
	GainAdapter(const ConstImageAdapter<Pixel>& image, int scale = 0)
		: BasicGainAdapter(scaledSize(scale, image.getSize()), scale),
		  _image(image) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s adapter Pixel type %s, "
			"scale = %d",
			getSize().toString().c_str(),
			astro::demangle(typeid(Pixel).name()).c_str(), _scale);
	}
	GainAdapter(const ConstImageAdapter<Pixel>& image, double gain,
		double brightness, int scale = 0)
		: BasicGainAdapter(scaledSize(scale, image.getSize()),
		  gain, brightness, scale), _image(image) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s adapter Pixel type %s, "
			"gain = %f, brightness = %f, scale = %d",
			getSize().toString().c_str(),
			astro::demangle(typeid(Pixel).name()).c_str(),
			_gain, _brightness, _scale);
	}
	virtual unsigned char	pixel(int x, int y) const {
		if (_scale > 0) {
			return upscalePixel(x, y);
		}
		if (_scale < 0) {
			return downscalePixel(x, y);
		}
		return normalPixel(x, y);
	}
};

/**
 * \brief Compute the rectangle to be used for the image
 */
ImageRectangle	Image2Pixmap::rectangle(ImagePtr image) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rectangle for image %s",
		image->size().toString().c_str());
	if (_rectangle.isEmpty()) {
		return ImageRectangle(image->size());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rectangle found: %s",
		_rectangle.toString().c_str());
	return _rectangle;
}

#define	ga(gainadapter, wa, Pixel, image, scale, rect)			\
	if (gainadapter == NULL) {					\
		Image<Pixel>	*img = dynamic_cast<Image<Pixel> *>(&*image);\
		if (NULL != img) {					\
			WindowAdapter<Pixel>	*windowadapter		\
				 = new WindowAdapter<Pixel>(*img, rect);\
			wa = BasicAdapterPtr(windowadapter);		\
			gainadapter = new GainAdapter<Pixel>(*windowadapter, scale);\
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
	Histogram<double>	*histo = new Histogram<double>(256);
	histo->logarithmic(_logarithmic);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "_histogram = %p", _histogram);
	if (_histogram) {
		delete _histogram;
		_histogram = NULL;
	}
	_histogram = histo;

	// compute the rectangle 
	ImageRectangle	r = rectangle(image);

	// get a gain adaapter
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"looking for gain adapter, scale = %d, rectangle = %s",
		_scale, r.toString().c_str());
	BasicGainAdapter	*gainadapter = NULL;
	BasicAdapterPtr		ba;
	ga(gainadapter, ba, unsigned char, image, _scale, r);
	ga(gainadapter, ba, unsigned short, image, _scale, r);
	ga(gainadapter, ba, unsigned long, image, _scale, r);
	ga(gainadapter, ba, float, image, _scale, r);
	ga(gainadapter, ba, double, image, _scale, r);
	if (NULL == gainadapter) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no suitable adapter found");
		return NULL;
	}
	gainadapter->gain(_gain);
	gainadapter->brightness(_brightness);

	// dimensions
	int	w = gainadapter->getSize().width();
	int	h = gainadapter->getSize().height();

	// prepare the result
	debug(LOG_DEBUG, DEBUG_LOG, 0, "preparing QImage(%d,%d)", w, h);
	QImage	*qimage = new QImage(w, h, QImage::Format_RGB32);

	// fill the image into the result
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			unsigned char	v = gainadapter->pixel(x, y);
			histo->add((double)v);
			unsigned long	value = convert(v);
			qimage->setPixel(x, h - 1 - y, value);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image data set");

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
	BasicGainRGBAdapter(const ImageSize& imagesize, int scale)
		: ConstImageAdapter<RGB<unsigned char> >(imagesize),
		  GainSettings(scale) {
	}
	BasicGainRGBAdapter(const ImageSize& imagesize,
		double gain, double brightness, int scale)
		: ConstImageAdapter<RGB<unsigned char> >(imagesize),
		  GainSettings(gain, brightness, scale) {
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
	RGB<unsigned char>	normalPixel(int x, int y) const {
		return rescale(_image.pixel(x, y));
	}
	RGB<unsigned char>	upscalePixel(int x, int y) const {
		return normalPixel(x >> _scale, y >> _scale);
	}
	RGB<unsigned char>	downscalePixel(int x, int y) const {
		int	startx = x << -_scale;
		int	starty = y << -_scale;
		int	endx = startx + (1 << -_scale);
		int	endy = starty + (1 << -_scale);
		RGB<double>	s;
		int	counter = 0;
		for (int x = startx; x < endx; x++) {
			for (int y = starty; y < endy; y++) {
				s = s + _image.pixel(x, y);
				counter++;
			}
		}
		s = s * (1. / counter);
		RGB<Pixel>	p(s);
		return rescale(p);
	}
public:
	GainRGBAdapter(const ConstImageAdapter<RGB<Pixel> >& image, int scale)
		: BasicGainRGBAdapter(scaledSize(scale, image.getSize()),
		  scale), _image(image) {
	}
	GainRGBAdapter(const ConstImageAdapter<RGB<Pixel> >& image, double gain,
		double brightness, int scale)
		: BasicGainRGBAdapter(scaledSize(scale, image.getSize()),
		  gain, brightness, scale), _image(image) {
	}
	virtual RGB<unsigned char>	pixel(int x, int y) const {
		if (_scale > 0) {
			return upscalePixel(x, y);
		}
		if (_scale < 0) {
			return downscalePixel(x, y);
		}
		return normalPixel(x, y);
	}
	void	gain(double g) { _gain = g; }
	void	brightness(double b) { _brightness = b; }
};

#define	gargb(gainadapter, wa, Pixel, image, scale, rect)		\
	if (gainadapter == NULL) {					\
		Image<RGB<Pixel> >	*img				\
			= dynamic_cast<Image<RGB<Pixel> > *>(&*image);	\
		if (NULL != img) {					\
			WindowAdapter<RGB<Pixel> >	*windowadapter	\
				 = new WindowAdapter<RGB<Pixel> >(*img, rect);\
			wa = BasicAdapterPtr(windowadapter);		\
			gainadapter = new GainRGBAdapter<Pixel>(*windowadapter, scale);\
		}							\
	}

/**
 * \brief Convert a RGB astro::image::ImagePtr to a QImage
 */
QImage	*Image2Pixmap::convertRGB(ImagePtr image) {
	ImageSize	size = image->size();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "converting RGB image of size %s",
		size.toString().c_str());
	Histogram<RGB<double> >	*histo = new Histogram<RGB<double> >(256);
	histo->logarithmic(_logarithmic);
	if (_histogram) {
		delete _histogram;
		_histogram = NULL;
	}
	_histogram = histo;
	
	// compute the rectangle 
	ImageRectangle	r = rectangle(image);

	BasicGainRGBAdapter	*gainadapter = NULL;
	BasicAdapterPtr		ba;
	gargb(gainadapter, ba, unsigned char, image, _scale, r);
	gargb(gainadapter, ba, unsigned short, image, _scale, r);
	gargb(gainadapter, ba, unsigned long, image, _scale, r);
	gargb(gainadapter, ba, double, image, _scale, r);
	gargb(gainadapter, ba, float, image, _scale, r);
	if (NULL == gainadapter) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no gain adapter found");
		return NULL;
	}
	gainadapter->gain(_gain);
	gainadapter->brightness(_brightness);

	// dimensions
	int	w = gainadapter->getSize().width();
	int	h = gainadapter->getSize().height();

	// prepare result structuren
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create QImage(%d, %d)", w, h);
	QImage	*qimage = new QImage(w, h, QImage::Format_RGB32);

	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			RGB<unsigned char>	p = gainadapter->pixel(x, y);
			histo->add(RGB<double>(p));
			unsigned long	value = convert(p);
			qimage->setPixel(x, h - 1 - y, value);
		}
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "QImage complete");
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

QPixmap	*Image2Pixmap::histogram(int width, int height) {
	if (_histogram) {
		return _histogram->pixmap(width, height);
	}
	return NULL;
}

} // namespace snowgui
