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

namespace astro {
namespace image {

/**
 * \brief Gradients are described by linear functions on the image coordinates
 */
class LinearFunctionBase {
	double	a[3];
public:
	typedef std::pair<Point, double>	doublevaluepair;
protected:
	void	reduce(const std::vector<doublevaluepair>& values);
public:
	// standard constructors
	LinearFunctionBase(double alpha = 0., double beta = 0., double gamma = 0.);
	LinearFunctionBase(const LinearFunctionBase& other);

	// constructor from data set
	LinearFunctionBase(const std::vector<doublevaluepair>& values);

	// evaluation
	double	evaluate(const Point& point) const;
	double	norm(const ImageSize& size) const;

	// coefficient access
	double	operator[](int i) const;
	double&	operator[](int i);

	// linear operators
	LinearFunctionBase	operator+(const LinearFunctionBase& other);
	LinearFunctionBase&	operator=(const LinearFunctionBase& other);
	
	// text representation
	std::string	toString() const;
};

/**
 * \brief Linear functions with complex pixel values
 */
template<typename Pixel>
class LinearFunction : public LinearFunctionBase {
public:
	typedef std::pair<Point, Pixel>	valuepair;
	typedef std::vector<valuepair>	values_type;

	LinearFunction(double alpha = 0, double beta = 0, double gamma = 0)
		: LinearFunctionBase(alpha, beta, gamma) { }
	LinearFunction(const LinearFunctionBase& other) : LinearFunctionBase(other) { }

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

/**
 * \brief Describing a background with a gradient
 */
template<typename Pixel>
class Background {
	LinearFunction<Pixel>	R;
	LinearFunction<Pixel>	G;
	LinearFunction<Pixel>	B;
public:
	Background() { }
	Background(const LinearFunction<Pixel>& _R,
		   const LinearFunction<Pixel>& _G,
		   const LinearFunction<Pixel>& _B) : R(_R), G(_G), B(_B) { }
	RGB<Pixel>	operator()(const Point& point) const {
		return RGB<Pixel>(R(point), G(point), B(point));
	}
	RGB<Pixel>	operator()(const ImagePoint& point) const {
		return operator()(Point(point));
	}
	RGB<Pixel>	operator()(unsigned int x, unsigned int y) const {
		return operator()(Point(x, y));
	}
};

/**
 * \brief Minimum estimator
 */
class	MinimumEstimator {
	unsigned int	alpha;
public:
	MinimumEstimator(unsigned int _alpha) : alpha(_alpha) { }
	LinearFunction<float>	operator()(const ConstImageAdapter<float>& values) const;
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
	Background<float>	operator()(const Image<RGB<float> >& image) const;
	Background<float>	operator()(const Image<float>& image) const;
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
	virtual const RGB<float>	pixel(unsigned int x, unsigned int y) const {
		return _image.pixel(x, y) - _background(x, y);
	}
};

} // namespace image
} // namespace astro

#endif /* _AstroBackground_h */
