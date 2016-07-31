/*
 * BasicAdapter.cpp -- implementation of the basic adapter class
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroImage.h>

using namespace astro::image;

namespace astro {
namespace image {

BasicAdapter::BasicAdapter(const ImageSize& size) : adaptersize(size) {
}

BasicAdapter::~BasicAdapter() {
}

const ImageSize&	BasicAdapter::getSize() const {
	return adaptersize;
}

} // namespace image
} // namespace astro
