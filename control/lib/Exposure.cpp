/*
 * Exposure.cpp -- implementation of the exposure class
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>
#include <AstroFormat.h>
#include <cmath>
#include <AstroIO.h>

using namespace astro::image;
using namespace astro::io;

namespace astro {
namespace camera {

Exposure::Exposure() : exposuretime(1.), gain(1.), limit(INFINITY),
	mode(1,1), shutter(SHUTTER_OPEN) {
}

Exposure::Exposure(const ImageRectangle& _frame,
	float _exposuretime)
                : frame(_frame), exposuretime(_exposuretime), gain(1.),
		  limit(INFINITY), shutter(SHUTTER_OPEN) {
}

std::string	Exposure::toString() const {
	return stringprintf("%dx%d@(%d,%d)/%s for %.3fs %s g=%.1f, l=%.0f",
		frame.size().width(), frame.size().height(),
		frame.origin().x(), frame.origin().y(),
		mode.toString().c_str(), exposuretime,
		(shutter == SHUTTER_OPEN) ? "light" : "dark", gain, limit);
}

std::ostream&	operator<<(std::ostream& out, const Exposure& exposure) {
	return out << exposure.toString();
}

void	Exposure::addToImage(ImageBase& image) const {
	// exposure time
	image.setMetadata(
		FITSKeywords::meta(std::string("EXPTIME"), exposuretime));

	// X binning
	long	binning;
	binning = mode.getX();
	image.setMetadata(
		FITSKeywords::meta(std::string("XBINNING"), binning));

	// Y binning
	binning = mode.getY();
	image.setMetadata(
		FITSKeywords::meta(std::string("YBINNING"), binning));

	// subframe information
	image.setMetadata(
		FITSKeywords::meta(std::string("XORGSUBF"),
			(long)frame.origin().x()));

	image.setMetadata(
		FITSKeywords::meta(std::string("YORGSUBF"),
			(long)frame.origin().y()));

	// limit information
	if (limit != INFINITY) {
		image.setMetadata(
			FITSKeywords::meta(std::string("DATAMAX"), limit));
	}
}

} // namespace camera
} // namespace astro
