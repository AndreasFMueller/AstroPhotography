/*
 * FocusPoint.h -- Information about focus points
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _FocusPoint_h
#define _FocusPoint_h

#include <list>
#include <vector>
#include <AstroImage.h>

namespace snowgui {

namespace FocusPointOrder {
	typedef enum { position = 0, sequence = 1, time = 2 } order_t;
}

namespace FocusPointMeasure {
	typedef enum { fwhm = 0, brenner = 1 } measure_t;
}

class FocusPoints;

/**
 * \brief Class encapsulationg the focus information
 */
class FocusPoint {
	int		_sequence;
	unsigned short	_position;
	double		_when;
	double		_l1norm;
	double		_fwhm;
	double		_brenner;
public:
	int	sequence() const { return _sequence; }
	unsigned short	position() const { return _position; }
	double	l1norm() const { return _l1norm; }
	double	fwhm() const { return _fwhm; }
	double	brenner() const { return _brenner; }
	double	when() const { return _when; }
	FocusPoint(astro::image::ImagePtr image, unsigned short position = 0);
	std::string	toString() const;
friend class FocusPoints;
};

/**
 * \brief Encapsulation for raw points
 *
 * Raw points only contain x and y values (not scaled yet) to be used in the
 * display of the FocusPoints
 */
class FocusRawPoint {
	double	_x;
	double	_y;
public:
	FocusRawPoint(double x, double y) : _x(x), _y(y) { }
	const double&	x() const { return _x; }
	const double&	y() const { return _y; }
};

/**
 * \brief Extract raw point information from a FocusPoint
 */
class FocusRawPointExtractor {
public:
private:
	FocusPointOrder::order_t	_order;
	FocusPointMeasure::measure_t	_measure;
public:
	FocusPointOrder::order_t	order() const { return _order; }
	FocusPointMeasure::measure_t	measure() const { return _measure; }

	FocusRawPointExtractor(FocusPointOrder::order_t order
		= FocusPointOrder::position,
		FocusPointMeasure::measure_t measure = FocusPointMeasure::fwhm)
		: _order(order), _measure(measure) { }
	FocusRawPoint	operator()(const FocusPoint&) const;
};

class FocusRawValueExtractor : public FocusRawPointExtractor {
public:
	FocusRawValueExtractor(FocusPointOrder::order_t order = FocusPointOrder::position,
		FocusPointMeasure::measure_t measure = FocusPointMeasure::fwhm)
		: FocusRawPointExtractor(order, measure) { }
	virtual double	value(const FocusPoint& p) const = 0;
};

class FocusRawXValueExtractor : public FocusRawValueExtractor {
public:
	FocusRawXValueExtractor(FocusPointOrder::order_t order = FocusPointOrder::position)
		: FocusRawValueExtractor(order, FocusPointMeasure::fwhm) { }
	double	value(const FocusPoint& p) const {
		return (*this)(p).x();
	}
};

class FocusRawYValueExtractor : public FocusRawValueExtractor {
public:
	FocusRawYValueExtractor(FocusPointMeasure::measure_t measure = FocusPointMeasure::fwhm)
		: FocusRawValueExtractor(FocusPointOrder::position, measure) { }
	double	value(const FocusPoint& p) const {
		return (*this)(p).y();
	}
};

/**
 * \brief A list of FocusPoints
 *
 * This class adds some convenience function to improve the display of the
 * focus points
 */
class FocusPoints : public std::list<FocusPoint> {
	int	_sequence;
public:
	FocusPoints();
	void	add(const FocusPoint& point);
	void	add(astro::image::ImagePtr image, unsigned short position = 0);

	// x-ranges
	double	minposition() const;
	double	maxposition() const;
	double	minsequence() const;
	double	maxsequence() const;
	double	minwhen() const;
	double	maxwhen() const;

	// value ranges
	double	minbrenner() const;
	double	maxbrenner() const;
	double	minfwhm() const;
	double	maxfwhm() const;

	// by order/measure constant
	double	min(FocusPointOrder::order_t order) const;
	double	max(FocusPointOrder::order_t order) const;
	double	min(FocusPointMeasure::measure_t measure) const;
	double	max(FocusPointMeasure::measure_t measure) const;

private:
	// generic methods
	double	min(const FocusRawValueExtractor&) const;
	double	max(const FocusRawValueExtractor&) const;

public:
	std::vector<FocusRawPoint> sort(const FocusRawPointExtractor&) const;
};

} // namespace snowgui

#endif /* _FocusPoint_h */
