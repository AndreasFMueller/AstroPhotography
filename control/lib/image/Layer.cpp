/*
 * Layer.cpp -- layer in a stack
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroStacking.h>
#include <AstroFormat.h>
#include <typeindex>

namespace astro {
namespace image {
namespace stacking {

Layer::Layer(ImagePtr image) : _image(image) {
}

std::string	Layer::toString() const {
	return stringprintf("Image<%s>%s transform=%s",
		_image->pixel_type().name(),
		_image->size().toString().c_str(),
		_transform.toString().c_str());
}

} // namespace stacking
} // namespace image
} // namespace stro
