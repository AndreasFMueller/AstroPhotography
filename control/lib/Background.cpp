/*
 * Background.cpp -- algorithms to extract a background gradient from an image
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroBackground.h>
#include <AstroFormat.h>
#include <AstroFilter.h>
#include <AstroUtils.h>
#include <glpk.h>
#include <math.h>
#include <set>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_ACCELERATE_ACCELERATE_H
#include <Accelerate/Accelerate.h>
#else
#include <lapack.h>
#endif /* HAVE_ACCELERATE_ACCELERATE_H */

using namespace astro::image;
using namespace astro::image::filter;

namespace astro {
namespace adapter {

//////////////////////////////////////////////////////////////////////
// FunctionBase implementation
//////////////////////////////////////////////////////////////////////

FunctionBase::FunctionBase(const FunctionBase& other) :
	_symmetric(other.symmetric()), _center(other.center()) {
	_gradient = other.gradient();
	_scalefactor = other.scalefactor();
}

std::string	FunctionBase::toString() const {
	return stringprintf("[gradient=%s,symmetric=%s,scalefactor=%.3f]",
		(gradient()) ? "YES" : "NO", (symmetric()) ? "YES" : "NO",
		scalefactor());
}

//////////////////////////////////////////////////////////////////////
// LinearFunctionBase implementation
//////////////////////////////////////////////////////////////////////

LinearFunctionBase::LinearFunctionBase(const ImagePoint& point, bool symmetric)
	: FunctionBase(point, symmetric) {
	for (int i = 0; i < 3; i++) { a[i] = 0; }
}

LinearFunctionBase::LinearFunctionBase(const LinearFunctionBase& other) :
	FunctionBase(other) {
	for (int i = 0; i < 3; i++) { a[i] = other.a[i]; }
}

double  LinearFunctionBase::evaluate(const Point& point) const {
	double	value = a[2];
	if (gradient() && symmetric()) {
		double	deltax = point.x() - center().x();
		double	deltay = point.y() - center().y();
		value += (deltax * a[0] + deltay * a[1]);
	}
	return scalefactor() * value;
}

inline static double	sqr(const double& x) {
	return x * x;
}

double	LinearFunctionBase::norm() const {
	double	result = 0;
	result += sqr(center().x() * a[0]);
	result += sqr(center().y() * a[1]);
	result += sqr(a[2]);
	return sqrt(result);
}

LinearFunctionBase      LinearFunctionBase::operator+(const LinearFunctionBase& other) {
	LinearFunctionBase      result(*this);
	for (unsigned int i = 0; i < 3; i++) {
		result.a[i] = a[i] + other.a[i];
	}
	return result;
}

LinearFunctionBase&     LinearFunctionBase::operator=(
	const LinearFunctionBase& other) {
	for (int i = 0; i < 3; i++) {
		a[i] = other.a[i];
	}
	return *this;
}

/**
 * \brief read only access to coefficients
 */
double	LinearFunctionBase::operator[](int i) const {
	if ((i < 0) || (i > 2)) {
		throw std::range_error("index out of range");
	}
	return a[i];
}

/**
 * \brief modifying access to coefficients
 */
double&	LinearFunctionBase::operator[](int i) {
	if ((i < 0) || (i > 2)) {
		throw std::range_error("index out of range");
	}
	return a[i];
}

/**
 * \brief Compute the best possible coefficients from a data set
 */
void	LinearFunctionBase::reduce(const std::vector<doublevaluepair>& values) {
	// build a linear system for coefficients a[3]
	int	m = values.size();
	int	n = 3;
	double	A[3 * m * n];
	double	b[m];
	int	line = 0;
	std::vector<doublevaluepair>::const_iterator	vp;
	for (vp = values.begin(); vp != values.end(); vp++, line++) {
		A[line + 0 * m] = vp->first.x() - center().x();
		A[line + 1 * m] = vp->first.y() - center().x();
		A[line + 2 * m] = 1;
		b[line] = vp->second;
	}

	// set up the lapack stuff
	char	trans = 'N';
	int	nrhs = 1;
	int	lda = m;
	int	ldb = m;
	int	lwork = -1;
	int	info = 0;

	// first find out how large we have to make the work area
	double	x;
	dgels_(&trans, &m, &n, &nrhs, A, &lda, b, &ldb, &x, &lwork, &info);
	if (info != 0) {
		std::string	msg = stringprintf("dgels cannot determine "
			"work area size: %d", info);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	lwork = x;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "work area size: %d", lwork);

	// now allocate memory and get the solution
	double	work[lwork];
	dgels_(&trans, &m, &n, &nrhs, A, &lda, b, &ldb, work, &lwork, &info);
	if (info != 0) {
		std::string	msg = stringprintf("dgels cannot solve "
			"equations: %d", info);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// copy the result vector
	for (int i = 0; i < 3; i++) {
		a[i] = b[i];
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Linear function found: %s",
		toString().c_str());
}

/**
 * \brief Create a linear function from a set of value pairs
 */
LinearFunctionBase::LinearFunctionBase(const ImagePoint& center, bool symmetric,
	const std::vector<LinearFunctionBase::doublevaluepair>& values)
	: FunctionBase(center, symmetric) {
	a[0] = a[1] = a[2] = 0;
	reduce(values);
}

/**
 * \brief Text representation of a linear form
 */
std::string	LinearFunctionBase::toString() const {
	return FunctionBase::toString()
		+ stringprintf("%f * dx + %f * dy + %f", a[0], a[1], a[2]);
}

//////////////////////////////////////////////////////////////////////
// Tile class
//////////////////////////////////////////////////////////////////////
class Tile : public ImageRectangle, public Point {
public:
	Tile(const ImageRectangle& rectangle) : ImageRectangle(rectangle),
		Point(rectangle.origin().x() + rectangle.size().width(),
		      rectangle.origin().y() + rectangle.size().height()) {
	}
	std::string	toString() const {
		return stringprintf("Tile %s, center %s", 
			ImageRectangle::toString().c_str(),
			Point::toString().c_str());
	}
};

typedef std::vector<Tile>	TileSet;

//////////////////////////////////////////////////////////////////////
// TileFactory class
//////////////////////////////////////////////////////////////////////
class TileFactory {
	ImageSize	_tilesize;
public:
	TileFactory(const ImageSize& tilesize) : _tilesize(tilesize) { }
	const ImageSize&	tilesize() const { return _tilesize; }
	TileSet	operator()(const ConstImageAdapter<float>& image) const;
};

TileSet	TileFactory::operator()(const ConstImageAdapter<float>& image) const {
	TileSet	result;

	// compute the tile sizes
	ImageSize	imagesize = image.getSize();
	unsigned int	htiles = imagesize.width() / _tilesize.width();
	if ((imagesize.width() - htiles * _tilesize.width()) < 20) {
		htiles -= 1;
	}
	unsigned int	vtiles = imagesize.height() / _tilesize.height();
	if ((imagesize.height() - vtiles * _tilesize.height()) < 20) {
		vtiles -= 1;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating %ux%u tiles", htiles, vtiles);

	// compute the origin of the tiles
	int	originx = (imagesize.width() - htiles * _tilesize.width()) / 2;
	int	originy = (imagesize.height() - vtiles * _tilesize.height()) / 2;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "origin of tile grid: (%d,%d)",
		originx, originy);

	// compute all tiles
	for (unsigned int x = 0; x < htiles; x++) {
		for (unsigned int y = 0; y < vtiles; y++) {
			ImagePoint	origin(
				originx + x * _tilesize.width(),
				originy + y * _tilesize.height());
			ImageRectangle	rectangle(origin, _tilesize);
			Tile	t(rectangle);
			//debug(LOG_DEBUG, DEBUG_LOG, 0, "new tile: %s",
			//	t.toString().c_str());
			result.push_back(t);
		}
	}
	return result;
}

//////////////////////////////////////////////////////////////////////
// Order statistics adapter
//////////////////////////////////////////////////////////////////////

template<typename T>
class OrderStatisticsFilter : public PixelTypeFilter<T, T> {
	unsigned int	_order;
public:
	OrderStatisticsFilter(unsigned int order) : _order(order) { }
        virtual T       filter(const ConstImageAdapter<T>& image);
        virtual T       operator()(const ConstImageAdapter<T>& image);
};

template<typename T>
T	OrderStatisticsFilter<T>::operator()(
		const ConstImageAdapter<T>& image) {
	Timer	timer;
	timer.start();
	if (image.getSize().getPixels() < _order) {
		throw std::range_error("not enough pixels in image");
	}
	unsigned int	width = image.getSize().width();
	unsigned int	height = image.getSize().height();
	// there are two ways one can implement this: either use an ordered
	// container or use and unordered container and sort later. It turns
	// out the using an ordered contained is about 70% slower
	//std::multiset<T>	v;
	std::vector<T>	v;
	for (unsigned int x = 0; x < width; x++) {
		for (unsigned int y = 0; y < height; y++) {
			//v.insert(image.pixel(x, y));
			v.push_back(image.pixel(x, y));
		}
	}
	//typename std::multiset<T>::const_iterator	vp = v.begin();
	//for (unsigned int n = 0; n < _order; n++) { vp++; }
	sort(v.begin(), v.end());
	timer.end();
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "order time: %.6f", timer.elapsed());
	//return *vp;
	return v[_order];
}

template<typename T>
T	OrderStatisticsFilter<T>::filter(const ConstImageAdapter<T>& image) {
	return operator()(image);
}

//////////////////////////////////////////////////////////////////////
// LinearFunction adapter
//////////////////////////////////////////////////////////////////////
class LinearFunctionAdapter : public ConstImageAdapter<float> {
	const ConstImageAdapter<float>&	_image;
	const LinearFunction<float>&	_linfunc;
	const ImagePoint	_origin;
public:
	LinearFunctionAdapter(const ConstImageAdapter<float>& image,
		const LinearFunction<float>& linfunc, const ImagePoint& origin)
		: ConstImageAdapter<float>(image.getSize()), _image(image),
		  _linfunc(linfunc), _origin(origin) {
	}
	virtual const float	pixel(unsigned int x, unsigned int y) const;
};

const float	LinearFunctionAdapter::pixel(unsigned int x, unsigned int y)
	const {
	return _image.pixel(x, y) - _linfunc(_origin.x() + x, _origin.y() + y);
}

//////////////////////////////////////////////////////////////////////
// QuadraticFunctionAdapter
//////////////////////////////////////////////////////////////////////
class QuadraticFunctionAdapter : public ConstImageAdapter<float> {
	const ConstImageAdapter<float>&	_image;
	const QuadraticFunction<float>&	_qfunc;
	const ImagePoint	_origin;
public:
	QuadraticFunctionAdapter(const ConstImageAdapter<float>& image,
		const QuadraticFunction<float>& qfunc,
		const ImagePoint& origin)
		: ConstImageAdapter<float>(image.getSize()), _image(image),
		  _qfunc(qfunc), _origin(origin) {
	}
	virtual const float	pixel(unsigned int x, unsigned int y) const;
};

const float	QuadraticFunctionAdapter::pixel(unsigned int x, unsigned int y)
	const {
	return _image.pixel(x, y) - _qfunc(_origin.x() + x, _origin.y() + y);
}

//////////////////////////////////////////////////////////////////////
// Optimization problem solution: the LowerBound class
//////////////////////////////////////////////////////////////////////
class LowerBound {
public:
	typedef	std::pair<Tile, float>	tilevalue;
	typedef std::vector<tilevalue>	tilevaluevector;
};

class LinearLowerBound : public LowerBound {
	LinearFunction<float>	symmetricfunction(const ImagePoint& center,
		const tilevaluevector& values) const;
	LinearFunction<float>	asymmetricfunction(const ImagePoint& center,
		const tilevaluevector& values) const;
public:
	LinearFunction<float>	operator()(const ImagePoint& center,
		bool symmetric, const tilevaluevector& values) const;
};

LinearFunction<float>	LinearLowerBound::symmetricfunction(
	const ImagePoint& center, const tilevaluevector& values) const {
	double	minimum = std::numeric_limits<double>::infinity();
	tilevaluevector::const_iterator	i;
	for (i = values.begin(); i != values.end(); i++) {
		if (i->second < minimum) {
			minimum = i->second;
		}
	}
	LinearFunction<float>	result(center, true);
	result[2] = minimum;
	return result;
}

LinearFunction<float>	LinearLowerBound::asymmetricfunction(
	const ImagePoint& center, const tilevaluevector& values) const {
	// create a problem
	glp_prob	*lp = glp_create_prob();
	glp_set_obj_dir(lp, GLP_MAX);

	// we have three columns for the three coefficients of the linear
	// function, and one constraint for each tile
	glp_add_cols(lp, 3);
	glp_set_col_name(lp, 1, "alpha");
	glp_set_col_bnds(lp, 1, GLP_DB, -10, 10);
	glp_set_col_name(lp, 2, "beta");
	glp_set_col_bnds(lp, 2, GLP_DB, -10, 10);
	glp_set_col_name(lp, 3, "gamma");
	glp_set_col_bnds(lp, 3, GLP_LO, 0, 0);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "columns set up");

