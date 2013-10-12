/*
 * AstroBackground.h -- classes for Background subtraction in images
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroBackground_h
#define _AstroBackground_h

#include <AstroTypes.h>
#include <AstroImage.h>
#include <vector>
#include <algorithm>

namespace astro {
namespace adapter {

/**
 * The classes defined in this file are used to perform background subtraction
 * in astrophotography images. This is mainly needed for images where 
 * atmospheric light (light pollution) changes the image.
 *
 * A background consists of three functions that can be linear or quadratic
 * (other functions could be implemented, but there currently is no reason
 * for doing so).
 */

template<typename f>
struct function_tag {
	typedef	f	FunctionType;
};

/**
 * \brief Common base class for all background functions
 *
 * The FunctionBase class defines all the methods needed by functions.
 * Some or not implemented, but it is essential that they are all defined,
 * so that we can access them via a pointer to the FunctionBase
 */
class FunctionBase {
public:
	typedef std::pair<Point, double>	doublevaluepair;
private:
	/**
 	 * \brief reduce a set of point-value pairs to a function
	 *
	 * This virtual method is only implemented in derived classes.
	 * Such a method must take the data from the values argument
	 * and compute the parameters of the function from it.
	 */
	virtual void	reduce(const std::vector<doublevaluepair>& values) = 0;
	/**
	 * \brief Whether or not to display changes
	 *
	 * Applications may only want to see part of the function. By setting
	 * _gradient to false, the function essentially becomes a constant.
	 */
	bool	_gradient;
public:
	bool	gradient() const { return _gradient; }
	void	gradient(bool gradient) { _gradient = gradient; }

private:
	/**
 	 * \brief flag that indicates whether the function should be symmetric
	 *
	 * By setting the _symmetric to false turns of all terms in the 
	 * function that make the function asymmetric. It turns of linear
	 * terms, and the only remaining term for a quadratic function are
	 * terms proportional to (x^2+y^2)
	 */
	bool	_symmetric;
public:
	bool	symmetric() const { return _symmetric; }
	void	symmetric(bool symmetric) { _symmetric = symmetric; }

private:
	/**
	 * \brief The scalefactor scales the pixel values
	 *
	 * This mechanism is used to adapt to different types of
	 * pixels. It allows to scale pixels to the maximum range of the
	 * pixel type.
	 */
	double	_scalefactor;
public:
	double	scalefactor() const { return _scalefactor; }
	void	scalefactor(double scalefactor) { _scalefactor = scalefactor; }

protected:
	/**
	 * \brief The center of the image
	 *
	 * We can only talk about symmetric functions if we have a symmetry
	 * center.
	 */
	ImagePoint	_center;
public:
	ImagePoint	center() const { return _center; }

public:

	/**
	 * \brief Create a new function
	 */
	FunctionBase(const ImagePoint& center, bool symmetric)
		: _symmetric(symmetric), _center(center) {
		_gradient = true;
		_scalefactor = 1.;
	}

	/**
	 * \brief Copy a function
	 */
	FunctionBase(const FunctionBase& other);

	/**
	 * \brief Function evaluation
	 *
	 * Function evaluation depends on the type of function, so the base
	 * class does not implement it. Derivce classes should override 
	 * this method.
	 */
	virtual double	evaluate(const Point& point) const = 0;
	double	evaluate(const ImagePoint& point) const;
	double	evaluate(unsigned int x, unsigned int y) const;

	double	operator()(const Point& point) const;
	double	operator()(const ImagePoint& point) const;
	double	operator()(unsigned int x, unsigned int y) const;

	/**
	 * \brief The norm encodes how close to zero the function is
	 */
	virtual double	norm() const = 0;

	virtual std::string	toString() const;
};

class FunctionBaseAdapter : public ConstImageAdapter<float> {
	const FunctionBase	*_funcp;
public:
	FunctionBaseAdapter(const ImageSize& size, const FunctionBase *funcp)
		: ConstImageAdapter<float>(size), _funcp(funcp) {
	}
	const float	pixel(unsigned int x, unsigned int y) const {
		return (*_funcp)(x, y);
	}
};

typedef std::tr1::shared_ptr<FunctionBase>	FunctionPtr;
FunctionPtr	operator+(const FunctionPtr& a, const FunctionPtr& b);

