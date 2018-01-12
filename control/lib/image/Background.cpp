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
	int	htiles = imagesize.width() / _tilesize.width();
	if ((imagesize.width() - htiles * _tilesize.width()) < 20) {
		htiles -= 1;
	}
	int	vtiles = imagesize.height() / _tilesize.height();
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
	for (int x = 0; x < htiles; x++) {
		for (int y = 0; y < vtiles; y++) {
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
class LowerBoundBase : public std::map<std::string, double> {
public:
	LowerBoundBase(const std::map<std::string, double>& parameters) 
		: std::map<std::string, double>(parameters) { }
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
	LowerBound(const std::map<std::string, double>& parameters)
		: LowerBoundBase(parameters) { }
	FunctionPtr	operator()(const ImagePoint& center,
		bool symmetric, const tilevaluevector& values) const {
		if (symmetric) {
			return this->symmetricfunction(center, values);
		} else {
			return this->asymmetricfunction(center, values);
		}
	}
};

#include <LowerBoundLinearFunction.h>
#include <LowerBoundQuadraticFunction.h>
#include <LowerBoundDegreeNFunction.h>

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
//debug(LOG_DEBUG, DEBUG_LOG, 0, "Z = %f", Z);
			tv.push_back(std::make_pair(*tileiterator, Z));
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "values computed");

		// set up the optimization problem
		LowerBound<FunctionType>	lb(*this);
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
Background<float>	getBackground(const ImagePoint& center, bool symmetric,
				const BackgroundExtractor& extractor,
				const ConstImageAdapter<RGB<float> >& image,
				const f& /* t */) {
	// compute the lower bound for each color 
	ColorRedAdapter<float>		redimage(image);
	MinimumEstimator<typename f::FunctionType>	rme(extractor,
		redimage, extractor.alpha());
	ColorGreenAdapter<float>	greenimage(image);
	MinimumEstimator<typename f::FunctionType>	gme(extractor,
		greenimage, extractor.alpha());
	ColorBlueAdapter<float>		blueimage(image);
	MinimumEstimator<typename f::FunctionType>	bme(extractor,
		blueimage, extractor.alpha());
	
	FunctionPtr	R = rme(center, symmetric);
	FunctionPtr	G = gme(center, symmetric);
	FunctionPtr	B = bme(center, symmetric);
	Background<float>	result(R, G, B);
	return result;
}

template<typename f>
Background<float>	getBackground(const ImagePoint& center, bool symmetric,
				const BackgroundExtractor& extractor,
				const ConstImageAdapter<float>& image,
				const f& /* t */) {
	// compute the lower bound for each color 
	MinimumEstimator<typename f::FunctionType>	me(extractor, image,
		extractor.alpha());
	FunctionPtr	l = me(center, symmetric);
	Background<float>	result(l, l, l);
	return result;
}

/**
 * \brief Compute the background of an image
 */
Background<float> BackgroundExtractor::operator()(const ImagePoint& center,
			bool symmetric, functiontype f,
			const ConstImageAdapter<RGB<float> >& image) const {
	switch (f) {
	case CONSTANT:
		symmetric = true;
		[[fallthrough]];
	case LINEAR:
		return getBackground(center, symmetric, *this, image,
			function_tag<LinearFunction>());
	case QUADRATIC:
		return getBackground(center, symmetric, *this, image,
			function_tag<QuadraticFunction>());
	case DEGREE4:
		return getBackground(center, symmetric, *this, image,
			function_tag<DegreeNFunction>());
	}
	throw std::runtime_error("unknown function type");
}

Background<float> BackgroundExtractor::operator()(const ImagePoint& center,
			bool symmetric, functiontype f,
			const ConstImageAdapter<float>& image) const {
	switch (f) {
	case CONSTANT:
		symmetric = true;
		[[fallthrough]];
	case LINEAR:
		return getBackground(center, symmetric, *this, image,
			function_tag<LinearFunction>());
	case QUADRATIC:
		return getBackground(center, symmetric, *this, image,
			function_tag<QuadraticFunction>());
	case DEGREE4:
		return getBackground(center, symmetric, *this, image,
			function_tag<DegreeNFunction>());
	}
	throw std::runtime_error("unknown function type");
}

} // namespace image
} // namespace astro
