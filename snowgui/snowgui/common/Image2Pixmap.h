/*
 * Image2Pixmap.h -- convert an ImagePtr to a pixmap
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Image2Pixmap_h
#define _Image2Pixmap_h

#include <QPixmap>
#include <QImage>
#include <AstroImage.h>
#include <Histogram.h>

namespace snowgui {

#if 0
template<typename Pixel>
class MonoAdapter : public ConstImageAdapter<Pixel> {
	Image<Pixel>	*_image;
public:
	MonoAdapter(Image<Pixel> *image)
		: ConstImageAdapter<Pixel>(image->size()), _image(image) {
	}
	virtual Pixel	pixel(int x, int y) const {
		return _image->pixel(x, y);
	}
};

template<typename Pixel>
class ColorAdapter : public ConstImageAdapter<RGB<Pixel> > {
	Image<RGB<Pixel> >	*_image;
public:
	ColorAdapter(Image<RGB<Pixel> > *image)
		: ConstImageAdapter<RGB<Pixel> >(_image->size()),
		  _image(image) {
	}
	virtual RGB<Pixel>	pixel(int x, int y) const {
		return _image->pixel(x, y);
	}
};
#endif


class	Image2Pixmap {
	double	_brightness;
	double	_gain;
	bool	_logarithmic;
	int	_scale;
	astro::image::ImageRectangle	_rectangle;
	astro::image::MosaicType	_mosaic;
public:
	Image2Pixmap();
	~Image2Pixmap();

	double	brightness() const { return _brightness; }
	void	brightness(double b) { _brightness = b; }
	double	gain() const { return _gain; }
	void	gain(double g) { _gain = g; }
	bool	logarithmic() const { return _logarithmic; }
	void	logarithmic(bool l) { _logarithmic = l; }
	int	scale() const { return _scale; }
	void	scale(int s) { _scale = s; }
	const astro::image::ImageRectangle&	rectangle() const {
		return _rectangle;
	}
	void	rectangle(const astro::image::ImageRectangle& r) {
		_rectangle = r;
	}
	const astro::image::MosaicType&	mosaic() const { return _mosaic; }
	void	mosaic(const astro::image::MosaicType m) { _mosaic = m; }
private:
	HistogramBase	*_histogram;
	astro::image::ImageRectangle	rectangle(astro::image::ImagePtr image) const;
	template<typename Pixel>
	astro::image::ImageRectangle	rectangle(const ConstImageAdapter<Pixel>& image) const;

	// adapters 
	template<typename Pixel>
	QImage	*convertRGB(const ConstImageAdapter<RGB<Pixel> >& image);
	template<typename Pixel>
	QImage	*convertMono(const ConstImageAdapter<Pixel>& image);

	QImage	*convertRGB(astro::image::ImagePtr image);
	QImage	*convertMono(astro::image::ImagePtr image);
	QImage	*convertMosaic(astro::image::ImagePtr image);

public:
	QPixmap	*histogram(int width, int height);
	
	QPixmap	*operator()(astro::image::ImagePtr image);
};

} // namespace snowgui

#endif /* _Image2Pixmap_h */