/**
 * \brief Adapter for an arbitrary function
 *
 * By using a FunctionPtr we can use any function to create the pixel
 * values
 */
class FunctionPtrAdapter : public ConstImageAdapter<float> {
	FunctionPtr	_function;
	ImagePoint	_origin;
public:
	FunctionPtrAdapter(const ImageSize& size, FunctionPtr function,
		const ImagePoint& origin)
		: ConstImageAdapter<float>(size), _function(function),
		  _origin(origin) {
	}

	const float	pixel(unsigned int x, unsigned int y) const {
		return _function->evaluate(_origin.x() + x, _origin.y() + y);
	}
};

class FunctionPtrSubtractionAdapter : public FunctionPtrAdapter {
	const ConstImageAdapter<float>&	_image;
public:
	FunctionPtrSubtractionAdapter(const ConstImageAdapter<float>& image,
		FunctionPtr function, const ImagePoint& origin)
		: FunctionPtrAdapter(image.getSize(), function, origin),
		  _image(image) {
	}
	const float	pixel(unsigned int x, unsigned int y) const {
//debug(LOG_DEBUG, DEBUG_LOG, 0, "%d,%d = %f - %f",
//	x, y, _image.pixel(x, y), FunctionPtrAdapter::pixel(x, y));
		return _image.pixel(x, y) - FunctionPtrAdapter::pixel(x, y);
	}
};

/**
 * \brief Gradients are described by linear functions on the image coordinates
 */
class LinearFunction : public FunctionBase {
	double	a[3];
protected:
	virtual void	reduce(const std::vector<doublevaluepair>& values);
public:
	// standard constructors
	LinearFunction(const ImagePoint& point, bool symmetric);
	LinearFunction(const LinearFunction& other);

	// constructor from data set
	LinearFunction(const ImagePoint& center, bool symmetric,
		const std::vector<doublevaluepair>& values);

	// evaluation
	virtual double	evaluate(const Point& point) const;
	virtual double	norm() const;

	// coefficient access
	virtual double	operator[](int i) const;
	virtual double&	operator[](int i);

	// linear operators
	LinearFunction	operator+(const LinearFunction& other);
	LinearFunction&	operator=(const LinearFunction& other);
	
	// text representation
	virtual std::string	toString() const;

	//typedef linear_function_tag	function_category;
	typedef function_tag<LinearFunction>	function_category;
};

/**
 * \brief Quadratic background function
 *
 * A quadratic function is a linear function that also has some quadratic
 * terms. 
 */
class QuadraticFunction : public LinearFunction {
	double	q[3];
protected:
	virtual void	reduce(const std::vector<doublevaluepair>& values);
public:
	QuadraticFunction(const ImagePoint& center, bool symmetric);
	QuadraticFunction(const LinearFunction& linear);
	virtual double	evaluate(const Point& point) const;
	virtual double	operator[](int i) const;
	virtual double&	operator[](int i);
	virtual double	norm() const;
	QuadraticFunction	operator+(const QuadraticFunction& other);
	QuadraticFunction	operator+(const LinearFunction& other);
	QuadraticFunction&	operator=(const QuadraticFunction& other);
	QuadraticFunction&	operator=(const LinearFunction& other);
	virtual std::string	toString() const;
	//typedef quadratic_function_tag	function_category;
	typedef function_tag<QuadraticFunction>	function_category;
};

/**
 * \brief
 */
template<typename Pixel, typename FunctionType>
class Function : public FunctionType {
public:
	typedef std::pair<Point, double>	doublevaluepair;
	typedef std::pair<Point, Pixel>	valuepair;
	typedef std::vector<valuepair>	values_type;

	Function() : FunctionType(ImagePoint(), true) { }

	Function(const ImagePoint& center, bool symmetric)
		: FunctionType(center, symmetric) { }

	Function(const FunctionType& other)
		: FunctionType(other) { }

	Function(const values_type& values) {
		std::vector<doublevaluepair>	converted;
		typename std::vector<valuepair>::const_iterator	i;
		for (i = values.begin(); i != values.end(); i++) {
			double	v = i->second;
			converted.push_back(std::make_pair(i->first, v));
		}
		this->reduce(converted);
	}

	virtual Pixel	operator()(const Point& point) const {
		Pixel	result = this->evaluate(point);
		return result;
	}

