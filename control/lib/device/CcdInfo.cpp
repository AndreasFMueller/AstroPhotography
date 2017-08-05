/*
 * CcdInfo.cpp -- CcdInfo implementation
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroCamera.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <AstroExceptions.h>
#include <AstroIO.h>
#include <AstroUtils.h>
#include <includes.h>
#include <sstream>

using namespace astro::image;
using namespace astro::io;

namespace astro {
namespace camera {

DeviceName	CcdInfo::defaultname(const DeviceName& parent,
			const std::string& unitname) {
	return DeviceName(parent, DeviceName::Ccd, unitname);
}

CcdInfo::CcdInfo(const std::string& name, const ImageSize& size, int _ccdid)
	: _name(name), _size(size), ccdid(_ccdid) {
	_shutter = false;
	// the default pixel width and height is set to 0 to indicate that
	// it is not known yet
	_pixelwidth = 0;
	_pixelheight = 0;
	_minexposuretime = 0.001;
	_maxexposuretime = 3600;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "constructor: %s",
		this->toString().c_str());
}

CcdInfo::CcdInfo(const CcdInfo& other)
	: _name(other.name()), _size(other.size()), ccdid(other.getId()),
	  binningmodes(other.modes()), _shutter(other.shutter()),
	  _pixelwidth(other.pixelwidth()), _pixelheight(other.pixelheight()),
	  _maxexposuretime(other.maxexposuretime()),
	  _minexposuretime(other.minexposuretime()) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"copy constructor: %s <- %s",
		this->toString().c_str(), other.toString().c_str(),
		binningmodes.size());
}

CcdInfo&	CcdInfo::operator=(const CcdInfo& other) {
	_name = other.name();
	_size = other.size();
	ccdid = other.getId();
	binningmodes = other.modes();
	_shutter = other.shutter();
	_pixelwidth = other.pixelwidth();
	_pixelheight = other.pixelheight();
	_minexposuretime = other.minexposuretime();
	_maxexposuretime = other.maxexposuretime();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "assignment operator: %s <- %s",
		this->toString().c_str(), other.toString().c_str());
	return *this;
}

/**
 * \brief Get a frame filling the CCD
 *
 * This method returns an image rectangle that fills the CCD. This can
 * be used to initialize the exposure object for the getExposure
 * method. Some cameras, like the UVC cameras, can only display full
 * frames, not subframes.
 */
const ImageRectangle	CcdInfo::getFrame() const {
	return ImageRectangle(ImagePoint(0, 0), _size);
}

/**
 * \brief add a binning mode
 */
void	CcdInfo::addMode(const Binning& mode) {
	binningmodes.insert(mode);
}

/**
 * \brief add a set of binning modes
 */
void	CcdInfo::addModes(const BinningSet& modes) {
	BinningSet::const_iterator	i;
	for (i = modes.begin(); i != modes.end(); i++) {
		addMode(*i);
	}
}

/**
 * \brief Return a string representation.
 */
std::string	CcdInfo::toString(bool withbinningmodes) const {
	std::ostringstream	out;
	out << (std::string)name() <<
		stringprintf(": %ux%u (%.1fum x %.1fum),",
			_size.width(), _size.height(),
			_pixelwidth * 1000000, _pixelheight * 1000000);
	if (withbinningmodes) {
		out << binningmodes.toString();
	} else {
		out << stringprintf("%d binning modes", binningmodes.size());
	}
	return out.str();
}

std::ostream&	operator<<(std::ostream& out, const CcdInfo& ccdinfo) {
	return out << ccdinfo.toString();
}

/**
 * \brief Fit a rectangle inside a ccd
 */
ImageRectangle	CcdInfo::clipRectangle(const ImageRectangle& rectangle) const {
	if (_size.width() < rectangle.origin().x()) {
		throw std::runtime_error("image rectangle outside ccd");
	}
	if (_size.height() < rectangle.origin().y()) {
		throw std::runtime_error("image rectangle outside ccd");
	}
	int	w = rectangle.size().width();
	if ((rectangle.size().width() + rectangle.origin().x()) > _size.width()) {
		w = _size.width() - rectangle.origin().x();
	}
	int	h = rectangle.size().height();
	if ((rectangle.size().height() + rectangle.origin().y()) > _size.height()) {
		h = _size.height() - rectangle.origin().y();
	}
	return ImageRectangle(rectangle.origin(), ImageSize(w, h));
}

/**
 * \brief Get a centered rectangle of a given size
 *
 * \param s	size for the rectangled to be computed
 */
ImageRectangle	CcdInfo::centeredRectangle(const ImageSize& s) const {
	int	w = s.width();
	int	h = s.height();
	if (w > _size.width()) {
		w = _size.width();
	}
	if (h > _size.height()) {
		h = _size.height();
	}
	int	xoffset = (_size.width() - w) / 2;
	int	yoffset = (_size.height() - h) / 2;
	return ImageRectangle(ImagePoint(xoffset, yoffset), ImageSize(w, h));
}

/**
 * \brief Add Metadata from the CCD to the image
 *
 * \param image	image to add the information to
 */
void	CcdInfo::addMetadata(ImageBase& image) const {
	image.setMetadata(
		FITSKeywords::meta(std::string("PXLWIDTH"),
			_pixelwidth * 1000000.));
	image.setMetadata(
		FITSKeywords::meta(std::string("PXLHIGHT"),
			_pixelheight * 1000000.));
}

} // namespace camera
} // namespace astro