	// got through all the rows
	glp_add_rows(lp, values.size());
	tilevaluevector::const_iterator	vp;
	unsigned int	row = 1;
	int	ind[4] = { 0, 1, 2, 3 };
	double	obj[3] = { 0., 0., values.size() };
	for (vp = values.begin(); vp != values.end(); vp++, row++) {
		// row name
		char	rowname[10];
		snprintf(rowname, sizeof(rowname), "s[%d]", row);
		glp_set_row_name(lp, row, rowname);

		// row coefficients and bounds
		glp_set_row_bnds(lp, row, GLP_UP, 0, vp->second);
		double	val[4];
		val[1] = vp->first.x() - center.x();
		val[2] = vp->first.y() - center.y();
		val[3] = 1;
		glp_set_mat_row(lp, row, 3, ind, val);

		// objective function
		obj[0] += vp->first.x() - center.x();
		obj[1] += vp->first.y() - center.y();
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rows set up");

	// objective function
	glp_set_obj_coef(lp, 1, obj[0]);
	glp_set_obj_coef(lp, 2, obj[1]);
	glp_set_obj_coef(lp, 3, obj[2]);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "objective function set up");

	// solve the linear problem
	glp_simplex(lp, NULL);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "simplex solution found");

	// cleanup the problem
	glp_delete_prob(lp);

