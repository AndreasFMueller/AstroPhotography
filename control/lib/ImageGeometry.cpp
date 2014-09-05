/*
 * ImageGeometry.cpp -- ImageGeometry implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroChart.h>
#include <string>
#include <includes.h>
#include <AstroFormat.h>
#include <AstroIO.h>

using namespace astro::io;

namespace astro {
namespace catalog {

/**
 * \brief Add geometry information to an image
 *
 * \param image		image to add the meta data to
 */
void	ImageGeometry::addMetadata(ImageBase& image) const {
	image.setMetadata(
		FITSKeywords::meta(std::string("PXLWIDTH"), 
			_pixelsize * 1000000.));
	image.setMetadata(
		FITSKeywords::meta(std::string("PXLHIGHT"), 
			_pixelsize * 1000000.));
	image.setMetadata(
		FITSKeywords::meta(std::string("FOCAL"), _focallength));
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
		((std::string)(image.getMetadata(std::string("PXLWIDTH")))).c_str());
	_pixelsize = (double)(image.getMetadata(std::string("PXLWIDTH")))
			/ 1000000.;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "pixel size: %f", _pixelsize);
	double	pxy = (double)(image.getMetadata(std::string("PXLHIGHT")))
			/ 1000000.;
	if (_pixelsize != pxy) {
		throw std::runtime_error("cannot handle nonsquare pixels");
	}
	_focallength = (double)(image.getMetadata(std::string("FOCAL")));
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

/**
 * \brief compute relative coordinates
 */
Point	ImageGeometry::coordinates(const Point& a) const {
	Point	relative = (a - center());
	return Point(2 * relative.x() / width(), 2 * relative.y() / height());
}

} // namespace catalog
} // namespace astro
