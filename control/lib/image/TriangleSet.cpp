/*
 * TriangleSet.cpp -- implementation of the triangle matching algorithm
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroTransform.h>
#include <cmath>

namespace astro {
namespace image {
namespace transform {

/**
 * \brief Triangle set constructor
 */
TriangleSet::TriangleSet() {
	_allow_mirror = false;
	_tolerance = 0.01;
}

/**
 * \brief Find triangle closest to a given triangle
 *
 * This uses the distance function to find the closes triangle using a
 * linear search.
 */
const Triangle& TriangleSet::closest(const Triangle& other) const {
	auto candidate = begin();
	double	distance = other.distance(*candidate);
	auto ptr = begin();
	for (ptr++; ptr != end(); ptr++) {
		double	d = other.distance(*ptr);
		if (d < distance) {
			candidate = ptr;
			distance = d;
		}
	}
	return *candidate;
}

class TrianglePair : public std::pair<Triangle, Triangle> {
public:
	TrianglePair(const Triangle& t1, const Triangle& t2)
		: std::pair<Triangle, Triangle>(t1, t2) {
	}
	std::string	toString() const {
		return stringprintf("%s ~ %s, d=%f, rotate=%f, scale=%f",
			first.toString().c_str(), second.toString().c_str(),
			first.distance(second),
			first.rotate_to(second) * 180 / M_PI,
			first.scale_to(second));
	}
};


class PairFunction {
public:
	virtual double	operator()(const TrianglePair& pair) const = 0;
};

class RotateTo : public PairFunction {
public:
	virtual double	operator()(const TrianglePair& pair) const {
		return pair.first.rotate_to(pair.second);
	}
};

class ScaleTo : public PairFunction {
public:
	virtual double	operator()(const TrianglePair& pair) const {
		return log(pair.first.scale_to(pair.second));
	}
};

template<int n>
class CharacteristicValue {
	PairFunction&	_pairfunction;
	double	_min;
	double	_max;
	double	_delta;
	int	counts[n];
	int	_maxindex;
public:
	double	delta() const { return _delta; }
private:
	int	index(double v) const {
		if (v < _min) {
			return 0;
		}
		if (v > _max) {
			return n - 1;
		}
		return trunc((v - _min) / _delta);
	}
public:
	CharacteristicValue(PairFunction& pairfunction, double min, double max)
		: _pairfunction(pairfunction), _min(min), _max(max) {
		_delta = (_max - _min) / n;
		std::fill_n(counts, n, 0);
	}
	void	add(const TrianglePair& pair) {
		double	v = _pairfunction(pair);
		counts[index(v)]++;
	}
	void	evaluate() {
		int	count = -1;
		for (int i = 0; i < n; i++) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "counts[%d] = %d, %f", i,
				counts[i], _min + _delta * (i + 0.5));
			if (counts[i] > count) {
				_maxindex = i;
				count = counts[_maxindex];
			}
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "characteristic bin: %d (%d)",
			_maxindex, count);
	}
	double	value() const {
		return _min + (_maxindex + 0.5) * _delta;
	}
	bool	faroff(const TrianglePair& pair) const {
		double	v = _pairfunction(pair);
		int	i = index(v);
		if ((n - 1) == _maxindex) {
			return ((i > 0) && (i < n - 2));
		}
		if (0 == _maxindex) {
			return ((i > 1) && (i < n - 1));
		}
		return (std::abs(_maxindex - i) > 1);
	}
};

double	angle_reduce(double a) {
	while (a > M_PI) {
		a -= 2 * M_PI;
	}
	while (a < -M_PI) {
		a += 2 * M_PI;
	}
	return a;
}

/**
 * \brief Find the closest transform from a set of triangles
 */