	// return the result
	LinearFunction<float>	lb(center, false);
	lb[0] = glp_get_col_prim(lp, 1);
	lb[1] = glp_get_col_prim(lp, 2);
	lb[2] = glp_get_col_prim(lp, 3);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "linear function: %s",
		lb.toString().c_str());
	return lb;
}

LinearFunction<float>	LinearLowerBound::operator()(const ImagePoint& center,
	bool symmetric, const tilevaluevector& values)
	const {
	if (symmetric) {
		return symmetricfunction(center, values);
	} else {
		return asymmetricfunction(center, values);
	}
}


class QuadraticLowerBound : public LowerBound {
	QuadraticFunction<float>	symmetricfunction(
		const ImagePoint& center, const tilevaluevector& values) const;
	QuadraticFunction<float>	asymmetricfunction(
		const ImagePoint& center, const tilevaluevector& values) const;
public:
	QuadraticFunction<float>	operator()(const ImagePoint& center,
		bool symmetric, const tilevaluevector& values) const;
};

QuadraticFunction<float>	QuadraticLowerBound::symmetricfunction(
	const ImagePoint& center, const tilevaluevector& values) const {
	QuadraticFunction<float>	result(center, true);
	return result;
}

QuadraticFunction<float>	QuadraticLowerBound::asymmetricfunction(
	const ImagePoint& center, const tilevaluevector& values) const {
	QuadraticFunction<float>	result(center, false);
	return result;
}

