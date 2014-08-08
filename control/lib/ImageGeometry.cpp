/*
 * ImageGeometry.cpp -- ImageGeometry implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroChart.h>
#include <string>
#include <includes.h>

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
		: _size(image.size()) {
	_pixelsize = std::stod(image.getMetadata(std::string("PXLWIDTH"))
		.getValue()) / 1000000.;
	double	pxy = std::stod(image.getMetadata(std::string("PXLHIGHT"))
		.getValue()) / 1000000.;
	if (_pixelsize != pxy) {
		throw std::runtime_error("cannot handle nonsquare pixels");
	}
	_focallength = std::stod(image.getMetadata(std::string("FOCAL"))
		.getValue());
}

} // namespace catalog
} // namespace astro
