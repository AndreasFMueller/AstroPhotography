/*
 * AstroTransform.h -- geometric transformation of images
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroTransform_h
#define _AstroTransform_h

#include <AstroTypes.h>
#include <AstroImage.h>
#include <AstroFormat.h>
#include <AstroAdapter.h>
#include <set>

namespace astro {
namespace image {
namespace transform {

/**
 * \brief Base class for the Adapters that move and interpolate images
 */
template<typename Pixel>
class OffsetAdapter : public ConstImageAdapter<Pixel> {
protected:
	const ConstImageAdapter<Pixel>& _image;
	const ConstImageAdapter<Pixel>	*_raw;
	Point	_translation;
	ImagePoint	_t;
	double	_weights[4];
public:
	OffsetAdapter(const ConstImageAdapter<Pixel>& image,
		const ConstImageAdapter<Pixel> *raw, const Point& translation);
	virtual ~OffsetAdapter() {
		delete _raw;
	}
	virtual Pixel	pixel(int x, int y) const;
};

template<typename Pixel>
OffsetAdapter<Pixel>::OffsetAdapter(const ConstImageAdapter<Pixel>& image,
	const ConstImageAdapter<Pixel> *raw, const Point& translation)
	: ConstImageAdapter<Pixel>(image.getSize()),
	  _image(image), _raw(raw), _translation(translation),
	  _t(translation.x(), translation.y()) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"create offset adapter with offset tx = %f/%d, ty = %f/%d",
		_translation.x(), _t.x(), _translation.y(), _t.y());
	double	wx = _translation.x() - _t.x();
	double	wy = _translation.y() - _t.y();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "wx = %f, wy = %f", wx, wy);
	// compute the weights
	_weights[0] = wx * wy;
	_weights[1] = (1 - wx) * wy;
	_weights[2] = wx * (1 - wy);
	_weights[3] = (1 - wx) * (1 - wy);
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"w[0] = %f, w[1] = %f, w[2] = %f, w[3] = %f",
		_weights[0], _weights[1], _weights[2], _weights[3]);
}

template<typename Pixel>
Pixel	OffsetAdapter<Pixel>::pixel(int x, int y) const {
	Pixel	a[4];
	a[0] = _raw->pixel(-_t.x() + x - 1, -_t.y() + y - 1);
	a[1] = _raw->pixel(-_t.x() + x    , -_t.y() + y - 1);
	a[2] = _raw->pixel(-_t.x() + x - 1, -_t.y() + y    );
	a[3] = _raw->pixel(-_t.x() + x    , -_t.y() + y    );
	return weighted_sum(4, _weights, a);
}

/**
 * \brief Adapter that rolls an image
 */
template<typename Pixel>
class RollAdapter : public OffsetAdapter<Pixel> {
public:
	RollAdapter(const ConstImageAdapter<Pixel>& image,
		const Point& translation)
		: OffsetAdapter<Pixel>(image,
			new adapter::RollAdapter<Pixel>(image,
				ImagePoint(translation.x(), translation.y())),
			translation) {
	}
};

/**
 * \brief A translation adapter applies a translation to an image
 */
template<typename Pixel>
class TranslationAdapter : public ConstImageAdapter<Pixel> {
	const ConstImageAdapter<Pixel>& image;
	Point	translation;
	int	tx, ty;
	double	weights[4];
public:
	TranslationAdapter(const ConstImageAdapter<Pixel>& image,
		const Point& translation);
	virtual Pixel	pixel(int x, int y) const;
};

template<typename Pixel>
TranslationAdapter<Pixel>::TranslationAdapter(
	const ConstImageAdapter<Pixel>& _image, const Point& _translation) 
	: ConstImageAdapter<Pixel>(_image.getSize()),
	  image(_image), translation(_translation) {
	tx = floor(translation.x());
	ty = floor(translation.y());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tx = %d, ty = %d", tx, ty);
	double	wx = translation.x() - tx;
	double	wy = translation.y() - ty;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "wx = %f, wy = %f", wx, wy);
	// compute the weights
	weights[0] = wx * wy;
	weights[1] = (1 - wx) * wy;
	weights[2] = wx * (1 - wy);
	weights[3] = (1 - wx) * (1 - wy);
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"w[0] = %f, w[1] = %f, w[2] = %f, w[3] = %f",
		weights[0], weights[1], weights[2], weights[3]);
}

