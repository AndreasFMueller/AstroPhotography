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

double	FunctionBase::evaluate(const ImagePoint& point) const {
	return evaluate(Point(point.x(), point.y()));
}

double	FunctionBase::evaluate(unsigned int x, unsigned int y) const {
	return evaluate(Point(x, y));
}

double	FunctionBase::operator()(const Point& point) const {
	return evaluate(point);
}

double	FunctionBase::operator()(const ImagePoint& point) const {
	return evaluate(point);
}

double	FunctionBase::operator()(unsigned int x, unsigned int y) const {
	return evaluate(x, y);
}

std::string	FunctionBase::toString() const {
	return stringprintf("[gradient=%s,symmetric=%s,scalefactor=%.3f]",
		(gradient()) ? "YES" : "NO", (symmetric()) ? "YES" : "NO",
		scalefactor());
}

//////////////////////////////////////////////////////////////////////
// LinearFunction implementation
//////////////////////////////////////////////////////////////////////

LinearFunction::LinearFunction(const ImagePoint& point, bool symmetric)
	: FunctionBase(point, symmetric) {
	for (int i = 0; i < 3; i++) {
		a[i] = 0;
	}
}

LinearFunction::LinearFunction(const LinearFunction& other) :
	FunctionBase(other) {
	for (int i = 0; i < 3; i++) {
		a[i] = other.a[i];
	}
}

double  LinearFunction::evaluate(const Point& point) const {
	double	value = a[2];
	if (gradient() && (!symmetric())) {
		double	deltax = point.x() - center().x();
		double	deltay = point.y() - center().y();
		value += (deltax * a[0] + deltay * a[1]);
	}
	return scalefactor() * value;
}

inline static double	sqr(const double& x) {
	return x * x;
}

double	LinearFunction::norm() const {
	double	result = 0;
	result += sqr(center().x() * a[0]);
	result += sqr(center().y() * a[1]);
	result += sqr(a[2]);
	return sqrt(result);
}

LinearFunction      LinearFunction::operator+(const LinearFunction& other) {
	LinearFunction      result(*this);
	for (unsigned int i = 0; i < 3; i++) {
		result.a[i] = a[i] + other.a[i];
	}
	return result;
}

LinearFunction&     LinearFunction::operator=(
	const LinearFunction& other) {
	for (int i = 0; i < 3; i++) {
		a[i] = other.a[i];
	}
	return *this;
}

/**
 * \brief read only access to coefficients
 */
double	LinearFunction::operator[](int i) const {
	if ((i < 0) || (i > 2)) {
		throw std::range_error("index out of range");
	}
	return a[i];
}

/**
 * \brief modifying access to coefficients
 */
double&	LinearFunction::operator[](int i) {
	if ((i < 0) || (i > 2)) {
		throw std::range_error("index out of range");
	}
	return a[i];
}

/**
 * \brief Compute the best possible coefficients from a data set
 */
