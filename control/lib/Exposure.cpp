/*
 * Exposure.cpp -- implementation of the exposure class
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>
#include <AstroFormat.h>
#include <math.h>

using namespace astro::image;

namespace astro {
namespace camera {

Exposure::Exposure() : exposuretime(1.), gain(1.), limit(INFINITY) {
}

Exposure::Exposure(const ImageRectangle& _frame,
	float _exposuretime)
                : frame(_frame), exposuretime(_exposuretime), gain(1.),
		  limit(INFINITY) {
}

std::string	Exposure::toString() const {
	return stringprintf("%dx%d@(%d,%d)/%s for %.3fs",
		frame.size.getWidth(), frame.size.getHeight(),
		frame.origin.x, frame.origin.y,
		mode.toString().c_str(), exposuretime);
}

std::ostream&	operator<<(std::ostream& out, const Exposure& exposure) {
	return out << exposure.toString();
}

void	Exposure::addToImage(ImageBase& image) const {
	// exposure time
	Metavalue	mv(exposuretime,
		std::string("duration of exposure in seconds"));
	image.setMetadata(std::string("EXPTIME"), mv);

	// X binning
	unsigned int	binning;
	if (mode.isXwildcard()) {
		binning = 0;
	} else {
		binning = mode.getX();
	}
	Metavalue	mvbinx(binning,
		std::string("binning factor used on X axis"));
	image.setMetadata(std::string("XBINNING"), mvbinx);

	// Y binning
	if (mode.isYwildcard()) {
		binning = 0;
	} else {
		binning = mode.getY();
	}
	Metavalue	mvbiny(binning,
		std::string("binning factor used on Y axis"));
	image.setMetadata(std::string("YBINNING"), mvbiny);

	// subframe information
	Metavalue	mvorigx(frame.origin.x,
		std::string("subframe origin on X axis"));
	image.setMetadata(std::string("XORGSUBF"), mvorigx);

	Metavalue	mvorigy(frame.origin.y,
		std::string("subframe origin on Y axis"));
	image.setMetadata(std::string("YORGSUBF"), mvorigy);

	// limit information
	if (limit != INFINITY) {
		Metavalue	mvlimit(limit, 
			std::string("pixel values above this level are "
			"considered saturated"));
		image.setMetadata(std::string("DATAMAX"), mvlimit);
	}
}

} // namespace camera
} // namespace astro