template<typename Pixel>
Pixel	TranslationAdapter<Pixel>::pixel(int x, int y) const {
	Pixel	a[4];
	ImageSize	size = ConstImageAdapter<Pixel>::getSize();
	// lower left corner
	if (size.contains(-tx + x - 1, -ty + y - 1)) {
		a[0] = image.pixel(-tx + x - 1, -ty + y - 1);
	} else {
		a[0] = Pixel(0);
	}
	// lower right corner
	if (size.contains(-tx + x    , -ty + y - 1)) {
		a[1] = image.pixel(-tx + x    , -ty + y - 1);
	} else {
		a[1] = Pixel(0);
	}
	// upper left corner
	if (size.contains(-tx + x - 1, -ty + y    )) {
		a[2] = image.pixel(-tx + x - 1, -ty + y    );
	} else {
		a[2] = Pixel(0);
	}
	// upper right corner
	if (size.contains(-tx + x    , -ty + y    )) {
		a[3] = image.pixel(-tx + x    , -ty + y    );
	} else {
		a[3] = Pixel(0);
	}
	return weighted_sum(4, weights, a);
}

ImagePtr	translate(ImagePtr source, const Point& translation);

/**
 * \brief Adapter to interpolate pixels
 *
 * If the pixel type allows NaNs, then pixels that are mapped outside the
 * original image are given NaN values. This allows e.g. the Analyzer to
 * to detect when there is no data to compute a residual.
 */
template<typename Pixel>
class PixelInterpolationAdapter : public ConstImageAdapter<Pixel> {
	const ConstImageAdapter<Pixel>&	image;
	Pixel	defaultpixel;
public:
	PixelInterpolationAdapter(const ConstImageAdapter<Pixel>& _image)
		: ConstImageAdapter<Pixel>(_image.getSize()), image(_image) {
		if (std::numeric_limits<Pixel>::has_quiet_NaN) {
			defaultpixel = std::numeric_limits<Pixel>::quiet_NaN();
		} else {
			defaultpixel = Pixel(0);
		}
	}
	virtual Pixel	pixel(int x, int y) const {
		return image.pixel(x, y);
	}
	Pixel	pixel(const astro::Point& t) const {
		// find out in which pixel this is located
		int     tx = floor(t.x());
		int     ty = floor(t.y());

		// compute the weights
		double  wx = t.x() - tx;
		double  wy = t.y() - ty;

		// compute the weights
		double  weights[4];
		weights[0] = (1 - wx) * (1 - wy);
		weights[1] = wx * (1 - wy);
		weights[2] = (1 - wx) * wy;
		weights[3] = wx * wy;

		// now compute the weighted sum of the pixels
		Pixel   a[4];
		ImageSize       size = ConstImageAdapter<Pixel>::getSize();

		// lower left corner
		if (size.contains(tx    , ty    )) {
			a[0] = image.pixel(tx    , ty    );
		} else {
			a[0] = defaultpixel;
		}
		// lower right corner
		if (size.contains(tx + 1, ty    )) {
			a[1] = image.pixel(tx + 1, ty    );
		} else {
			a[1] = defaultpixel;
		}
		// upper left corner
		if (size.contains(tx    , ty + 1)) {
			a[2] = image.pixel(tx    , ty + 1);
		} else {
			a[2] = defaultpixel;
		}
		// upper right corner
		if (size.contains(tx + 1, ty + 1)) {
			a[3] = image.pixel(tx + 1, ty + 1);
		} else {
			a[3] = defaultpixel;
		}
		return weighted_sum(4, weights, a);
	}
};

/**
 * \brief Residuals needed to analyze transforms
 */
