/*
 * FWHMEvaluator.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "FWHMEvaluator.h"
#include <AstroFilter.h>
#include <ConnectedComponent.h>
#include "BackgroundAdapter.h"

namespace astro {
namespace focusing {

/**
 * \brief Construct a FWHM evaluator
 */
FWHMEvaluator::FWHMEvaluator(const ImageRectangle& rectangle)
	: FocusEvaluatorImplementation(rectangle) {
}

/**
 * \brief A class designed to detect peaks in an image
 */
class PeakDetectorAdapter : public ConstImageAdapter<float> {
	const ConstImageAdapter<float>	&_image;
	float	_min;
	float	b(int u, int v, float& s) const {
		float	p = _image.pixel(u, v);
		s += p;
		return p;
	}
public:
	PeakDetectorAdapter(const ConstImageAdapter<float>& image, float min)
		: ConstImageAdapter<float>(image.getSize()),
		  _image(image), _min(min) {
	}
	float	pixel(int x, int y) const {
		float	v = _image.pixel(x, y);
		// value to low, skip it
		if (v < _min) { return 0; }
		// make sure this really is the local maximum
		float	s = v;
		float	l = 0.5 * (_min + v);
		if (b(x - 1, y - 1, s) > v) { return 0; }
		if (b(x - 1, y    , s) > v) { return 0; }
		if (b(x - 1, y + 1, s) > v) { return 0; }
		if (b(x    , y - 1, s) > v) { return 0; }
		if (b(x    , y + 1, s) > v) { return 0; }
		if (b(x + 1, y - 1, s) > v) { return 0; }
		if (b(x + 1, y    , s) > v) { return 0; }
		if (b(x + 1, y + 1, s) > v) { return 0; }
		// compute the average
		s = s / 9;
		if (s > l) { return v; }
		return 0;
	}
};

/**
 * \brief A class representing a bright point in an image
 *
 * Bright points are the ones the focusing algorithm should focus on
 */
class BrightPoint {
	ImagePoint	_point;
	float		_value;
public:
	const ImagePoint&	point() const { return _point; }
	float	value() const { return _value; }
	BrightPoint(const ImagePoint& point, float value)
		: _point(point), _value(value) {
	}
	BrightPoint(int x, int y, float value)
		: _point(x, y), _value(value) {
	}
	std::string	toString() const {
		return stringprintf("%s,value=%f", _point.toString().c_str(),
			_value);
	}
};

/**
 * \brief A list of bright points
 * 
 * The constructor of this class is supposed to build a list of bright
 * points of an image
 */
class BrightPoints : public std::list<BrightPoint> {
	float	_min;
public:
	BrightPoints(const ConstImageAdapter<float>& image, float min)
		: _min(min) {
		clear();
		int	w = image.getSize().width() - 1;
		int	h = image.getSize().height() - 1;
		PeakDetectorAdapter	pd(image, min);
		for (int x = 1; x < w; x++) {
			for (int y = 1; y < h; y++) {
				float v = pd.pixel(x, y);
				if (v > min) {
					push_back(BrightPoint(x, y, v));
				}
			}
		}
		// show debug
		std::for_each(begin(), end(),
			[](const BrightPoint& b) {
				debug(LOG_DEBUG, DEBUG_LOG, 0, "bright point %s",
					b.toString().c_str());
			}
		);
	}
	float	mean() const {
		int	counter = 0;
		float	sum;
		std::for_each(begin(), end(),
			[&](const BrightPoint& p) {
				sum += p.value();
				counter++;
			}
		);
		return sum / counter;
	}
	float	quantile(float q) const {
		std::multiset<float>	values;
		std::for_each(begin(), end(),
			[&](const BrightPoint& p) {
				values.insert(p.value());
			}
		);
		int	i = round((values.size() - 1) * q);
		auto	j = begin();
		while (i--) j++;
		return j->value();
	}
	float	median() const { return quantile(0.5); }
};