void	LinearFunction::reduce(const std::vector<doublevaluepair>& values) {
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
LinearFunction::LinearFunction(const ImagePoint& center, bool symmetric,
	const std::vector<LinearFunction::doublevaluepair>& values)
	: FunctionBase(center, symmetric) {
	a[0] = a[1] = a[2] = 0;
	reduce(values);
}

/**
 * \brief Text representation of a linear form
 */
std::string	LinearFunction::toString() const {
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
// Optimization problem solution: the LowerBound class
//////////////////////////////////////////////////////////////////////
class LowerBoundBase {
public:
	typedef	std::pair<Tile, float>	tilevalue;
	typedef std::vector<tilevalue>	tilevaluevector;
};

/**
 * \brief Find lower bound function
 *
 * This class finds symmetric and asymmetric lower bound functions of any
 * function type. Since the various function types use different optimization
 * problems, we have to provide specializations for each function type.
 */
template<typename FunctionType>
class LowerBound : public LowerBoundBase {
	virtual FunctionPtr	symmetricfunction(const ImagePoint& center,
		const tilevaluevector& values) const;
	virtual FunctionPtr	asymmetricfunction(const ImagePoint& center,
		const tilevaluevector& values) const;
public:
	FunctionPtr	operator()(const ImagePoint& center,
		bool symmetric, const tilevaluevector& values) const {
		if (symmetric) {
			return this->symmetricfunction(center, values);
		} else {
			return this->asymmetricfunction(center, values);
		}
	}
};

/**
 * \brief problem for symmetric linear functions
 *
 * A symmetric linear function is simply a constant, which is the minimum
 * of all values in the tiles.
 */
template<>
FunctionPtr	LowerBound<LinearFunction>::symmetricfunction(
	const ImagePoint& center, const tilevaluevector& values) const {
	double	minimum = std::numeric_limits<double>::infinity();
	tilevaluevector::const_iterator	i;
	for (i = values.begin(); i != values.end(); i++) {
		if (i->second < minimum) {
			minimum = i->second;
		}
	}
	LinearFunction	*result = new LinearFunction(center, true);
	(*result)[2] = minimum;
	return FunctionPtr(result);
}

/**
 * \brief Optimization problem for asymmetric linear functions
 */
template<>
FunctionPtr	LowerBound<LinearFunction>::asymmetricfunction(
	const ImagePoint& center, const tilevaluevector& values) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "asymmetric linear problem");
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
		double	deltax = vp->first.x() - center.x();
		double	deltay = vp->first.y() - center.y();
		double	val[4];
		val[1] = deltax;
		val[2] = deltay;
		val[3] = 1;
		glp_set_mat_row(lp, row, 3, ind, val);

		// objective function
		obj[0] += deltax;
		obj[1] += deltay;
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
	LinearFunction	*lb = new LinearFunction(center, false);
	(*lb)[0] = glp_get_col_prim(lp, 1);
	(*lb)[1] = glp_get_col_prim(lp, 2);
	(*lb)[2] = glp_get_col_prim(lp, 3);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "linear function: %s",
		lb->toString().c_str());
	return FunctionPtr(lb);
}

/**
 * \brief Optimization problem for symmetric quadratic functions
 */
template<>
FunctionPtr	LowerBound<QuadraticFunction>::symmetricfunction(
	const ImagePoint& center, const tilevaluevector& values) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "symmetric quadratic problem");
	// create a problem
	glp_prob	*lp = glp_create_prob();
	glp_set_obj_dir(lp, GLP_MAX);

	// we have three columns for the three coefficients of the linear
	// function, and one constraint for each tile
	glp_add_cols(lp, 2);
	glp_set_col_name(lp, 1, "minimum");
	glp_set_col_bnds(lp, 1, GLP_LO, 0, 0);
	glp_set_col_name(lp, 2, "q0");
	glp_set_col_bnds(lp, 2, GLP_DB, -10, 10);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "columns set up");

	// got through all the rows
	glp_add_rows(lp, values.size());
	tilevaluevector::const_iterator	vp;
	unsigned int	row = 1;
	int	ind[3] = { 0, 1, 2 };
	double	obj[2] = { values.size(), 0. };
	for (vp = values.begin(); vp != values.end(); vp++, row++) {
		// row name
		char	rowname[10];
		snprintf(rowname, sizeof(rowname), "s[%d]", row);
		glp_set_row_name(lp, row, rowname);

		// row coefficients and bounds
		glp_set_row_bnds(lp, row, GLP_UP, 0, vp->second);
		double	val[3];
		double	deltax = vp->first.x() - center.x();
		double	deltay = vp->first.y() - center.y();
		double	a = sqr(deltax) + sqr(deltay);
		val[1] = 1;
		val[2] = a;
		glp_set_mat_row(lp, row, 2, ind, val);

		// objective function
		obj[1] += a;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rows set up");

	// objective function
	glp_set_obj_coef(lp, 1, obj[0]);
	glp_set_obj_coef(lp, 2, obj[1]);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "objective function set up");

	// solve the linear problem
	glp_simplex(lp, NULL);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "simplex solution found");

	// cleanup the problem
	glp_delete_prob(lp);

	// return the result
	QuadraticFunction	*q = new QuadraticFunction(center, true);