class Residual : public std::pair<ImagePoint, Point> {
	ImagePoint	_from;
	Point	_offset;
	double	_weight;
public:
	Residual(const ImagePoint& from, const Point& offset,
		double weight = 1.)
		: _from(from), _offset(offset), _weight(weight) {
	}
	const ImagePoint&	from() const { return _from; }
	ImagePoint&	from() { return _from; }
	const Point&	offset() const { return _offset; }
	Point&	offset() { return _offset; }
	const double&	weight() const { return _weight; }
	double&	weight() { return _weight; }
	bool	invalid() const;
	bool	valid() const { return !invalid(); }
	operator	std::string() const;
};

/**
 * \brief Abstraction of an affine transform
 */

class Transform {
	double	a[6];
	void	identity();
	void	translation(const std::vector<Residual>& residuals);
	void	build(const std::vector<Point>& from,
			const std::vector<Point>& to,
			const std::vector<double>& weight);
public:
	// constructors
	Transform();
	Transform(const Transform& other);
	Transform(double angle, const Point& translation,
		double scalefactor = 1);
	Transform(const std::vector<Residual>& residuals);
	Transform(const std::vector<Point>& from,
		const std::vector<Point>& to);
	Transform(const std::vector<Point>& from,
		const std::vector<Point>& to,
		const std::vector<double>& weights);

	// return some quality measure for how far away from an aspect
	// preserving 
	double	skew() const;

	// check whether this is a certain type of transform
	bool	isIdentity() const;
	bool	isTranslation() const;
	bool	isRotation() const;
	bool	isHomothety() const;
	bool	isIsometry() const;
	bool	isAreaPreserving() const;
	bool	isAnglePreserving() const;
	bool	fixesOrigin() const;
	bool	isAspectPreserving() const;
	bool	operator==(const Transform& other) const;
	bool	operator!=(const Transform& other) const;

	// compute the inverse transformation
	Transform	inverse() const;

	// data access
	Point	getTranslation() const;

	// how far away from the identity is the transform
	double	discrepancy(const ImageSize& size) const;

	// operations
	Transform	operator*(const Transform& other) const;
	Transform	operator+(const Point& translation) const;
	Transform	operator+(const astro::image::ImagePoint& translation)
				const;

	// access to the coefficients
	virtual double	operator[](int i) const;
	virtual double&	operator[](int i);

	// operating on points 
	virtual Point	operator()(const Point& point) const;

	// for debugging
	virtual std::string	toString() const;
	friend std::ostream&	operator<<(std::ostream& out,
		const Transform& transform);
};

/**
 * \brief
 */
template<typename Pixel>
class TransformAdapter : public ConstImageAdapter<Pixel> {
	const PixelInterpolationAdapter<Pixel>	image;
	Transform	transform;
	Transform	inverse;
public:
	TransformAdapter(const ImageSize& targetsize,
		const ConstImageAdapter<Pixel>& image,
		const Transform& transform);
	TransformAdapter(const ConstImageAdapter<Pixel>& image,
		const Transform& transform);
	virtual Pixel	pixel(int x, int y) const;
};

template<typename Pixel>
TransformAdapter<Pixel>::TransformAdapter(const ImageSize& targetsize,
	const ConstImageAdapter<Pixel>& _image, const Transform& _transform)
	: ConstImageAdapter<Pixel>(targetsize), image(_image),
	  transform(_transform) {
	inverse = transform.inverse();
}

template<typename Pixel>
TransformAdapter<Pixel>::TransformAdapter(
	const ConstImageAdapter<Pixel>& _image, const Transform& _transform)
	: ConstImageAdapter<Pixel>(_image.getSize()),
	  image(_image), transform(_transform) {
	inverse = transform.inverse();
}

template<typename Pixel>
Pixel	TransformAdapter<Pixel>::pixel(int x, int y) const {
	// compute the image if the point x, y under the inverse transform
	Point	t = inverse(Point(x, y));
	return image.pixel(t);
}

ImagePtr	transform(ImagePtr image, const Transform& transform);

/**
 * \brief Find a translation between two images
 *
 * This method uses Fourier transform and phase correlation to find the
 * (necessarily small) translation with subpixel accuracy.
 */
