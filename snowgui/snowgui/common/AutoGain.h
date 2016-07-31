/*
 * AutoGain.h -- class to determine the best gain settings for an image
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AutoGain_h
#define _AutoGain_h

#include <AstroImage.h>

namespace snowgui {

class AutoGain {
	double	_gain;
	double	_brightness;
	void	setup(double minimum, double maximum);
	void	setup(const astro::image::ConstImageAdapter<double>& doubleadapter,
			const astro::image::ImageRectangle& rectangle);
public:
	AutoGain(const astro::image::ImagePtr image);
	AutoGain(const astro::image::ImagePtr image,
		const astro::image::ImageRectangle& rectangle);
	double	gain() const { return _gain; }
	double	brightness() const { return _brightness; }
};

} // namespace snowgui

#endif /* _AutoGain_h */
