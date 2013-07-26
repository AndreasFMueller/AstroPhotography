/*
 * AstroDisplay.h -- conversion functions to produce displayable versions
 *                   of astro images
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroDisplay_h
#define _AstroDisplay_h

#include <AstroImage.h>

namespace astro {
namespace image {

class DisplayConverter {
	ImageRectangle	subframe;
	double	scale;
	double	minpixel;
	double	maxpixel;
	bool	color;

	Image<RGB<unsigned char> >	*convertColor(const ImagePtr image);
	Image<RGB<unsigned char> >	*convertBW(const ImagePtr image);
public:
	DisplayConverter();

	void	setSubframe(const ImageRectangle& subframe);
	ImageRectangle	getSubframe() const;

	void	setScale(double scale);
	double	getScale() const;

	void	setMinpixel(double minpixel);
	double	getMinpixel() const;

	void	setMaxpixel(double maxpixel);
	double	getMaxpixel() const;

	void	setColor(bool color);
	bool	getColor() const;

	Image<RGB<unsigned char> >	*operator()(const ImagePtr image);
};

} // namespace image
} // namespace astro

#endif /* _AstroDisplay_h */