Transform	TriangleSet::closest(const TriangleSet& other) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "finding transform from %d to %d "
		"triangles", size(), other.size());
	// for each triangle find a close triangle in the other set, the
	// parameter _tolerance decides how close is close enough
	std::list<TrianglePair>	trianglepairs;
	for (auto ptr = begin(); ptr != end(); ptr++) {
		Triangle	b = other.closest(*ptr);
		// reject pairs with that imply mirror images
		if (!_allow_mirror) {
			if (b.mirror_to(*ptr)) {
				continue;
			}
		}
		// reject pairs that are not close enough
		double	d = ptr->distance(b);
		if (d > _tolerance) {
			continue;
		}
		// add the pair to the set
		TrianglePair	pair(*ptr, b);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "closest triangle: %s",
			pair.toString().c_str());
		trianglepairs.push_back(pair);
	}

	// stop if we have no suitable triangle pairs
	if (trianglepairs.size() == 0) {
		std::string	msg = stringprintf("no close triangles at "
			"tolerance %f found", _tolerance);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// some of the triangle pairs may have scales or rotation angles
	// that are completely off, so we now find common scales and rotation
	// angles using a histogram
	RotateTo	anglefunction;
	CharacteristicValue<256>	anglehistogram(anglefunction,
						0, 2 * M_PI);
	for (auto p = trianglepairs.begin(); p != trianglepairs.end(); p++) {
		anglehistogram.add(*p);
	}
	anglehistogram.evaluate();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "characteristic angle: %f",
		anglehistogram.value() * 180 / M_PI);

	// remove all triangle pairs that are far off the common values
	int	counter = 0;
	auto ptr = trianglepairs.begin();
	while (ptr != trianglepairs.end()) {
		if (anglehistogram.faroff(*ptr)) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "exclude %s",
				ptr->toString().c_str());
			ptr = trianglepairs.erase(ptr);
			counter++;
		} else {
			ptr++;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"%d pairs excluded: rotation angle far from %f",
		counter, anglehistogram.value() * 180 / M_PI);

	// remove all triangles for which the angle is far off.
	double	rotatebase = anglehistogram.value();
	double	rotatesum = 0, rotate2sum = 0;
	ptr = trianglepairs.begin();
	while (ptr != trianglepairs.end()) {
		double	r = ptr->first.rotate_to(ptr->second) - rotatebase;
		r = angle_reduce(r);
		rotatesum += r;
		rotate2sum += r * r;
		ptr++;
	}
	counter = 0;
	ptr = trianglepairs.begin();
	double	mean = rotatesum / trianglepairs.size() + rotatebase;
	double	stddev = sqrt(rotate2sum / trianglepairs.size());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rotate mean: %f, stddev: %f",
		mean * 180 / M_PI, stddev * 180 / M_PI);
	while (ptr != trianglepairs.end()) {
		double	r = ptr->first.rotate_to(ptr->second) - mean;
		r = angle_reduce(r);
		if (fabs(r) > stddev) {
			ptr = trianglepairs.erase(ptr);
			counter++;
		} else {
			ptr++;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d pairs excluded for too large "
		"rotation angle", counter);

	// collect scale values
	ScaleTo	scalefunction;
	CharacteristicValue<101>	scalehistogram(scalefunction, -1, 1);
	for (auto p = trianglepairs.begin(); p != trianglepairs.end(); p++) {
		scalehistogram.add(*p);
	}
	scalehistogram.evaluate();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "characteristic scale: %f",
		exp(scalehistogram.value()));

	// remove the triangle pairs that have bad scale
	counter = 0;
	ptr = trianglepairs.begin();
	while (ptr != trianglepairs.end()) {
		if (scalehistogram.faroff(*ptr)) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "exclude %s",
				ptr->toString().c_str());
			ptr = trianglepairs.erase(ptr);
			counter++;
		} else {
			ptr++;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"%d pairs excluded: scale factor far from %f",
		counter, exp(scalehistogram.value()));

	// compute the mean and variance of the scale
	double	scalesum = 0, scale2sum = 0;
	ptr = trianglepairs.begin();
	while (ptr != trianglepairs.end()) {
		double	s = ptr->first.scale_to(ptr->second);
		scalesum += s;
		scale2sum += s * s;
		ptr++;
	}
	mean = scalesum / trianglepairs.size();
	stddev = sqrt((scale2sum / trianglepairs.size()) - mean * mean);
	counter = 0;
	ptr = trianglepairs.begin();
	while (ptr != trianglepairs.end()) {
		double	s = ptr->first.scale_to(ptr->second);
		if ((fabs(s - mean) / stddev) > 1) {
			ptr = trianglepairs.erase(ptr);
			counter++;
		} else {
			ptr++;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d pairs eliminated for scale variance",
		counter);

	// display the triangles we plan to use for transform computation
	std::list<TrianglePair>::const_iterator	tp;
	int	i;
	for (i = 0, tp = trianglepairs.begin();
		tp != trianglepairs.end(); i++, tp++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "using triangle pair %d: %s",
			i, tp->toString().c_str());
	}

	// now that we have triangles that we know match, we can also 
	// construct a set of points that should match
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d matching triangles",
		trianglepairs.size());
	std::vector<Point>	from;
	std::vector<Point>	to;
	for (tp = trianglepairs.begin(); tp != trianglepairs.end(); tp++) {
		for (int i = 0; i < 3; i++) {
			from.push_back(tp->first[i]);
			to.push_back(tp->second[i]);
		}
	}
	TransformFactory	tf;
	return tf(from, to);
}

} // namespace transform
} // namespace image
} // namespace astro