/**
 * \brief Accumulator for components
 *
 * This essentially is a immage that collects information about components
 */
class ComponentAccumulator : public Image<unsigned char> {
public:
	ComponentAccumulator(const ImageSize& size)
		: Image<unsigned char>(size) {
		fill(0);
	}
	void	accumulate(const Image<unsigned char> image) {
		int	w = image.size().width();
		int	h = image.size().height();
		for (int x = 0; x < w; x++) {
			for (int y = 0; y < h; y++) {
				if (image.pixel(x, y) == 255) {
					pixel(x, y) = 255;
				}
			}
		}
	}
	bool	previous_component(const ImagePoint& point) const {
		return pixel(point) == 255;
	}
	bool	previous_component(int x, int y) const {
		return pixel(x, y) == 255;
	}
};

/**
 * \brief A data class collecting information about each component
 */
class	ComponentInfo {
	ImagePoint	_point;
	float		_value;
	long		_npoints;
	ImagePoint	_center;
	float		_radius;
public:
	const ImagePoint&	point() const { return _point; }
	void	point(const ImagePoint& p) { _point = p; }

	float	value() const { return _value; }
	void	value(float v) { _value = v; }

	long	npoints() const { return _npoints; }
	void	npoints(long n) { _npoints = n; }

	float	radius() const { return _radius; }
	void	radius(float r) { _radius = r; }

	const ImagePoint	center() const { return _center; }
	void	center(const ImagePoint& c) { _center = c; }
	void	center(const Point& p) {
		_center = ImagePoint(round(p.x()), round(p.y()));
	}

	ComponentInfo(const ImagePoint& point, float value)
		: _point(point), _value(value), _radius(0) {
	}

	ComponentInfo(const BrightPoint& bp)
		: _point(bp.point()), _value(bp.value()), _radius(0) {
	}

	std::string	toString() const {
		return stringprintf("at=%s, value=%.2f, npoints=%ld, "
			"center=%s, radius=%.2f",
			point().toString().c_str(), value(),
			npoints(), center().toString().c_str(), radius());
	}
	void	draw(Image<unsigned char>& image) {
		int	X = point().x();
		int	Y = point().y();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "drawing at (%d,%d)", X, Y);
		int	w = image.getSize().width() - 1;
		int	h = image.getSize().height() - 1;
		int	minx = X - 5; if (minx < 0) minx = 0;
		int	maxx = X + 5; if (maxx > w) maxx = w;
		int	miny = Y - 5; if (miny < 0) miny = 0;
		int	maxy = Y + 5; if (maxy > h) maxy = h;
		for (int x = minx; x <= maxx; x++) {
			image.pixel(x, Y) = 255;
		}
		for (int y = miny; y <= maxy; y++) {
			image.pixel(X, y) = 255;
		}
	}
};

/**
 * \brief A list of component info objects with additional capabilities
 */
class	ComponentInfoList : public std::list<ComponentInfo> {
public:
	float	meanreadius() {
		if (0 == size()) {
			throw std::runtime_error("cannot form mean of 0 "
				"components");
		}
		double	sum = 0;
		std::for_each(begin(), end(),
			[&sum](const ComponentInfo& ci) {
				sum += ci.radius();
			}
		);
		return sum / size();
	}
	float	medianradius() {
		Median<float>	M;
		std::for_each(begin(), end(),
			[&](const ComponentInfo& ci) {
				M.add(ci.radius());
			}
		);
		return M.median();
	}

	void	draw(Image<unsigned char>& image) {
		std::for_each(begin(), end(),
			[&](ComponentInfo& ci) {
				ci.draw(image);
			}
		);
	}
};

/**
 * \brief A class able to draw circles
 */
class CircleAccumulator : public Image<unsigned char> {
public:
	CircleAccumulator(const ImageSize& size) : Image<unsigned char>(size) {
		fill(0);
	}

