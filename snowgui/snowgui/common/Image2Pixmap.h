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

class	Image2Pixmap {
	double	_brightness;
	double	_gain;
	bool	_logarithmic;
	int	_scale;
	astro::image::ImageRectangle	_rectangle;
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
private:
	HistogramBase	*_histogram;
	astro::image::ImageRectangle	rectangle(astro::image::ImagePtr image);
	QImage	*convertRGB(astro::image::ImagePtr image);
	QImage	*convertMono(astro::image::ImagePtr image);

public:
	QPixmap	*histogram(int width, int height);
	
	QPixmap	*operator()(astro::image::ImagePtr image);
};

} // namespace snowgui

#endif /* _Image2Pixmap_h */