QuadraticFunction<float>	QuadraticLowerBound::operator()(
	const ImagePoint& center, bool symmetric, const tilevaluevector& values)
	const {
	if (symmetric) {
		return symmetricfunction(center, values);
	} else {
		return asymmetricfunction(center, values);
	}
}

/**
 * \brief Minimum Estimator implementation
 */
LinearFunction<float>	MinimumEstimator::linearfunction(
				const ImagePoint& center, bool symmetric,
				const ConstImageAdapter<float>& image) const {
	// construct a set of tiles
	TileFactory	tf(ImageSize(100, 100));
	TileSet	tileset = tf(image);

	// initialize the loop
	LinearFunction<float>	h(center, symmetric);
	float	delta = std::numeric_limits<float>::infinity();
	float	epsilon = 0.1;
	unsigned int	count = 0;
	while ((count < 10) && (delta > epsilon)) {
		LowerBound::tilevaluevector	tv;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "start new iteration %d", count);

		// compute the order statistics in a tile
		TileSet::const_iterator	tileiterator;
		for (tileiterator = tileset.begin();
			tileiterator != tileset.end();
			tileiterator++) {
			WindowAdapter<float>	wa(image, *tileiterator);
			LinearFunctionAdapter	la(wa, h, tileiterator->origin());
			OrderStatisticsFilter<float>	of(alpha);
			float	Z = of(la);
			tv.push_back(std::make_pair(*tileiterator, Z));
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "values computed");

		// set up the optimization problem
		LinearLowerBound	lb;
		LinearFunction<float>	hhat
			= lb(image.getSize().center(), symmetric, tv);

		// compute the improved lower bound function
		delta = hhat.norm();
		h = h + hhat;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "new lower bound: %s, delta = %f",
			h.toString().c_str(), delta);
	}

	// return
	return h;
}

