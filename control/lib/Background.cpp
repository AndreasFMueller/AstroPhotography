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
#include <cmath>
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

static inline double	sqr(const double& x) {
	return x * x;
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
	double	obj[3] = { 0., 0., (double)values.size() };
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
	double	obj[2] = { (double)values.size(), 0. };
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

// force instantiation for the two most commonly used functions. We need to
// do this because we have not defined the operator() method of the
// MinimumEstimator class in the header file, so it cannot be instantiated
// where used
template class MinimumEstimator<LinearFunction>;
template class MinimumEstimator<QuadraticFunction>;

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
	throw std::runtime_error("unknown function type");
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
	throw std::runtime_error("unknown function type");
}

} // namespace image
} // namespace astro
