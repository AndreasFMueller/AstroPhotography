/*
 * NoiseAdapter.cpp -- base class for the noise adapters
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroAdapter.h>

namespace astro {
namespace adapter {

NoiseAdapter::NoiseAdapter(const ImageSize& size)
	: ConstImageAdapter(size) {
	_background = NULL;
}

double	NoiseAdapter::pixel(int x, int y) const {
	if (_background) {
		return _background->pixel(x, y);
	}
	return 0.;
}

} // namespace adapter
} // namespace astro