class PhaseCorrelator {
	void	write(const Image<double>& image);
	bool	_hanning;
public:
	bool	hanning() const { return _hanning; }
	void	hanning(bool h) { _hanning = h; }
private:
	std::string	_imagedir;
public:
	const std::string&	imagedir() const { return _imagedir; }
	void	imagedir(const std::string& i) { _imagedir = i; }
private:
	std::string	_prefix;
public:
	const std::string&	prefix() const { return _prefix; }
	void	prefix(const std::string& p) { _prefix = p; }
public:
	PhaseCorrelator(bool hanning = true) : _hanning(hanning),
		_imagedir("tmp"), _prefix("corr") {
	}
	virtual std::pair<Point, double>	operator()(
		const ConstImageAdapter<double>& fromimage,
		const ConstImageAdapter<double>& toimage);
};

/**
 * \brief Phase correlator using the derivative of an image
 *
 * Correlation is not precise if there are no features. Taking the
 * derivative of an image creates such features, but also increases
 * the noise level.
 */
template<typename Adapter>
class DerivedPhaseCorrelator : public PhaseCorrelator {
public:
	DerivedPhaseCorrelator(bool hanning = true)
		: PhaseCorrelator(hanning) { }
	virtual std::pair<Point, double>	operator()(
		const ConstImageAdapter<double>& fromimage,
		const ConstImageAdapter<double>& toimage) {
		Adapter	from(fromimage);
		Adapter	to(toimage);
		return PhaseCorrelator::operator()(from, to);
	}
};

/**
 * \brief Analysis of a transformation and get a list of 
 */
class Analyzer {
	const ConstImageAdapter<double>& _baseimage;
	int	_spacing;
public:
	int	spacing() const { return _spacing; }
	void	spacing(int s) { _spacing = s; }
private:
	int	_patchsize;
public:
	int	patchsize() const { return _patchsize; }
	void	patchsize(int p) { _patchsize = p; }
private:
	bool	_hanning;
public:
	bool	hanning() const { return _hanning; }
	void	hanning(bool h) { _hanning = h; }
public:
	Analyzer(const ConstImageAdapter<double>& baseimage,
		int spacing = 128, int patchsize = 128);
	Residual	translation(const ConstImageAdapter<double>& image,
		const ImagePoint& where, int patchsize) const;
        std::vector<Residual>	operator()(const ConstImageAdapter<double>& image) const;
};

/**
 * \brief Find a general transformation between
 */
class TransformAnalyzer : public Analyzer {
	Transform	basetransform;
public:
	TransformAnalyzer(const ConstImageAdapter<double>& _baseimage,
		int _spacing = 128, int _patchsize = 128)
		: Analyzer(_baseimage, _spacing, _patchsize) { }
	Transform	transform(const ConstImageAdapter<double>& image) const;
};

/**
 * \brief Triangle data structure for the triangle matching algorithm
 */
class Triangle {
	std::vector<Point>	points;
	double	_longside;
	double	_middleside;
	double	_angle;
	double	_azimut;
	double	_area;
public:
	Triangle(Point p1, Point p2, Point p3);
	Triangle(const std::vector<Point>);
	const Point&	operator[](int i) const { return points[i]; }

	// attributes of triangles
	double	longside() const { return _longside; }
	double	middleside() const { return _middleside; }
	double	angle() const { return _angle; }
	double	azimut() const { return _azimut; }
	double	area() const { return _area; }

	// find similar triangles
	bool	operator<(const Triangle& other) const;
	double	distance(const Triangle& other) const;

	// find transformation between triangles
	bool	mirror_to(const Triangle& other) const;
	double	rotate_to(const Triangle& other) const;
	double	scale_to(const Triangle& other) const;
	Point	basepoint() const { return points[0]; }
	Transform	to(const Triangle& other) const;

	// display
	std::string	toString() const;
	operator std::string() const;
};

/**
 * \brief A set of triangles
 *
 * Triangle sets are used for the triangle matching algorithm.
 */
