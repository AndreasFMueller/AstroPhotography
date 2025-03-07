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
	astro::image::ImageSize		_frame;
	astro::image::ImageRectangle	_rectangle;
	astro::image::MosaicType	_mosaic;
	bool	_crosshairs;
	astro::image::ImagePoint	_crosshairs_center;
	bool	_vertical_flip;
	bool	_horizontal_flip;
	bool	_show_red;
	bool	_show_green;
	bool	_show_blue;
	bool	_negative;
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
	bool	crosshairs() const { return _crosshairs; }
	void	crosshairs(bool c) { _crosshairs = c; }
	bool	vertical_flip() const { return _vertical_flip; }
	void	vertical_flip(bool f) { _vertical_flip = f; }
	bool	horizontal_flip() const { return _horizontal_flip; }
	void	horizontal_flip(bool f) { _horizontal_flip = f; }
	bool	show_red() const { return _show_red; }
	void	show_red(bool s) { _show_red = s; }
	bool	show_green() const { return _show_green; }
	void	show_green(bool s) { _show_green = s; }
	bool	show_blue() const { return _show_blue; }
	void	show_blue(bool s) { _show_blue = s; }
	bool	negative() const { return _negative; }
	void	negative(bool n) { _negative = n; }

	void	frame(const astro::image::ImageSize& f) { _frame = f; }
	const astro::image::ImageSize&	frame() const { return _frame; }

	astro::image::ImagePoint	crosshairs_center() const {
		return _crosshairs_center;
	}
	void	crosshairs_center(const astro::image::ImagePoint& c) {
		_crosshairs_center = c;
	}
	const astro::image::ImageRectangle&	rectangle() const {
		return _rectangle;
	}
	void	rectangle(const astro::image::ImageRectangle& r) {
		_rectangle = r;
	}
	const astro::image::MosaicType&	mosaic() const { return _mosaic; }
	void	mosaic(const astro::image::MosaicType m) { _mosaic = m; }
private:
	double	_colorscales[3];
	double	_coloroffsets[3];
public:
	void	setColorScales(double, double, double);
	void	setColorScale(int, double);
	void	setColorOffsets(double, double, double);
	void	setColorOffset(int, double);
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

	// crosshairs
	void	drawCrosshairs(QPixmap *);

public:
	QPixmap	*histogram(int width, int height);
	
	QPixmap	*operator()(astro::image::ImagePtr image);
};

} // namespace snowgui

#endif /* _Image2Pixmap_h */
