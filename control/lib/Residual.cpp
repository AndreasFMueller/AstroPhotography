/*
 * Residual.cpp -- a class used to represent differences in image mappings
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTransform.h>

namespace astro {
namespace image {
namespace transform {

bool	Residual::invalid() const {
	return ((offset().x() != offset().x())
		|| (offset().y() != offset().y()));
}

} // namespace transform
} // namespace image
} // namespace astro
