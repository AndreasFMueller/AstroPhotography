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
	// size of the patch to use when doing phase correlator analysis 
	int	_patchsize;
public:
	int	patchsize() const { return _patchsize; }
	void	patchsize(int p) { _patchsize = p; }
private:
	double	_residual;
public:
	double	residual() const { return _residual; }
	void	residual(double r) { _residual = r; }
protected:
	// number of stars th collect to build the triangle set
	int	_numberofstars;
public:
	int	numberofstars() const { return _numberofstars; }
	void	numberofstars(int n) { _numberofstars = n; }
protected:
	// radius in pixels to use when searching for triangles
	int	_searchradius;
public:
	int	searchradius() const { return _searchradius; }
	void	searchradius(int s) { _searchradius = s; }
private:
	// do not transform the images, just stack tham as they are
	bool	_notransform;
public:
	bool	notransform() const { return _notransform; }
	void	notransform(bool n) { _notransform = n; }
private:
	// whether to use the triangle analysis step to find the transforms
	bool	_usetriangles;
public:
	bool	usetriangles() const { return _usetriangles; }
	void	usetriangles(bool u) { _usetriangles = u; }
private:
	bool	_rigid;
public:
	bool	rigid() const { return _rigid; }
	void	rigid(bool r) { _rigid = r; }

	static StackerPtr	get(ImagePtr baseimage);
protected:
	Stacker(ImagePtr baseimage)
		: _baseimage(baseimage),
		  _patchsize(256), _residual(30),
		  _numberofstars(0), _searchradius(16),
		  _notransform(true), _usetriangles(false), _rigid(false) {
	}
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
