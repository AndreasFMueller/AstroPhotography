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


class Stacker;
typedef std::shared_ptr<Stacker>	StackerPtr;
/**
 * \brief Stacker class
 */
class Stacker {
protected:
	ImagePtr	_baseimage;
	int	_patchsize;
public:
	int	patchsize() const { return _patchsize; }
	void	patchsize(int p) { _patchsize = p; }
protected:
	int	_numberofstars;
public:
	int	numberofstars() const { return _numberofstars; }
	void	numberofstars(int n) { _numberofstars = n; }
protected:
	int	_searchradius;
public:
	int	searchradius() const { return _searchradius; }
	void	searchradius(int s) { _searchradius = s; }
private:
	bool	_notransform;
public:
	bool	notransform() const { return _notransform; }
	void	notransform(bool n) { _notransform = n; }

	static StackerPtr	get(ImagePtr baseimage);
protected:
	Stacker(ImagePtr baseimage) : _baseimage(baseimage), _patchsize(256) { }
public:
	virtual void	add(ImagePtr) = 0;
	virtual ImagePtr	image() = 0;

protected:
	transform::Transform	findtransform(const ConstImageAdapter<double>& base,
				const ConstImageAdapter<double>& image) const;
};

} // namespace stacking
} // namespace image
} // namespace astro

#endif /* _AstroStacking_h */