debug(LOG_DEBUG, DEBUG_LOG, 0, "const term: %f", glp_get_col_prim(lp, 1));
	(*q)[2] = glp_get_col_prim(lp, 1);
	(*q)[3] = glp_get_col_prim(lp, 2);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "quadratic function: %s",
		q->toString().c_str());
	return FunctionPtr(q);
}

/**
 * \brief Optimization problem for asymmetric quadratic functions
 */
template<>
FunctionPtr	LowerBound<QuadraticFunction>::asymmetricfunction(
	const ImagePoint& center, const tilevaluevector& values) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "symmetric quadratic problem");
	// create a problem
	glp_prob	*lp = glp_create_prob();
	glp_set_obj_dir(lp, GLP_MAX);

	// we have three columns for the three coefficients of the linear
	// function, and one constraint for each tile
	glp_add_cols(lp, 6);
	glp_set_col_name(lp, 1, "alpha");
	glp_set_col_bnds(lp, 1, GLP_DB, -10, 10);
	glp_set_col_name(lp, 2, "beta");
	glp_set_col_bnds(lp, 2, GLP_DB, -10, 10);
	glp_set_col_name(lp, 3, "gamma");
	glp_set_col_bnds(lp, 3, GLP_LO, 0, 0);
	glp_set_col_name(lp, 4, "qsymmetric");
	glp_set_col_bnds(lp, 4, GLP_DB, -10, 10);
	glp_set_col_name(lp, 5, "qmixed");
	glp_set_col_bnds(lp, 5, GLP_DB, -10, 10);
	glp_set_col_name(lp, 6, "qhyperbolic");
	glp_set_col_bnds(lp, 6, GLP_DB, -10, 10);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "columns set up");

	// got through all the rows
	glp_add_rows(lp, values.size());
	tilevaluevector::const_iterator	vp;
	unsigned int	row = 1;
	int	ind[7] = { 0, 1, 2, 3, 4, 5, 6 };
	double	obj[6] = { 0., 0., 0., 0., 0., 0. };
	for (vp = values.begin(); vp != values.end(); vp++, row++) {
		// row name
		char	rowname[10];
		snprintf(rowname, sizeof(rowname), "s[%d]", row);
		glp_set_row_name(lp, row, rowname);

		// row coefficients and bounds
		glp_set_row_bnds(lp, row, GLP_UP, 0, vp->second);
		double	deltax = vp->first.x() - center.x();
		double	deltay = vp->first.y() - center.y();
		double	val[7];
		val[1] = deltax;
		val[2] = deltay;
		val[3] = 1;
		val[4] = sqr(deltax) + sqr(deltay);
		val[5] = deltax * deltay;
		val[6] = sqr(deltax) - sqr(deltay);
		glp_set_mat_row(lp, row, 6, ind, val);

		// objective function
		for (unsigned int i = 0; i < 6; i++) {
			obj[i] += val[i + 1];
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rows set up");

	// objective function
	for (unsigned int i = 0; i < 6; i++) {
		glp_set_obj_coef(lp, i + 1, obj[i]);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "objective function set up");

	// solve the linear problem
	glp_simplex(lp, NULL);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "simplex solution found");

	// cleanup the problem
	glp_delete_prob(lp);

	// return the result
	QuadraticFunction	*q = new QuadraticFunction(center, false);
	for (unsigned int i = 0; i < 6; i++) {
		(*q)[i] = glp_get_col_prim(lp, i + 1);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "quadratic function: %s",
		q->toString().c_str());
	return FunctionPtr(q);
}

//////////////////////////////////////////////////////////////////////
// MinimumEstimator implementation
//////////////////////////////////////////////////////////////////////

