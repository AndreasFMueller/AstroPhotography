/*
 * Image2Pixmap.h -- convert an ImagePtr to a pixmap
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <QPixmap>
#include <QImage>
#include <AstroImage.h>

namespace snowgui {

class	Image2Pixmap {
	double	_brightness;
	double	_gain;
public:
	Image2Pixmap() {
		_brightness = 0;
		_gain = 1;
	}

	double	brightness() const { return _brightness; }
	void	brightness(double b) { _brightness = b; }
	double	gain() const { return _gain; }
	void	gain(double g) { _gain = g; }
private:
	QImage	*convertRGB(astro::image::ImagePtr image);
	QImage	*convertMono(astro::image::ImagePtr image);

public:
	
	QPixmap	*operator()(astro::image::ImagePtr image);
};

} // namespace snowgui
