/*
 * AstroBackground.h
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
 * \brief Common base class for all background functions
 */
class FunctionBase {
	bool	_gradient;
public:
	bool	gradient() const { return _gradient; }
	void	gradient(bool gradient) { _gradient = gradient; }

private:
	bool	_symmetric;
public:
	bool	symmetric() const { return _symmetric; }
	void	symmetric(bool symmetric) { _symmetric = symmetric; }

private:
	double	_scalefactor;
public:
	double	scalefactor() const { return _scalefactor; }
	void	scalefactor(double scalefactor) { _scalefactor = scalefactor; }

protected:
	ImagePoint	_center;
public:
	ImagePoint	center() const { return _center; }

public:
	typedef std::pair<Point, double>	doublevaluepair;

	FunctionBase(const ImagePoint& center, bool symmetric)
		: _symmetric(symmetric), _center(center) {
		_gradient = true;
		_scalefactor = 1.;
	}
	FunctionBase(const FunctionBase& other);
	virtual double	evaluate(const Point& point) const = 0;
	virtual std::string	toString() const;
};

/**
 * \brief Gradients are described by linear functions on the image coordinates
 */
class LinearFunctionBase : public FunctionBase {
	double	a[3];
protected:
	void	reduce(const std::vector<doublevaluepair>& values);
public:
	// standard constructors
	LinearFunctionBase(const ImagePoint& point, bool symmetric);
	LinearFunctionBase(const LinearFunctionBase& other);

	// constructor from data set
	LinearFunctionBase(const ImagePoint& center, bool symmetric,
		const std::vector<doublevaluepair>& values);

	// evaluation
	virtual double	evaluate(const Point& point) const;
	double	norm() const;

	// coefficient access
	virtual double	operator[](int i) const;
	virtual double&	operator[](int i);

	// linear operators
	LinearFunctionBase	operator+(const LinearFunctionBase& other);
	LinearFunctionBase&	operator=(const LinearFunctionBase& other);
	
	// text representation
	virtual std::string	toString() const;
};

/**
 * \brief Quadratic background function
 */
class QuadraticFunctionBase : public LinearFunctionBase {
	double	q[3];
protected:
	void	reduce(const std::vector<doublevaluepair>& values);
public:
	QuadraticFunctionBase(const ImagePoint& center, bool symmetric);
	QuadraticFunctionBase(const LinearFunctionBase& linear);
	virtual double	evaluate(const Point& point) const;
	virtual double	operator[](int i) const;
	virtual double&	operator[](int i);
	QuadraticFunctionBase	operator+(const QuadraticFunctionBase& other);
	QuadraticFunctionBase	operator+(const LinearFunctionBase& other);
	QuadraticFunctionBase&	operator=(const QuadraticFunctionBase& other);
	QuadraticFunctionBase&	operator=(const LinearFunctionBase& other);
	virtual std::string	toString() const;
};


/**
 * \brief Linear functions with complex pixel values
 */
template<typename Pixel>
class LinearFunction : public LinearFunctionBase {
public:
	typedef std::pair<Point, Pixel>	valuepair;
	typedef std::vector<valuepair>	values_type;

	LinearFunction() : LinearFunctionBase(ImagePoint(), true) { }
	LinearFunction(const ImagePoint& center, bool symmetric)
		: LinearFunctionBase(center, symmetric) { }
	LinearFunction(const LinearFunctionBase& other)
		: LinearFunctionBase(other) { }

	LinearFunction(const values_type& values) {
		std::vector<doublevaluepair>	converted;
		typename std::vector<valuepair>::const_iterator	i;
		for (i = values.begin(); i != values.end(); i++) {
			double	v = i->second;
			converted.push_back(std::make_pair(i->first, v));
		}
		reduce(converted);
	}

	virtual Pixel	operator()(const Point& point) const {
		Pixel	result = evaluate(point);
		return result;
	}

	virtual Pixel	operator()(const ImagePoint& point) const {
		return operator()(Point(point));
	}

	virtual Pixel	operator()(unsigned int x, unsigned int y) const {
		return operator()(Point(x, y));
	}

	LinearFunction<Pixel>	operator+(const LinearFunction<Pixel>& other) {
		return LinearFunction<Pixel>(LinearFunctionBase::operator+(other));
	}

	LinearFunction<Pixel>&	operator=(const LinearFunction<Pixel>& other) {
		LinearFunctionBase::operator=(other);
		return *this;
	}
};

