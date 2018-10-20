/*
 * Image2Pixmap.cpp -- implementation of image to pixmap conversion
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "Image2Pixmap.h"
#include <AstroDebug.h>
#include <AstroAdapter.h>
#include <AstroDemosaicAdapter.h>
#include <AstroUtils.h>
#include <QPainter>

using namespace astro::image;
using namespace astro::adapter;
using namespace astro::adapter::demosaic;

namespace snowgui {

Image2Pixmap::Image2Pixmap() {
	_gain = 1;
	_brightness = 0;
	_scale = 0;
	_histogram = NULL;
	for (int i = 0; i < 3; i++) {
		_colorscales[i] = 1.;
		_coloroffsets[i] = 0.;
	}
	_crosshairs = false;
	_vertical_flip = false;
	_horizontal_flip = false;
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

/**
 * \brief Compute the scaled image size
 */
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
	double	_colorscales[3];
	double	_coloroffsets[3];
	void	initcolor() {
		for (int i = 0; i < 3; i++) {
			_colorscales[i] = 1.;
			_coloroffsets[i] = 0.;
		}
	}
public:
	GainSettings() : _gain(1), _brightness(0), _scale(0) {
		initcolor();
	}
	GainSettings(int scale) : _gain(1), _brightness(0), _scale(scale) {
		initcolor();
	}
	GainSettings(double gain, double brightness, int scale)
		: _gain(gain), _brightness(brightness), _scale(scale) {
		initcolor();
	}
	GainSettings(const GainSettings& other) {
		_gain = other._gain;
		_brightness = other._brightness;
		_scale = other._scale;
		for (int i = 0; i < 3; i++) {
			_colorscales[i] = other._colorscales[i];
			_coloroffsets[i] = other._coloroffsets[i];
		}
	}
	void	gain(double g) {
		_gain = g;
	}
	void	brightness(double b) {
		_brightness = b;
	}
	void	setColorScales(double r, double b, double g) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"setting scales to %.2f, %.2f, %.2f", r, g, b);
		_colorscales[0] = r;
		_colorscales[1] = g;
		_colorscales[2] = b;
	}
	void	setColorOffsets(double r, double b, double g) {
		_coloroffsets[0] = r;
		_coloroffsets[1] = g;
		_coloroffsets[2] = b;
	}
	void	setColorScales(double *colorscales) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"setting scales to %.2f, %.2f, %.2f",
			colorscales[0], colorscales[1], colorscales[2]);
		for (int i = 0; i < 3; i++) {
			_colorscales[i] = colorscales[i];
		}
	}
	void	setColorOffsets(double *coloroffsets) {
		for (int i = 0; i < 3; i++) {
			_coloroffsets[i] = coloroffsets[i];
		}
	}
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
			return 255;
		}
		if (value < 0) {
			return 0;
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
ImageRectangle	Image2Pixmap::rectangle(ImagePtr image) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rectangle for image %s",
		image->size().toString().c_str());
	if (_rectangle.isEmpty()) {
		return ImageRectangle(image->size());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rectangle found: %s",
		_rectangle.toString().c_str());
	return _rectangle;
}

/**
 * \brief Compute the rectangle to be used for the image
 *
 * This template function does the same thing as the rectangle(ImagePtr)
 * method but with explicitly known type.
 */
template<typename Pixel>
ImageRectangle	Image2Pixmap::rectangle(const ConstImageAdapter<Pixel>& image) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rectangle for image %s",
		image.getSize().toString().c_str());
	if (_rectangle.isEmpty()) {
		return ImageRectangle(image.getSize());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rectangle found: %s",
		_rectangle.toString().c_str());
	return _rectangle;
}

#define convert_mono(imageptr, Pixel)					\
{									\
	Image<Pixel>	*image = dynamic_cast<Image<Pixel> *>(&*imageptr);\
	if (NULL != image) {						\
		return convertMono<Pixel>(*image);			\
	}								\
}

/**
 * \brief Monochrome image conversion
 *
 * This method converts a monochrome image to a QImage. It can work with
 * mono images of arbitrary pixel types. The values of gain and brightness
 * must be set to reasonable values or most pixel values may lie outside
 * the displayable range.
 */
QImage	*Image2Pixmap::convertMono(ImagePtr imageptr) {
	convert_mono(imageptr, unsigned char);
	convert_mono(imageptr, unsigned short);
	convert_mono(imageptr, unsigned int);
	convert_mono(imageptr, unsigned long);
	convert_mono(imageptr, float);
	convert_mono(imageptr, double);
	return NULL;
}

