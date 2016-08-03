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

class FocusPoints;

/**
 * \brief Class encapsulationg the focus information
 */
class FocusPoint {
	int		_sequence;
	unsigned short	_position;
	double		_focusvalue;
	double		_l1norm;
	double		_when;
public:
	int	sequence() const { return _sequence; }
	unsigned short	position() const { return _position; }
	double	focusvalue() const { return _focusvalue; }
	double	l1norm() const { return _l1norm; }
	double	when() const { return _when; }
	FocusPoint(astro::image::ImagePtr image, unsigned short position = 0);
	std::string	toString() const;
friend class FocusPoints;
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
	double	minwhen() const;
	double	maxwhen() const;
	unsigned short	minposition() const;
	unsigned short	maxposition() const;
	double	minfocus() const;
	double	maxfocus() const;
	int	minsequence() const;
	int	maxsequence() const;
	std::vector<FocusPoint>	sortBySequence() const;
	std::vector<FocusPoint>	sortByPosition() const;
};

} // namespace snowgui

#endif /* _FocusPoint_h */
