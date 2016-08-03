/*
 * AstroStacking.h -- primitive stacking function
 * 
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroStacking_h
#define _AstroStacking_h

#include <AstroImage.h>
#include <AstroTransform.h>
#include <vector>

namespace astro {
namespace image {
namespace stacking {

/**
 * \brief Layer of a stack
 *
 * A stack consists of a number of layers, each layer consists of an image
 * and the transformation that makes the layer congruent to the base image
 * of the stack.
 */
class Layer {
	ImagePtr	_image;
	transform::Transform	_transform;
public:
	const transform::Transform&	transform() const { return _transform; }
	void	transform(const transform::Transform& t) { _transform = t; }
public:
	Layer(ImagePtr image);
	std::string	toString() const;
};
typedef std::shared_ptr<Layer>	LayerPtr;

/**
 * \brief A stack of layers
 */
class Stack : public std::vector<LayerPtr> {
	ImagePtr	_base;
public:
	ImagePtr	base() const { return _base; }
	Stack(ImagePtr baseimage);
	void	add(ImagePtr image);
};

/**
 * \brief Stacker class
 */
class Stacker {
	int	_patchsize;
public:
	Stacker(int patchsize = 256) : _patchsize(patchsize) { }
	ImagePtr	operator()(ImageSequence images);
};

} // namespace stacking
} // namespace image
} // namespace astro

#endif /* _AstroStacking_h */