	virtual Pixel	operator()(const ImagePoint& point) const {
		return operator()(Point(point));
	}

	virtual Pixel	operator()(unsigned int x, unsigned int y) const {
		return operator()(Point(x, y));
	}
};

/**
 * \brief Function adapter template
 */
template<typename FunctionType>
class ImageFunctionAdapter : public ConstImageAdapter<float> {
public:
	typedef	Function<float, FunctionType>	function;
private:
	const function&		_func;
	const ImagePoint	_origin;
public:
	ImageFunctionAdapter(const ImageSize& size,
		const function& func,
		const ImagePoint& origin) :
		ConstImageAdapter<float>(size), _func(func), _origin(origin) {
	}

	virtual const float	pixel(unsigned int x, unsigned int y) const {
		return _func(_origin.x() + x, _origin.y() + y);
	}
};

template<typename FunctionType>
class FunctionSubtractionAdapter : public ConstImageAdapter<float> {
public:
	typedef	Function<float, FunctionType>	function;
private:
	const function&		_func;
	const ImagePoint	_origin;
	const ConstImageAdapter<float>&	_image;
public:
	FunctionSubtractionAdapter(const ConstImageAdapter<float>& image,
		const function& func,
		const ImagePoint& origin)
		: ConstImageAdapter<float>(image.getSize()), _image(image),
		  _func(func), _origin(origin) {
	}

	virtual const float	pixel(unsigned int x, unsigned int y) const {
		return _image.pixel(x, y)
				- _func(_origin.x() + x, _origin.y() + y);
	}
};

/**
 * \brief Estimate the minimum of the image
 */
template<typename FunctionType>
class MinimumEstimator {
	const ConstImageAdapter<float>&	_image;
	unsigned int	_alpha;
public:
	unsigned int	alpha() const { return _alpha; }
	MinimumEstimator(const ConstImageAdapter<float>& image,
		const unsigned int alpha)
		: _image(image), _alpha(alpha) { }
	FunctionPtr	operator()(const ImagePoint& center, bool symmetric) const;
};

/**
 * \brief Background base
 */
template<typename Pixel>
class BackgroundBase {
public:
	virtual bool	gradient() const = 0;
	virtual void	gradient(bool gradient) = 0;
	virtual bool	symmetric() const = 0;
	virtual void	symmetric(bool symmetric) = 0;
	virtual double	scalefactor() const = 0;
	virtual void	scalefactor(double scalefactor) = 0;
	virtual	RGB<Pixel>	operator()(const Point& point) const = 0;
	virtual RGB<Pixel>	operator()(const ImagePoint& point) const = 0;
	virtual RGB<Pixel>	operator()(unsigned int x, unsigned int y) const = 0;
};

typedef std::tr1::shared_ptr<BackgroundBase<float> >	BackgroundPtr;

/**
 * \brief Background template
 *
 * Backgrounds can use different functions for the background, linear functions
 * or quadratic functions being just the most common ones. To allow for more
 * different types of background functions, the Template has an argument
 * FunctionType that 
 *
 * \param Pixel		type of the pixel to be used by the the background
 * \param FunctionType	background function class to use to compute background
 *			values
 */
template<typename Pixel>
class Background : public BackgroundBase<Pixel> {
	FunctionPtr	R;
	FunctionPtr	G;
	FunctionPtr	B;
	double	r(const Point& point) const {
		return R->evaluate(point);
	}
	double	g(const Point& point) const {
		return G->evaluate(point);
	}
	double	b(const Point& point) const {
		return B->evaluate(point);
	}
public:
	Background() { }
	Background(const FunctionPtr _R,
		const FunctionPtr _G,
		const FunctionPtr _B) : R(_R), G(_G), B(_B) {
	}
	virtual bool	gradient() const {
		return R->gradient();
	}
	virtual void	gradient(bool gradient) {
		R->gradient(gradient);
		G->gradient(gradient);
		B->gradient(gradient);
	}
	virtual bool	symmetric() const {
		return R->symmetric();
	}
	virtual void	symmetric(bool symmetric) {
		R->symmetric(symmetric);
		G->symmetric(symmetric);
		B->symmetric(symmetric);
	}
	virtual double	scalefactor() const {
		return R->scalefactor();
	}
	virtual void	scalefactor(double scalefactor) {
		R->scalefactor(scalefactor);
		G->scalefactor(scalefactor);
		B->scalefactor(scalefactor);
	}
	virtual RGB<Pixel>	operator()(const Point& point) const {
		return RGB<Pixel>((Pixel)r(point), (Pixel)g(point), (Pixel)b(point));
	}
	virtual RGB<Pixel>	operator()(const ImagePoint& point) const {
		return this->operator()(Point(point.x(), point.y()));
	}
	virtual RGB<Pixel>	operator()(unsigned int x, unsigned int y) const {
		return this->operator()(Point(x, y));
	}
};

