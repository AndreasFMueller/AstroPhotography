/*
 * Stack.cpp -- implementation of the stack class
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroStacking.h>
#include <AstroDebug.h>

namespace astro {
namespace image {
namespace stacking {

using namespace astro::adapter;

Stack::Stack(ImagePtr baseimage) : _base(baseimage) {
}

typedef std::shared_ptr<StackingAdapter>	StackingAdapterPtr;

/**
 * \brief Add an image
 *
 * Adding an image to the stack means that we have to find the transform
 * is needed to make the image congruent to the base image
 */
void	Stack::add(ImagePtr image) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"adding %s-sized image to stack (already %d images)",
		image->size().toString().c_str(), size());

	// create a new layer
	LayerPtr	newlayer = LayerPtr(new Layer(image));

	// get adapters for the two images to compare
	StackingAdapter	*baseadapter = StackingAdapter::get(_base);
	StackingAdapterPtr	baseadapterptr(baseadapter);
	StackingAdapter	*imageadapter = StackingAdapter::get(image);
	StackingAdapterPtr	imageadapterptr(imageadapter);

	// use a transform analyzer to find the transform, add the transform
	// to the layer
	transform::TransformAnalyzer	ta(*baseadapter, 2048, 2048);
	transform::Transform	t = ta.transform(*imageadapter);
	newlayer->transform(t);

	// add the layer to the stack
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding layer %d: %s", size(),
		newlayer->toString().c_str());
	push_back(newlayer);
}

} // namespace stacking
} // namespace image
} // namespace astro