template<typename Pixel>
class QuadraticFunction : public QuadraticFunctionBase {
public:
	typedef std::pair<Point, Pixel>	valuepair;
	typedef std::vector<valuepair>	values_type;

	QuadraticFunction(const ImagePoint& center, bool symmetric)
		: QuadraticFunctionBase(center, symmetric) {
	}

	QuadraticFunction(const values_type& values) {
		std::vector<doublevaluepair>	converted;
		typename std::vector<valuepair>::const_iterator	i;
		for (i = values.begin(); i != values.end(); i++) {
			double	v = i->second;
			converted.push_back(std::make_pair(i->first, v));
		}
		reduce(converted);
	}

	virtual Pixel	operator()(const Point& point) const {
		Pixel	result = evaluate(point);
		return result;
	}

	virtual Pixel	operator()(const ImagePoint& point) const {
		return operator()(Point(point));
	}

	virtual Pixel	operator()(unsigned int x, unsigned int y) const {
		return operator()(Point(x, y));
	}

	QuadraticFunction<Pixel>	operator+(
		const QuadraticFunction<Pixel>& other) {
		return QuadraticFunction<Pixel>(
			QuadraticFunctionBase::operator+(other));
	}

	QuadraticFunction<Pixel>&	operator=(
		const QuadraticFunction<Pixel>& other) {
		QuadraticFunctionBase::operator=(other);
		return *this;
	}
};

/**
 * \brief Background base
 */
template<typename Pixel>
class BackgroundBase {
public:
	virtual bool	gradient() const = 0;
	virtual void	gradient(bool gradient) = 0;
	virtual double	scalefactor() const = 0;
	virtual void	scalefactor(double scalefactor) = 0;
	virtual RGB<Pixel>	operator()(const Point& point) const = 0;
	virtual RGB<Pixel>	operator()(const ImagePoint& point) const = 0;
	virtual RGB<Pixel>	operator()(unsigned int x, unsigned int y) const = 0;
};

/**
 * \brief Describing a background with a gradient
 */
template<typename Pixel>
class Background : public BackgroundBase<Pixel> {
	LinearFunction<Pixel>	R;
	LinearFunction<Pixel>	G;
	LinearFunction<Pixel>	B;
public:
	Background() { }
	Background(const LinearFunction<Pixel>& _R,
		   const LinearFunction<Pixel>& _G,
		   const LinearFunction<Pixel>& _B) : R(_R), G(_G), B(_B) { }
	bool	gradient() const { return R.gradient(); }
	void	gradient(bool gradient) {
		R.gradient(gradient);
		G.gradient(gradient);
		B.gradient(gradient);
	}
	RGB<Pixel>	operator()(const Point& point) const {
		return RGB<Pixel>(R(point), G(point), B(point));
	}
	RGB<Pixel>	operator()(const ImagePoint& point) const {
		return operator()(Point(point));
	}
	RGB<Pixel>	operator()(unsigned int x, unsigned int y) const {
		return operator()(Point(x, y));
	}
	double	scalefactor() const { return R.scalefactor(); }
	void	scalefactor(double scalefactor) { 
		R.scalefactor(scalefactor);
		G.scalefactor(scalefactor);
		B.scalefactor(scalefactor);
	}
};

/**
 * \brief Minimum estimator
 */
class	MinimumEstimator {
	unsigned int	alpha;
public:
	MinimumEstimator(unsigned int _alpha) : alpha(_alpha) { }
	LinearFunction<float>	linearfunction(const ImagePoint& center,
		bool symmetric,
		const ConstImageAdapter<float>& values) const;
	QuadraticFunction<float> quadraticfunction(const ImagePoint& center,
		bool symmetric,
		const ConstImageAdapter<float>& values) const;
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
	BackgroundExtractor(unsigned int _alpha) : alpha(_alpha) { }
	Background<float>	operator()(const ImagePoint& center, bool symmetric, const Image<RGB<float> >& image) const;
	Background<float>	operator()(const ImagePoint& center, bool symmetric, const Image<float>& image) const;
};

/**
 * \brief Backgroud Subtraction
 *
 * We implement this as an adapter
 */
class BackgroundSubtractionAdapter : public ConstImageAdapter<RGB<float> > {
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
	void	background(const Background<float>& background) {
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
	Background<BgPixel>	_background;
	RGB<Pixel>	min;
	double	scale;
public:
	BackgroundImageAdapter(const ImageSize& size,
		const Background<BgPixel>& background)
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
