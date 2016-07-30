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
public:
	Image2Pixmap();
	~Image2Pixmap();

	double	brightness() const { return _brightness; }
	void	brightness(double b) { _brightness = b; }
	double	gain() const { return _gain; }
	void	gain(double g) { _gain = g; }
private:
	HistogramBase	*_histogram;
	QImage	*convertRGB(astro::image::ImagePtr image);
	QImage	*convertMono(astro::image::ImagePtr image);

public:
	QPixmap	*histogram(int width, int height);
	
	QPixmap	*operator()(astro::image::ImagePtr image);
};

} // namespace snowgui

#endif /* _Image2Pixmap_h */