/**
 * \brief Monochrome image Conversion, typed
 *
 * This template function converts an monochrome image presented in the
 * form of a monochrome image adapter.
 */
template<typename Pixel>
QImage	*Image2Pixmap::convertMono(const ConstImageAdapter<Pixel>& image) {
	ImageSize	size = image.getSize();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "converting Image<%s> of size %s",
		astro::demangle(typeid(Pixel).name()).c_str(),
		size.toString().c_str());
	Histogram<double>	*histo = new Histogram<double>(256);
	histo->logarithmic(_logarithmic);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "_histogram = %p", _histogram);
	if (_histogram) {
		delete _histogram;
		_histogram = NULL;
	}
	_histogram = histo;

	// create the flip adapter
	debug(LOG_DEBUG, DEBUG_LOG, 0, "vertical flip: %s, horizontal flip: %s",
		(vertical_flip()) ? "yes" : "no",
		(horizontal_flip()) ? "yes" : "no");
	FlipAdapter<Pixel>	flip(image, vertical_flip(), horizontal_flip());

	// compute the rectangle 
	ImageRectangle	r = rectangle(image);

	// create a windowadapter
	WindowAdapter<Pixel>	windowadapter(flip, r);

	// create a gain adapter
	GainAdapter<Pixel>	gainadapter(windowadapter, _scale);

	gainadapter.gain(_gain);
	gainadapter.brightness(_brightness);

	// dimensions
	int	w = gainadapter.getSize().width();
	int	h = gainadapter.getSize().height();

	// prepare the result
	debug(LOG_DEBUG, DEBUG_LOG, 0, "preparing QImage(%d,%d)", w, h);
	QImage	*qimage = new QImage(w, h, QImage::Format_RGB32);

	// fill the image into the result
#pragma omp parallel for
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			unsigned char	v = gainadapter.pixel(x, y);
			histo->add((double)v);
			unsigned long	value = convert(v);
			qimage->setPixel(x, h - 1 - y, value);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image data set");

	return qimage;
}

/**
 * \brief Base class for color gain adapters
 *
 * By mixing in the GainSettings, this class adds gain information to the
 * basic image adapter
 */
class BasicGainRGBAdapter : public ConstImageAdapter<RGB<unsigned char> >,
			    public GainSettings {
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
	unsigned char	rescale(double i) const {
		double	v = trunc(i * _gain + _brightness);
		if (v > 255) {
			return 255;
		}
		if (v < 0) {
			return 0;
		}
		return (unsigned char)v;
	}
	unsigned char	colorRescale(int i, double c) const {
		return rescale(_colorscales[i] * c + _coloroffsets[i]);
	}
	RGB<unsigned char>	rescale(const RGB<Pixel> i) const {
		return RGB<unsigned char>( colorRescale(0, i.R),
					   colorRescale(1, i.G),
					   colorRescale(2, i.B));
	}
	RGB<unsigned char>	normalPixel(int x, int y) const {
if ((x == 0) && (y == 0)) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "scales: %.2f, %.2f, %.2f",
		_colorscales[0], _colorscales[1], _colorscales[2]);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "offsets: %.2f, %.2f, %.2f",
		_coloroffsets[0], _coloroffsets[1], _coloroffsets[2]);
}
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
		double	r = 0, g = 0, b = 0;
		int	counter = 0;
		for (int x = startx; x < endx; x++) {
			for (int y = starty; y < endy; y++) {
				RGB<Pixel>	t = _image.pixel(x,y);
				r += t.R;
				g += t.G;
				b += t.B;
				counter++;
			}
		}
		return RGB<unsigned char>( colorRescale(0, r / counter),
					   colorRescale(1, g / counter),
					   colorRescale(2, b / counter));
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
};

/**
 * \brief Convert a RGB astro::image::ImagePtr to a QImage
 *
 * Because this adapter works with a RGB image adapter, it can be
 * be used on RGB images or on DemosaicAdapter<Pixel> without change.
 */
template<typename Pixel>
QImage	*Image2Pixmap::convertRGB(const ConstImageAdapter<RGB<Pixel> >& image) {
	ImageSize	size = image.getSize();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "converting RGB<%s> image of size %s",
		astro::demangle(typeid(Pixel).name()).c_str(),
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

	WindowAdapter<RGB<Pixel> >	windowadapter(image, r);
	GainRGBAdapter<Pixel>	gainadapter(windowadapter, _scale);
	gainadapter.gain(_gain);
	gainadapter.brightness(_brightness);
	gainadapter.setColorScales(_colorscales);
	gainadapter.setColorOffsets(_coloroffsets);

	// dimensions
	int	w = gainadapter.getSize().width();
	int	h = gainadapter.getSize().height();

	// prepare result structuren
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create QImage(%d, %d)", w, h);
	QImage	*qimage = new QImage(w, h, QImage::Format_RGB32);

#pragma omp parallel for
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			RGB<unsigned char>	p = gainadapter.pixel(x, y);
			histo->add(RGB<double>(p));
			unsigned long	value = convert(p);
			qimage->setPixel(x, h - 1 - y, value);
		}
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "QImage complete");
	return qimage;
}

