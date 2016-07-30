/*
 * Image2Pixmap.cpp -- implementation of image to pixmap conversion
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "Image2Pixmap.h"
#include <AstroDebug.h>

using namespace astro::image;

namespace snowgui {

static unsigned long	convert(unsigned char v) {
	return 0xff000000 | (v << 16) | (v << 8) | v;
}

static unsigned long	convert(const RGB<unsigned char>& v) {
	return 0xff000000 | (v.R << 16) | (v.G << 8) | v.B;
}

template<typename Pixel>
class GainAdapter : public ConstImageAdapter<unsigned char> {
	const ConstImageAdapter<Pixel>&	_image;
	double	_gain;
	double	_brightness;
public:
	GainAdapter(const ConstImageAdapter<Pixel>& image)
		: ConstImageAdapter<unsigned char>(image.getSize()),
		  _image(image), _gain(1./64), _brightness(0) {
	}
	GainAdapter(const ConstImageAdapter<Pixel>& image, double gain,
		double brightness)
		: ConstImageAdapter<unsigned char>(image.getSize()),
		  _image(image), _gain(gain), _brightness(brightness) {
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
	void	gain(double g) { _gain = g; }
	void	brightness(double b) { _brightness = b; }
};

#define	ga(gainadapter, Pixel, image)					\
	if (gainadapter == NULL) {					\
		Image<Pixel>	*img = dynamic_cast<Image<Pixel> *>(&*image);\
		if (NULL != img) {					\
			gainadapter = new GainAdapter<Pixel>(*img);	\
		}							\
	}

QImage	*Image2Pixmap::convertMono(ImagePtr image) {
	ImageSize	size = image->size();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "converting Mono image of size %s",
		size.toString().c_str());

	// get a gain adaapter
	ConstImageAdapter<unsigned char>	*gainadapter = NULL;
	ga(gainadapter, unsigned char, image);
	ga(gainadapter, unsigned short, image);
	ga(gainadapter, unsigned long, image);
	ga(gainadapter, float, image);
	ga(gainadapter, double, image);
	if (NULL == gainadapter) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no suitable adapter found");
		return NULL;
	}

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

template<typename Pixel>
class GainRGBAdapter : public ConstImageAdapter<RGB<unsigned char> > {
	const ConstImageAdapter<RGB<Pixel> >&	_image;
	double	_gain;
	double	_brightness;
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
		return RGB<unsigned char>(rescale(i.R), rescale(i.G), rescale(i.B));
	}
public:
	GainRGBAdapter(const ConstImageAdapter<RGB<Pixel> >& image)
		: ConstImageAdapter<RGB<unsigned char> >(image.getSize()),
		  _image(image), _gain(1), _brightness(0) {
	}
	GainRGBAdapter(const ConstImageAdapter<RGB<Pixel> >& image, double gain,
		double brightness)
		: ConstImageAdapter<RGB<unsigned char> >(image.getSize()),
		  _image(image), _gain(gain), _brightness(brightness) {
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


QImage	*Image2Pixmap::convertRGB(ImagePtr image) {
	ImageSize	size = image->size();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "converting RGB image of size %s",
		size.toString().c_str());
	
	ConstImageAdapter<RGB<unsigned char> >	*gainadapter = NULL;
	gargb(gainadapter, unsigned char, image);
	gargb(gainadapter, unsigned short, image);
	gargb(gainadapter, unsigned long, image);
	gargb(gainadapter, double, image);
	gargb(gainadapter, float, image);
	if (NULL == gainadapter) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no gain adapter found");
		return NULL;
	}

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