/**
 * \brief Background Extraction Factory
 *
 * This class implements extracting a background gradient with float pixels.
 * Other pixel types could be implemented similarly, but we only need it for
 * float pixels and formulating the whole argument as a template whould just
 * bee too much hassle.
 */
class BackgroundExtractor {
	unsigned int	alpha;
public:
	typedef enum { CONSTANT, LINEAR, QUADRATIC } functiontype;
	BackgroundExtractor(unsigned int _alpha) : alpha(_alpha) { }
	Background<float>	operator()(const ImagePoint& center,
				bool symmetric, functiontype f,
				const Image<RGB<float> >& image) const;
	Background<float>	operator()(const ImagePoint& center,
				bool symmetric, functiontype f,
				const Image<float>& image) const;
};

/**
 * \brief Backgroud Subtraction
 *
 * We implement this as an adapter
 */
class BackgroundSubtractionAdapter : public ConstImageAdapter<RGB<float> > {
public:
	typedef	Background<float>	BackgroundType;
private:
	const ConstImageAdapter<RGB<float> >&	_image;
	Background<float>	_background;
public:
	BackgroundSubtractionAdapter(
		const ConstImageAdapter<RGB<float> >& image,
		const Background<float>& background)
		: ConstImageAdapter<RGB<float> >(image.getSize()),
		  _image(image), _background(background) {
	}
	BackgroundSubtractionAdapter(
		const ConstImageAdapter<RGB<float> >& image)
		: ConstImageAdapter<RGB<float> >(image.getSize()),
		  _image(image) {
	}
	virtual const RGB<float>	pixel(unsigned int x, unsigned int y) const {
		return _image.pixel(x, y) - _background(x, y);
	}
	const Background<float>&	background() const { return _background; }
	void	background(const BackgroundType& background) {
		_background = background;
	}
	bool	gradient() const { return _background.gradient(); }
	void	gradient(bool gradient) { _background.gradient(gradient); }
	double	scalefactor() const { return _background.scalefactor(); }
	void	scalefactor(double scalefactor) {
		_background.scalefactor(scalefactor);
	}
};

/**
 * \brief Create an image for the background
 */
template<typename BgPixel, typename Pixel>
class BackgroundImageAdapter : public ConstImageAdapter<RGB<Pixel> > {
private:
	Background<float>	_background;
	RGB<Pixel>	min;
	double	scale;
public:
	BackgroundImageAdapter(const ImageSize& size,
		const Background<float>& background)
		: ConstImageAdapter<RGB<Pixel> >(size),
		  _background(background) {
		std::vector<BgPixel>	values;
		values.push_back(_background(size.lowerleft()).max());
		values.push_back(_background(size.upperleft()).max());
		values.push_back(_background(size.lowerright()).max());
		values.push_back(_background(size.upperright()).max());
		BgPixel	maxvalue = *max_element(values.begin(), values.end());
		values.clear();
		values.push_back(_background(size.lowerleft()).min());
		values.push_back(_background(size.upperleft()).min());
		values.push_back(_background(size.lowerright()).min());
		values.push_back(_background(size.upperright()).min());
		BgPixel	minvalue = *min_element(values.begin(), values.end());
		BgPixel	delta = maxvalue - minvalue;
		if (delta == 0) {
			scale = 0;
		} else {
			scale = RGB<Pixel>::limit / delta;
		}
		min = RGB<Pixel>(minvalue);
	}
	virtual const RGB<Pixel>	pixel(unsigned int x, unsigned int y)
		const {
		return (_background(ImagePoint(x, y)) - min) * scale;
	}
};

} // namespace image
} // namespace astro

#endif /* _AstroBackground_h */
