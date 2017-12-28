/*
 * VectorField.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroTransform.h>

namespace astro {
namespace image {
namespace transform {

/**
 * \brief Build a VectorField from a vector of pairs
 *
 * Copy the data into the Vectorfield class
 */
VectorField::VectorField(const std::vector<std::pair<ImagePoint, Point> >& data) {
	std::copy(data.begin(), data.end(), std::back_inserter(*this));
}

/**
 * \brief Construct a Vectorfield from a set of residuals
 */
VectorField::VectorField(const std::vector<Residual>& data) {
	std::vector<Residual>::const_iterator	i;
	for (i = data.begin(); i != data.end(); i++) {
		push_back(std::make_pair(i->from(), i->offset()));
	}
}

/**
 * \brief a particular point and count the number of other points it disagrees
 */
int	VectorField::verify(unsigned int i, double tolerance) {
	int	counter = 0;
	for (unsigned int j = 0; j < size(); j++) {
		if (i != j) {
			//debug(LOG_DEBUG, DEBUG_LOG, 0, "@%s %s - @%s %s",
			//	at(i).first.toString().c_str(),
			//	at(i).second.toString().c_str(),
			//	at(j).first.toString().c_str(),
			//	at(j).second.toString().c_str());
			double	r = distance(at(i).first, at(j).first);
			double	l = distance(at(i).second, at(j).second);
			//debug(LOG_DEBUG, DEBUG_LOG, 0,
			//	"[%d]: %d -> r = %f, l = %f, l/r = %f",
			//	i, j, r, l, l/r);
			if ((l/r) > tolerance) {
				//debug(LOG_DEBUG, DEBUG_LOG, 0,
				//	"discrepancy l/r = %f > %f at %d: r = %f, "
				//	"l = %f", l/r, tolerance, j, r, l);
				counter++;
			}
		}
	}
	return counter;
}

/**
 * \brief Verify a vectorfield and remove points that don't fit
 */
int	VectorField::verify(double tolerance) {
	int	deleted = 0;
	fielddata_t	t = badpoints(tolerance);
	fielddata_t::const_iterator	i;
	for (i = t.begin(); i != t.end(); i++) {
		fielddata_t::iterator j = std::find(begin(), end(), *i);
		if (j != end()) {
			erase(j);
			deleted++;
		}
	}
	return deleted;
}

/**
 * \brief count number of points to eliminate 
 */
VectorField::fielddata_t	VectorField::badpoints(double tolerance) {
	std::vector<std::pair<ImagePoint, Point> >	l;
	for (unsigned int i = 0; i < size(); i++) {
		if (verify(i, tolerance) > 3) {
			l.push_back(at(i));
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tol=%f gives %lu bad points",
		tolerance, l.size());
	return l;
}

/**
 * \brief Find tolerance that eliminates a given number of points
 */
double	VectorField::eliminate(int count) {
	int	iterations = 0;
	double	tolerance = 0.01;
	int	l = badpoints(tolerance).size();
	double	lowtolerance, hightolerance;
	int	lowcount, highcount;
	if (l > count) {
		while (l > count) {
			lowtolerance = tolerance;
			tolerance *= 2;
			lowcount = l;
			l = badpoints(tolerance).size();
			if (l == count) { return tolerance; }
			if (++iterations > 100) {
				throw std::runtime_error("cannot find tolerance");
			}
		}
		highcount = l;
		hightolerance = tolerance;
	} else {
		while (l < count) {
			hightolerance = tolerance;
			tolerance /= 2;
			highcount = l;
			l = badpoints(tolerance).size();
			if (l == count) { return tolerance; }
			if (++iterations > 100) {
				throw std::runtime_error("cannot find tolerance");
			}
		}
		lowcount = l;
		lowtolerance = tolerance;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for tol between %f and %f",
		lowtolerance, hightolerance);
	while (((lowcount - highcount) > 1)
		&& ((hightolerance - lowtolerance) > 0.00001)) {
		tolerance = (hightolerance + lowtolerance) / 2;
		l = badpoints(tolerance).size();
		if (l == count) {
			return tolerance;
		}
		if (++iterations > 100) {
			throw std::runtime_error("cannot find tolerance");
		}
		if (l < count) {
			hightolerance = tolerance;
			highcount = l;
		} else {
			lowtolerance = tolerance;
			lowcount = l;
		}
	}
	return tolerance;
}

/**
 * \brief
 */
void	VectorField::eliminate(double tolerance, std::vector<Residual>& residuals) {
	fielddata_t	b = badpoints(tolerance);
	fielddata_t::const_iterator	i;
	std::for_each(b.begin(), b.end(),
		[&residuals](const fielddata_t::value_type& bp) mutable {
			ImagePoint	ip = bp.first;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "eliminate @%s",
				ip.toString().c_str());
			std::vector<Residual>::iterator	j;
			for (j = residuals.begin(); j != residuals.end(); j++) {
				if (j->from() == ip) {
					debug(LOG_DEBUG, DEBUG_LOG, 0,
						"erase @%s -> %s",
						j->from().toString().c_str(),
						j->offset().toString().c_str());
					residuals.erase(j);
					return;
				}
			}
		}
	);
}

} // namespace transform
} // namespage image
} // namespace astro
