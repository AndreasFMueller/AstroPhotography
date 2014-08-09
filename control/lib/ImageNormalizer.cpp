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
	
}

} // namespace catalog
} // namespace astro