#define convert_mosaic(imageptr, Pixel)					\
{									\
	Image<Pixel>	*image = dynamic_cast<Image<Pixel>*>(&*imageptr);\
	if (NULL != image) {						\
		DemosaicAdapter<Pixel>	demosaicer(*image, _mosaic);	\
		return convertRGB(demosaicer);				\
	}								\
}

/**
 * \brief Convert and debayer an image at the same time
 *
 * This method selectes the convertMosaic template function with
 * the correct pixel type template argument.
 */
QImage	*Image2Pixmap::convertMosaic(ImagePtr imageptr) {
	convert_mosaic(imageptr, unsigned char)
	convert_mosaic(imageptr, unsigned short)
	convert_mosaic(imageptr, unsigned int)
	convert_mosaic(imageptr, unsigned long)
	convert_mosaic(imageptr, float)
	convert_mosaic(imageptr, double)
	return NULL;
}

#define convert_rgb(imageptr, Pixel)					\
{									\
	Image<RGB<Pixel> >	*image = dynamic_cast<Image<RGB<Pixel> >*>(&*imageptr);\
	if (NULL != image) {						\
		return convertRGB(*image);				\
	}								\
}

/**
 * \brief Convert an RGB image into a Pixmap
 *
 * This method selectes the convertMosaic template function with
 * the correct pixel type template argument.
 */
QImage	*Image2Pixmap::convertRGB(ImagePtr imageptr) {
	convert_rgb(imageptr, unsigned char)
	convert_rgb(imageptr, unsigned short)
	convert_rgb(imageptr, unsigned int)
	convert_rgb(imageptr, unsigned long)
	convert_rgb(imageptr, float)
	convert_rgb(imageptr, double)
	return NULL;
}

/**
 * \brief Convert an image
 *
 * This method distinguished between monochrome and color images and
 * calls the approppriate methods
 */
QPixmap	*Image2Pixmap::operator()(ImagePtr image) {
	// find the image size and allocate a buffer of appropriate size
	ImageSize	size = image->size();
	QImage	*qimage = NULL;
	switch (image->planes()) {
	case 3:
		qimage = convertRGB(image);
		break;
	case 1:
		if (_mosaic) {
			qimage = convertMosaic(image);
		} else {
			qimage = convertMono(image);
		}
		break;
	}
	QPixmap	*result = new QPixmap(size.width(), size.height());
	result->convertFromImage(*qimage);

	// draw the crosshairs if necessary
	if (_crosshairs) {
		drawCrosshairs(result);
	}

	return result;;
}

/**
 * \brief Draw the crosshairs to a QPixmap
 */
void	Image2Pixmap::drawCrosshairs(QPixmap *pixmap) {
	QPainter	painter(pixmap);
	QPen	pen(Qt::SolidLine);
	pen.setColor(Qt::red);
	painter.setPen(pen);
	int	w = pixmap->size().width();
	int	h = pixmap->size().height();
	int	y = h - _crosshairs_center.y();
	int	x = _crosshairs_center.x();
        painter.drawLine(QPoint(0, y), QPoint(x - 5, y));
	painter.drawLine(QPoint(x + 5, y), QPoint(w - 1, y));
	painter.drawLine(QPoint(x, 0), QPoint(x, y - 5));
	painter.drawLine(QPoint(x, y + 5), QPoint(x, h - 1));
}

/**
 * \brief Convert the histogram data into a Pixmap
 */
QPixmap	*Image2Pixmap::histogram(int width, int height) {
	if (_histogram) {
		return _histogram->pixmap(width, height);
	}
	return NULL;
}

void	Image2Pixmap::setColorScales(double r, double g, double b) {
	_colorscales[0] = r;
	_colorscales[1] = g;
	_colorscales[2] = b;
}

void	Image2Pixmap::setColorOffsets(double r, double g, double b) {
	_coloroffsets[0] = r;
	_coloroffsets[1] = g;
	_coloroffsets[2] = b;
}

void	Image2Pixmap::setColorScale(int i, double c) {
	_colorscales[i] = c;
}

void	Image2Pixmap::setColorOffset(int i, double c) {
	_coloroffsets[i] = c;
}

} // namespace snowgui
