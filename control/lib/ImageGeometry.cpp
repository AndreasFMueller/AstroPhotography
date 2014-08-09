/*
 * ImageGeometry.cpp -- ImageGeometry implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroChart.h>
#include <string>
#include <includes.h>
#include <AstroFormat.h>

namespace astro {
namespace catalog {

/**
 * \brief Add geometry information to an image
 *
 * \param image		image to add the meta data to
 */
void	ImageGeometry::addMetadata(ImageBase& image) const {
	image.setMetadata(std::string("PXLWIDTH"), 
		Metavalue(_pixelsize * 1000000.,
			std::string("width of a pixel in microns")));
	image.setMetadata(std::string("PXLHIGHT"), 
		Metavalue(_pixelsize * 1000000.,
			std::string("height of a pixel in microns")));
	image.setMetadata(std::string("FOCAL"),
		Metavalue(_focallength,
			std::string("focal length in m")));
}

/**
 * \brief Create an ImageGeometry object from the headers in an image
 *
 * This constructor takes the size from the image, and the pixel size
 * and focal length information from the headers.
 *
 * \param image	from which to create the ImageGeomtry object
 */
ImageGeometry::ImageGeometry(const ImageBase& image)
		: ImageSize(image.size()) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get pixel width: %s",
		image.getMetadata(std::string("PXLWIDTH")).getValue().c_str());
	_pixelsize = std::stod(image.getMetadata(std::string("PXLWIDTH"))
		.getValue()) / 1000000.;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "pixel size: %f", _pixelsize);
	double	pxy = std::stod(image.getMetadata(std::string("PXLHIGHT"))
		.getValue()) / 1000000.;
	if (_pixelsize != pxy) {
		throw std::runtime_error("cannot handle nonsquare pixels");
	}
	_focallength = std::stod(image.getMetadata(std::string("FOCAL"))
		.getValue());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "focal length: %f", _focallength);
}

/**
 * \brief Get the angular width of the rectangle belonging to this geometry
 */
Angle	ImageGeometry::rawidth() const {
        return Angle(width() * _pixelsize / _focallength);
}

/**
 * \brief Get the angular height of the rectangle belonging to this geometry
 */
Angle	ImageGeometry::decheight() const {
        return Angle(height() * _pixelsize / _focallength);
}

/**
 * \brief Convert the geometry to a string
 */
std::string	ImageGeometry::toString() const {
	return stringprintf("%s, pxsz = %f, f = %f",
		ImageSize::toString().c_str(), _pixelsize, _focallength);
}

} // namespace catalog
} // namespace astro