/**
 * \brief Compute the background of an image
 */
Background<float> BackgroundExtractor::operator()(const ImagePoint& center,
			bool symmetric,
			const Image<RGB<float> >& image) const {
	// compute the lower bound for each color 
	MinimumEstimator	me(alpha);
	ColorRedAdapter<float>		redimage(image);
	ColorGreenAdapter<float>	greenimage(image);
	ColorBlueAdapter<float>		blueimage(image);
	
	LinearFunction<float>	R = me.linearfunction(center, symmetric, redimage);
	LinearFunction<float>	G = me.linearfunction(center, symmetric, greenimage);
	LinearFunction<float>	B = me.linearfunction(center, symmetric, blueimage);
	Background<float>	result(R, G, B);
	return result;
}

Background<float>	BackgroundExtractor::operator()(
				const ImagePoint& center, bool symmetric,
				const Image<float>& image) const {
	MinimumEstimator	me(alpha);
	LinearFunction<float>	l = me.linearfunction(center, symmetric, image);
	Background<float>	result(l, l, l);
	return result;
}

//////////////////////////////////////////////////////////////////////
// QuadraticFunctionBase implementation
//////////////////////////////////////////////////////////////////////

/**
 * \brief 
 */
QuadraticFunctionBase::QuadraticFunctionBase(const ImagePoint& center,
	bool symmetric) : LinearFunctionBase(center, symmetric) {
	for (unsigned int i = 0; i < 3; i++) {
		q[i] = 0;
	}
}

double	QuadraticFunctionBase::evaluate(const Point& point) const {
	double	value = LinearFunctionBase::evaluate(point);
	if (gradient()) {
		double	deltax = point.x() - center().x();
		double	deltay = point.y() - center().y();
		value += q[0] * (sqr(deltax) + sqr(deltay));
		if (!symmetric()) {
			value += q[1] * deltax * deltay
				+ q[2] * (sqr(deltax) - sqr(deltay));
		}
	}
	return value;
}

double	QuadraticFunctionBase::operator[](int i) const {
	if (i < 3) {
		return LinearFunctionBase::operator[](i);
	}
	if (i > 5) {
		std::runtime_error("index out of range");
	}
	return q[i - 3];
}

double&	QuadraticFunctionBase::operator[](int i) {
	if (i < 3) {
		return LinearFunctionBase::operator[](i);
	}
	if (i > 5) {
		std::runtime_error("index out of range");
	}
	return q[i - 3];
}

QuadraticFunctionBase	QuadraticFunctionBase::operator+(
	const QuadraticFunctionBase& other) {
	LinearFunctionBase::operator+(other);
	for (unsigned int i = 0; i < 3; i++) {
		q[i] += other.q[i];
	}
	return *this;
}

QuadraticFunctionBase	QuadraticFunctionBase::operator+(
	const LinearFunctionBase& other) {
	LinearFunctionBase::operator+(other);
	return *this;
}

QuadraticFunctionBase&	QuadraticFunctionBase::operator=(
	const QuadraticFunctionBase& other) {
	LinearFunctionBase::operator=(other);
	for (unsigned int i = 0; i < 3; i++) {
		q[i] = other.q[i];
	}
	return *this;
}

QuadraticFunctionBase&	QuadraticFunctionBase::operator=(
	const LinearFunctionBase& other) {
	LinearFunctionBase::operator=(other);
	return *this;
}

std::string	QuadraticFunctionBase::toString() const {
	return LinearFunctionBase::toString()
		+ stringprintf("[%.3f, %.3f, %.3f]", q[0], q[1], q[2]);
} 

} // namespace image
} // namespace astro
