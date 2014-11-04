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
	mode(1,1), shutter(Shutter::OPEN), purpose(Exposure::light) {
}

Exposure::Exposure(const ImageRectangle& _frame,
	float _exposuretime)
                : frame(_frame), exposuretime(_exposuretime), gain(1.),
		  limit(INFINITY), shutter(Shutter::OPEN),
		  purpose(Exposure::light) {
}

std::string	Exposure::toString() const {
	return stringprintf("%dx%d@(%d,%d)/%s for %.3fs %s g=%.1f, l=%.0f",
		frame.size().width(), frame.size().height(),
		frame.origin().x(), frame.origin().y(),
		mode.toString().c_str(), exposuretime,
		(shutter == Shutter::OPEN) ? "light" : "dark", gain, limit);
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

	// purpose information
	image.setMetadata(FITSKeywords::meta(std::string("PURPOSE"),
		purpose2string(purpose)));
}

std::string	Exposure::purpose2string(purpose_t p) {
	switch (p) {
	case dark:
		return std::string("dark");
	case flat:
		return std::string("flat");
	case light:
		return std::string("light");
	}
	std::string	msg = stringprintf("unknown purpose %d", p);
	throw std::runtime_error(msg);
}

Exposure::purpose_t	Exposure::string2purpose(const std::string& p) {
	if (p == "dark") {
		return dark;
	}
	if (p == "flat") {
		return flat;
	}
	if (p == "light") {
		return light;
	}
	std::string	msg = stringprintf("unknown purpose %s", p.c_str());
	throw std::runtime_error(msg);
}

std::string	Exposure::state2string(State s) {
	switch (s) {
	case idle:
		return std::string("idle");
	case exposing:
		return std::string("exposing");
	case exposed:
		return std::string("exposed");
	case cancelling:
		return std::string("cancelling");
	}
	throw std::runtime_error("unknown exposure state");
}

Exposure::State	Exposure::string2state(const std::string& s) {
	if (s == "idle") {
		return idle;
	}
	if (s == "exposing") {
		return exposing;
	}
	if (s == "exposed") {
		return exposed;
	}
	if (s == "cancelling") {
		return cancelling;
	}
	throw std::runtime_error("unknown exposure state");
}

} // namespace camera
} // namespace astro
