/*
 * ImageNormalizer.cpp -- normalize onto star chart at the center of the image
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroChart.h>

using namespace astro::image;
using namespace astro::image::transform;

namespace astro {
namespace catalog {

ImageNormalizer::ImageNormalizer(ChartFactory& factory) : _factory(factory) { }

/**
 * \brief Auxiliary function to extract current center coordinates from image
 */
static RaDec	get_center(ImagePtr image) {
	RaDec	center;
	std::string	v = image->getMetadata("RACENTR").getValue();
	center.ra().hours(std::stod(v));
	v = image->getMetadata("DECCENTR").getValue();
	center.dec().degrees(std::stod(v));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "current image center: %s",
		center.toString().c_str());
	return center;
}

/**
 * \brief Compute the true 
 *
 * \param image			
 * \param initialprojection	initial value for the projection
 */
RaDec	ImageNormalizer::operator()(ImagePtr image, Projection& projection) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "normalizing %s image",
		image->size().toString().c_str());
	// get the geometry of the image
	ImageGeometry	geometry(*image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image geometry: %s",
		geometry.toString().c_str());

	// find the current central coordinates of the image
	RaDec	center = get_center(image);

	// compute the coordinates of the center
}

} // namespace catalog
} // namespace astro