	void	accumulate(const ComponentInfo& ci) {
		double	R = sqr(ci.radius());
		int	w = getSize().width() - 1;
		int	h = getSize().height() - 1;
		int	r = trunc(ci.radius());
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"drawing circle of radius %d at %s",
			r, ci.point().toString().c_str());
		int	minx = ci.center().x() - r; if (minx < 0) { minx = 0; }
		int	maxx = ci.center().x() + r; if (maxx > w) { maxx = w; }
		debug(LOG_DEBUG, DEBUG_LOG, 0, "drawing x=%d:%d", minx, maxx);
		for (int x = minx; x <= maxx; x++) {
			int	dy = trunc(sqrt(R - sqr(x - ci.center().x())));
			int	miny = ci.center().y() - dy;
			int	maxy = ci.center().y() + dy;
			if (maxy > h) { maxy = h; }
			if (miny < 0) { miny = 0; }
			for (int y = miny; y <= maxy; y++) {
				pixel(x, y) = 255;
			}
		}
	}

	void	accumulate(const std::list<ComponentInfo>& cis) {
		//CircleAccumulator	*ca = this;
		std::for_each(cis.begin(), cis.end(),
			[&](const ComponentInfo& ci) {
				accumulate(ci);
			}
		);
	}
};

/**
 * \brief evaluate an image
 */
double	FWHMEvaluator::evaluate(FocusableImage image) {

	FocusImagePreconditioner	precond(image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "value range: [%f, %f]",
		precond.noisefloor(), precond.mean() + 2 * precond.stddev());

	// find nonisolated maxima
	BrightPoints	bp(*image, precond.top());

	// prepare an image that we can use to report all the connected
	// components
	ComponentAccumulator	components(image->size());
	CircleAccumulator	circles(image->size());
	ComponentInfoList	componentinfolist;

	// find connected components. They are identified by a a representative
	// point for each component
	for (auto i = bp.begin(); i != bp.end(); i++) {
		// first find out whether we have found this component before
		// and skip it if so
		if (components.previous_component(i->point()))
			continue;

		// construct the comopnent at the bright point
		ComponentInfo	ci(*i);

		// find the component
		float	s = (i->value() + precond.noisefloor()) / 2;
		image::Component<float>	comp(*image, s, i->point());

		// throw away small components
		if (comp.npoints() <= 1) continue;
		ci.npoints(comp.npoints());

		// for each connected component, determine the diameter
		ci.radius(comp.radius());
		if (ci.radius() < 1) continue;
		ci.center(comp.center());

		// accumulate everything
		components.accumulate(comp);
		circles.accumulate(ci);

		debug(LOG_DEBUG, DEBUG_LOG, 0, "adding %s",
			ci.toString().c_str());

		// collect remaining 
		componentinfolist.push_back(ci);
	}

	// if we have no components, we have to give up
	if (componentinfolist.size() == 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no components found");
		return -1;
	}

	// get the median radius
	float	medianradius = componentinfolist.medianradius();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "median radius %.3f", medianradius);

	// draw the crosses
	Image<unsigned char>	crosses(image->getSize());
	crosses.fill(0);
	componentinfolist.draw(crosses);

	// combine the images accumulated so far into an RGB image
	adapter::CombinationAdapter<unsigned char> combine(components,
							circles, crosses);

	_evaluated_image = ImagePtr(new Image<RGB<unsigned char> >(combine));

	// copy meta data from the original image
	_evaluated_image->metadata(image->metadata());

	// modify the image based on the preconditioner
	precond.top(precond.noisefloor() + precond.stddev());
	int	w = image->getSize().width();
	int	h = image->getSize().height();
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
#if 1
			//double	v = precond.pixel(x, y);
			//double	w = image->pixel(x, y);
			//debug(LOG_DEBUG, DEBUG_LOG, 0, "change[%d,%d] %f -> %f",
			//	x, y, w, v);
#endif
			image->pixel(x, y) = precond.pixel(x, y);
		}
	}

	// return the average diameter as the result
	return medianradius;
}

} // namespace focusing
} // namespace astro
