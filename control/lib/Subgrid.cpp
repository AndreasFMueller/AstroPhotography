/*
 * Subgrid.cpp -- Subgrid implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroImage.h>
#include <AstroFormat.h>

namespace astro {
namespace image {

Subgrid::Subgrid() : origin(0, 0), stepsize(1, 1) {
}

Subgrid::Subgrid(const ImagePoint& _origin, const ImageSize& _stepsize)
	: origin(_origin), stepsize(_stepsize) {
}

Subgrid::Subgrid(const Subgrid& other) : origin(other.origin),
	stepsize(other.stepsize) {
}

unsigned int	Subgrid::x(unsigned int _x) const {
	return origin.x + _x * stepsize.getWidth();
}

unsigned int	Subgrid::y(unsigned int _y) const {
	return origin.y + _y * stepsize.getHeight();
}

unsigned int	Subgrid::volume() const {
	return stepsize.getPixels();
}

std::string	Subgrid::toString() const {
	return stringprintf("%ux%u@(%u,%u)",
		stepsize.getWidth(), stepsize.getHeight(),
		origin.x, origin.y);
}

} // namespace image
} // namespace astro