class TriangleSet : public std::set<Triangle> {
	double	_tolerance;
public:
	double	tolerance() const { return _tolerance; }
	void	tolerance(double t) { _tolerance = t; }
private:
	bool	_allow_mirror;
public:
	bool	allow_mirror() const { return _allow_mirror; }
	void	allow_mirror(bool b) { _allow_mirror = b; }

	TriangleSet();
	const Triangle&	closest(const Triangle& other) const;
	Transform	closest(const TriangleSet& other) const;
};

/*
 * \brief Star abstraction for transforms
 *
 * To determine transforms, we need an abstraction 
 */
class Star : public Point {
	double	_brightness;
public:	
	double	brightness() const { return _brightness; }
	void	brightness(double b) { _brightness = b; }
	Star(const Point& p, double b = 1.) : Point(p), _brightness(b) { }
	bool	operator<(const Star& other) const;
	std::string	toString() const;
	operator	std::string() const;
};

/**
 * \brief Star extractor class
 */
class StarExtractor {
	int	_numberofstars;
public:
	int	numberofstars() const { return _numberofstars; }
	void	numberofstars(int n) { _numberofstars = n; }
private:
	int	_searchradius;
public:
	int	searchradius() const { return _searchradius; }
	void	searchradius(int s) { _searchradius = s; }
public:
	StarExtractor(int numberofstars = 10, int searchradius = 10)
		: _numberofstars(numberofstars), _searchradius(searchradius) { }
	StarExtractor(const StarExtractor& other)
		: _numberofstars(other._numberofstars),
		  _searchradius(other._searchradius) { }
protected:
static std::vector<Point>	stars2points(const std::vector<Star>& stars);
public:
	std::vector<Star> 	stars(const ConstImageAdapter<double>& image) const;
	std::vector<Point>  	points(const ConstImageAdapter<double>& image) const;
	std::vector<Star>	stars(ImagePtr image) const;
	std::vector<Point>	points(ImagePtr image) const;
};

template<typename T>
class TypedStarExtractor : public StarExtractor {
public:
	TypedStarExtractor(int numberofstars = 10, int searchradius = 10)
		: StarExtractor(numberofstars, searchradius) { }
	TypedStarExtractor(const StarExtractor& other)
		: StarExtractor(other) { }
	std::vector<Star>	stars(const ConstImageAdapter<T>& image) const;
	std::vector<Point>	points(const ConstImageAdapter<T>& image) const;
};

template<typename T>
std::vector<Star>	TypedStarExtractor<T>::stars(const ConstImageAdapter<T>& image) const {
	adapter::LuminanceAdapter<T, double>	luminanceadapter(image);
	return StarExtractor::stars(luminanceadapter);
}

template<typename T>
std::vector<Point>	TypedStarExtractor<T>::points(const ConstImageAdapter<T>& image) const {
	adapter::LuminanceAdapter<T, double>	luminanceadapter(image);
	return StarExtractor::points(luminanceadapter);
}

/**
 * \brief Extract a set of triangles from an image
 */
class TriangleSetFactory {
	int	_numberofstars;
public:
	int	numberofstars() const { return _numberofstars; }
	void	numberofstars(int n) { _numberofstars = n; }
private:
	double	_radius;
public:
	double	radius() const { return _radius; }
	void	radius(double r) { _radius = r; }

private:
	bool	good(const Triangle& t, double l) const;
public:
	TriangleSetFactory();
private:
	TriangleSet	get(const std::vector<Star>& stars, double limit) const;
public:
	TriangleSet	get(ImagePtr) const;
	TriangleSet	get(const ConstImageAdapter<double>& image) const;
};

/**
 * \brief Extract a transform using the triangle method
 */
class TriangleAnalyzer {
	TriangleSetFactory	factory;
	TriangleSet	fromtriangles;
public:
	TriangleAnalyzer(const ConstImageAdapter<double>& image,
		int numberofstars, int searchradius);
	TriangleAnalyzer(ImagePtr image,
		int numberofstars, int searchradius);
	Transform	transform(const ConstImageAdapter<double>& image) const;
	Transform	transform(ImagePtr image) const;
};

} // namespace transform
} // namespace image
} // namespace astro

#endif /* _AstroTransform_h */
