/*
 * FocusPoints.cpp -- implementation of the FocusPoint classes
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <FocusPoints.h>
#include <limits>
#include <AstroDebug.h>
#include <AstroFilterfunc.h>
#include <AstroFormat.h>

using namespace astro::image;

namespace snowgui {

//////////////////////////////////////////////////////////////////////
// FocusPoint implementation
//////////////////////////////////////////////////////////////////////
FocusPoint::FocusPoint(astro::image::ImagePtr image, unsigned short position)
	: _position(position) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "analyzing %s image",
		image->size().toString().c_str());
	_sequence = -1;
	_l1norm = filter::l1norm(image);
	_focusvalue = filter::focus_squaredbrenner(image) / (_l1norm * _l1norm);
}

std::string	FocusPoint::toString() const {
	return astro::stringprintf("%d: l1=%f, f=%f, pos=%hu, when=%f",
		_sequence, _l1norm, _focusvalue, _position, _when);
}

//////////////////////////////////////////////////////////////////////
// FocusPoints implementation
//////////////////////////////////////////////////////////////////////
FocusPoints::FocusPoints() {
	_sequence = 0;
}

void	FocusPoints::add(const FocusPoint& focuspoint) {
	FocusPoint	f = focuspoint;
	f._sequence = _sequence++;
	push_back(f);
}

void	FocusPoints::add(ImagePtr image, unsigned short position) {
	add(FocusPoint(image, position));
}

double	FocusPoints::minwhen() const {
	double	t = std::numeric_limits<double>::max();
	std::for_each(begin(), end(),
		[t](const FocusPoint& f) mutable {
			if (f.when() < t) {
				t = f.when();
			}
		}
	);
	return t;
}

double	FocusPoints::maxwhen() const {
	double	t = 0;
	std::for_each(begin(), end(),
		[t](const FocusPoint& f) mutable {
			if (f.when() > t) {
				t = f.when();
			}
		}
	);
	return t;
}

unsigned short	FocusPoints::minposition() const {
	unsigned short	m = std::numeric_limits<unsigned short>::max();
	std::for_each(begin(), end(),
		[&m](const FocusPoint& f) mutable {
			if (f.position() < m) {
				m = f.position();
			}
		}
	);
	return m;
}

unsigned short	FocusPoints::maxposition() const {
	unsigned short	m = std::numeric_limits<unsigned short>::min();
	std::for_each(begin(), end(),
		[&m](const FocusPoint& f) mutable {
			if (f.position() > m) {
				m = f.position();
			}
		}
	);
	return m;
}

int	FocusPoints::minsequence() const {
	int	s = std::numeric_limits<int>::max();
	std::for_each(begin(), end(),
		[&s](const FocusPoint& f) mutable {
			if (f.sequence() < s) {
				s = f.sequence();
			}
		}
	);
	return s;
}

int	FocusPoints::maxsequence() const {
	int	s = std::numeric_limits<int>::min();
	std::for_each(begin(), end(),
		[&s](const FocusPoint& f) mutable {
			if (f.sequence() > s) {
				s = f.sequence();
			}
		}
	);
	return s;
}

double	FocusPoints::minfocus() const {
	double	v = std::numeric_limits<double>::max();
	std::for_each(begin(), end(),
		[&v](const FocusPoint& f) mutable {
			if (f.focusvalue() < v) {
				v = f.focusvalue();
			}
		}
	);
	return v;
}

double	FocusPoints::maxfocus() const {
	double	v = 0;
	std::for_each(begin(), end(),
		[&v](const FocusPoint& f) mutable {
			if (f.focusvalue() > v) {
				v = f.focusvalue();
			}
		}
	);
	return v;
}

class sequencecomparator {
public:
	bool operator()(const FocusPoint& a, const FocusPoint& b) {
		return a.sequence() < b.sequence();
	}
};

class positioncomparator {
public:
	bool operator()(const FocusPoint& a, const FocusPoint& b) {
		return a.position() < b.position();
	}
};

std::vector<FocusPoint>	FocusPoints::sortBySequence() const {
	std::vector<FocusPoint>	result;
	std::copy(begin(), end(), back_inserter(result));
	std::sort(result.begin(), result.end(), sequencecomparator());
	return result;
}

std::vector<FocusPoint>	FocusPoints::sortByPosition() const {
	std::vector<FocusPoint>	result;
	std::copy(begin(), end(), back_inserter(result));
	std::sort(result.begin(), result.end(), positioncomparator());
	return result;
}

} // namespace snowgui
