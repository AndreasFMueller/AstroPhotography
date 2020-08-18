/*
 * Chart.cpp -- class to create a chart from a set of stars
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroChart.h>
#include <AstroDebug.h>
#include <AstroFormat.h>

using namespace astro::image;

namespace astro {
namespace catalog {

/**
 * \brief Create a chart
 */
Chart::Chart(const SkyRectangle rectangle, const ImageSize& size)
	: _rectangle(rectangle), _size(size) {
	// create the image
	_image =  new Image<double>(size);
	_imageptr = ImagePtr(_image);
	_image->fill(0);

	// add the metadata
	rectangle.addMetadata(*_image);
}

} // namespace catalog
} // namespace astro
