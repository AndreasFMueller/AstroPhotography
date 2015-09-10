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
}

} // namespace adapter
} // namespace astro