template<typename FunctionType>
FunctionPtr	MinimumEstimator<FunctionType>::operator()(const ImagePoint& center, bool symmetric) const {
	
	// construct a set of tiles
	TileFactory	tf(ImageSize(100, 100));
	TileSet	tileset = tf(_image);

	// initialize the loop
	FunctionPtr	h(new FunctionType(center, symmetric));
	float	delta = std::numeric_limits<float>::infinity();
	float	epsilon = 0.1;
	unsigned int	iterationcount = 0;
	while ((iterationcount < 10) && (delta > epsilon)) {
		LowerBoundBase::tilevaluevector	tv;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "start new iteration %d, h = %s",
			iterationcount, h->toString().c_str());

		// compute the order statistics in a tile
		TileSet::const_iterator	tileiterator;
		for (tileiterator = tileset.begin();
			tileiterator != tileset.end();
			tileiterator++) {
			WindowAdapter<float>	wa(_image, *tileiterator);
			FunctionPtrSubtractionAdapter	la(wa, h,
				tileiterator->origin());
			OrderStatisticsFilter<float>	of(_alpha);
			float	Z = of(la);
debug(LOG_DEBUG, DEBUG_LOG, 0, "Z = %f", Z);
			tv.push_back(std::make_pair(*tileiterator, Z));
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "values computed");

		// set up the optimization problem
		LowerBound<FunctionType>	lb;
		FunctionPtr	hhat
			= lb(_image.getSize().center(), symmetric, tv);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "hhat = %s",
			hhat->toString().c_str());

		// compute the improved lower bound function
		delta = hhat->norm();
		h = h + hhat;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "new lower bound: %s, delta = %f",
			h->toString().c_str(), delta);

		// increment the counter
		iterationcount++;
	}

	// return
	return h;


}

//////////////////////////////////////////////////////////////////////
// BackgroundExtractor implementation
//////////////////////////////////////////////////////////////////////

template<typename f>
Background<float>	getBackground(const ImagePoint& center,
				bool symmetric,
				const Image<RGB<float> >& image,
				const f& t) {
	// compute the lower bound for each color 
	unsigned int	alpha = 100;
	ColorRedAdapter<float>		redimage(image);
	MinimumEstimator<typename f::FunctionType>	rme(redimage, alpha);
	ColorGreenAdapter<float>	greenimage(image);
	MinimumEstimator<typename f::FunctionType>	gme(greenimage, alpha);
	ColorBlueAdapter<float>		blueimage(image);
	MinimumEstimator<typename f::FunctionType>	bme(blueimage, alpha);
	
	FunctionPtr	R = rme(center, symmetric);
	FunctionPtr	G = gme(center, symmetric);
	FunctionPtr	B = bme(center, symmetric);
	Background<float>	result(R, G, B);
	return result;
}

template<typename f>
Background<float>	getBackground(const ImagePoint& center,
				bool symmetric, unsigned int alpha,
				const Image<float>& image,
				const f& t) {
	// compute the lower bound for each color 
	MinimumEstimator<typename f::FunctionType>	me(image, alpha);
	FunctionPtr	l = me(center, symmetric);
	Background<float>	result(l, l, l);
	return result;
}

/**
 * \brief Compute the background of an image
 */
Background<float> BackgroundExtractor::operator()(const ImagePoint& center,
			bool symmetric, functiontype f,
			const Image<RGB<float> >& image) const {
	switch (f) {
	case CONSTANT:
		symmetric = true;
	case LINEAR:
		return getBackground(center, symmetric, alpha, image,
			function_tag<LinearFunction>());
	case QUADRATIC:
		return getBackground(center, symmetric, alpha, image,
			function_tag<QuadraticFunction>());
	}
}

Background<float> BackgroundExtractor::operator()(const ImagePoint& center,
			bool symmetric, functiontype f,
			const Image<float>& image) const {
	switch (f) {
	case CONSTANT:
		symmetric = true;
	case LINEAR:
		return getBackground(center, symmetric, image,
			function_tag<LinearFunction>());
	case QUADRATIC:
		return getBackground(center, symmetric, image,
			function_tag<QuadraticFunction>());
	}
}

