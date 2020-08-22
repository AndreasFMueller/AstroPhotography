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

Exposure::Exposure() : _exposuretime(1.), _gain(-1.), _limit(INFINITY),
	_mode(1,1), _shutter(Shutter::OPEN), _purpose(Exposure::light),
	_quality(Exposure::fast) {
}

Exposure::Exposure(const ImageRectangle& frame,
	float exposuretime)
                : _frame(frame), _exposuretime(exposuretime), _gain(-1.),
		  _limit(INFINITY), _shutter(Shutter::OPEN),
		  _purpose(Exposure::light), _quality(Exposure::fast) {
}

std::string	Exposure::toString() const {
	return stringprintf("%dx%d@(%d,%d)/%s for %.3fs %s %s g=%.1f, l=%.0f",
		_frame.size().width(), _frame.size().height(),
		_frame.origin().x(), _frame.origin().y(),
		_mode.toString().c_str(), _exposuretime,
		(_shutter == Shutter::OPEN) ? "open" : "closed",
		purpose2string(_purpose).c_str(), _gain, _limit);
}

std::ostream&	operator<<(std::ostream& out, const Exposure& exposure) {
	return out << exposure.toString();
}

void	Exposure::addToImage(ImageBase& image) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add exposure %s to the image",
		toString().c_str());
	// exposure time
	image.setMetadata(
		FITSKeywords::meta(std::string("EXPTIME"), _exposuretime));

	// X binning
	long	binning;
	binning = _mode.x();
	image.setMetadata(
		FITSKeywords::meta(std::string("XBINNING"), binning));

	// Y binning
	binning = _mode.y();
	image.setMetadata(
		FITSKeywords::meta(std::string("YBINNING"), binning));

	// subframe information
	image.setMetadata(
		FITSKeywords::meta(std::string("XORGSUBF"),
			(long)_frame.origin().x()));

	image.setMetadata(
		FITSKeywords::meta(std::string("YORGSUBF"),
			(long)_frame.origin().y()));

	// limit information
	if (_limit != INFINITY) {
		image.setMetadata(
			FITSKeywords::meta(std::string("DATAMAX"), _limit));
	}

	// add the gain
	if (_gain != -1.0) {
		image.setMetadata(
			FITSKeywords::meta(std::string("GAIN"), _gain));
	}

	// purpose information
	image.setMetadata(FITSKeywords::meta(std::string("PURPOSE"),
		purpose2string(_purpose)));

	// quality information
	image.setMetadata(FITSKeywords::meta(std::string("QUALITY"),
		quality2string(_quality)));
}

bool	Exposure::needsshutteropen() const {
	switch (_purpose) {
	case flat:
	case light:
	case test:
	case guide:
	case focus:
	case preview:
		return true;
	case dark:
	case bias:
	case flood:
		return false;
	}
	throw std::runtime_error("unknown purpose");
}

std::string	Exposure::purpose2string(purpose_t p) {
	switch (p) {
	case dark:
		return std::string("dark");
	case flat:
		return std::string("flat");
	case light:
		return std::string("light");
	case bias:
		return std::string("bias");
	case test:
		return std::string("test");
	case guide:
		return std::string("guide");
	case focus:
		return std::string("focus");
	case flood:
		return std::string("flood");
	case preview:
		return std::string("preview");
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
	if (p == "bias") {
		return bias;
	}
	if (p == "test") {
		return test;
	}
	if (p == "guide") {
		return guide;
	}
	if (p == "focus") {
		return focus;
	}
	if (p == "preview") {
		return preview;
	}
	std::string	msg = stringprintf("unknown purpose %s", p.c_str());
	throw std::runtime_error(msg);
}

std::string	Exposure::quality2string(quality_t q) {
	switch (q) {
	case high:
		return std::string("high");
	case fast:
		return std::string("fast");
	}
	std::string	msg = stringprintf("unknown quality %d", q);
	throw std::runtime_error(msg);
}

Exposure::quality_t	Exposure::string2quality(const std::string& q) {
	if (q == "high") {
		return high;
	}
	if (q == "fast") {
		return fast;
	}
	std::string	msg = stringprintf("unknown quality %s", q.c_str());
	throw std::runtime_error(msg);
}

bool	Exposure::operator==(const Exposure& exposure) const {
	if (_frame != exposure._frame) {
		return false;
	}
	if (_exposuretime != exposure._exposuretime) {
		return false;
	}
	if (_gain != exposure._gain) {
		return false;
	}
	if (_limit != exposure._limit) {
		return false;
	}
	if (_mode != exposure._mode) {
		return false;
	}
	if (_shutter != exposure._shutter) {
		return false;
	}
	if (_purpose != exposure._purpose) {
		return false;
	}
	return true;
}

bool	Exposure::operator!=(const Exposure& exposure) const {
	return (!((*this) == exposure));
}

} // namespace camera
} // namespace astro
