/*
 * Residual.cpp -- a class used to represent differences in image mappings
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTransform.h>
#include <AstroFormat.h>

namespace astro {
namespace image {
namespace transform {

bool	Residual::invalid() const {
	return ((offset().x() != offset().x())
		|| (offset().y() != offset().y()));
}

Residual::operator	std::string() const {
	return stringprintf("@%s offset=%s (%f, %f)",
		_from.toString().c_str(), _offset.toString().c_str(), _weight,
		log(_weight));
}

} // namespace transform
} // namespace image
} // namespace astro