//////////////////////////////////////////////////////////////////////
// QuadraticFunction implementation
//////////////////////////////////////////////////////////////////////

/**
 * \brief 
 */
QuadraticFunction::QuadraticFunction(const ImagePoint& center,
	bool symmetric) : LinearFunction(center, symmetric) {
	for (unsigned int i = 0; i < 3; i++) {
		q[i] = 0;
	}
}

double	QuadraticFunction::evaluate(const Point& point) const {
	double	value = LinearFunction::evaluate(point);
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

double	QuadraticFunction::operator[](int i) const {
	if (i < 3) {
		return LinearFunction::operator[](i);
	}
	if (i > 5) {
		std::runtime_error("index out of range");
	}
	return q[i - 3];
}

double	QuadraticFunction::norm() const {
	double	s = sqr(LinearFunction::norm());
	for (int i = 0; i < 3; i++) {
		s += sqr(q[i]);
	}
	return sqrt(s);
}

void	QuadraticFunction::reduce(const std::vector<FunctionBase::doublevaluepair>& values) {
	throw std::runtime_error("QuadraticFunction::reduce not implemented");
}

double&	QuadraticFunction::operator[](int i) {
	if (i < 3) {
		return LinearFunction::operator[](i);
	}
	if (i > 5) {
		std::runtime_error("index out of range");
	}
	return q[i - 3];
}

QuadraticFunction	QuadraticFunction::operator+(
	const QuadraticFunction& other) {
	QuadraticFunction	result(center(),
					symmetric() || other.symmetric());
	for (unsigned int i = 0; i < 6; i++) {
		result[i] += (*this)[i] + other[i];
	}
	return result;
}

QuadraticFunction	QuadraticFunction::operator+(
	const LinearFunction& other) {
	QuadraticFunction	result(center(),
					symmetric() || other.symmetric());
	for (unsigned int i = 0; i < 3; i++) {
		result[i] += (*this)[i] + other[i];
	}
	for (unsigned int i = 3; i < 5; i++) {
		result[i] += (*this)[i];
	}
	return result;
}

QuadraticFunction&	QuadraticFunction::operator=(
	const QuadraticFunction& other) {
	LinearFunction::operator=(other);
	for (unsigned int i = 0; i < 3; i++) {
		q[i] = other.q[i];
	}
	return *this;
}

QuadraticFunction&	QuadraticFunction::operator=(
	const LinearFunction& other) {
	LinearFunction::operator=(other);
	return *this;
}

std::string	QuadraticFunction::toString() const {
	return LinearFunction::toString()
		+ stringprintf("[%.6f, %.6f, %.6f]", q[0], q[1], q[2]);
} 

//////////////////////////////////////////////////////////////////////
// arithmetic operators for FunctionPtr
//////////////////////////////////////////////////////////////////////
FunctionPtr	operator+(const FunctionPtr& a, const FunctionPtr& b) {
	LinearFunction	*la = dynamic_cast<LinearFunction*>(&*a);
	LinearFunction	*lb = dynamic_cast<LinearFunction*>(&*b);
	QuadraticFunction	*qa = dynamic_cast<QuadraticFunction*>(&*a);
	QuadraticFunction	*qb = dynamic_cast<QuadraticFunction*>(&*b);
	if (qa == NULL) {
		if (qb == NULL) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "linear + linear");
			return FunctionPtr(new LinearFunction(*la + *lb));
		} else {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "linear + quadratic");
			QuadraticFunction	q(*la);
			return FunctionPtr(new QuadraticFunction(q + *qb));
		}
	} else {
		if (qb == NULL) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "quadratic + linear");
			QuadraticFunction	q(*lb);
			return FunctionPtr(new QuadraticFunction(*qa + q));
		} else {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "quadratic + quadratic");
			return FunctionPtr(new QuadraticFunction(*qa + *qb));
		}
	}
}

} // namespace image
} // namespace astro
