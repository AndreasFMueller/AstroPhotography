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
#include <AstroFWHM.h>

using namespace astro::image;

namespace snowgui {

#define	MAX_FWHM_SIZE	800

//////////////////////////////////////////////////////////////////////
// FocusPoint implementation
//////////////////////////////////////////////////////////////////////
FocusPoint::FocusPoint(astro::image::ImagePtr image, long position)
	: _position(position) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "analyzing %s image",
		image->size().toString().c_str());
	_sequence = -1;
	_l1norm = filter::l1norm(image);
	// XXX we would rather like to have the square of _l1norm in the 
	// XXX denominator, but that makes the values too small.
	// XXX So we need a more reasonable rescaling method
	_brenner = filter::focus_squaredbrenner(image) / _l1norm;
	// use the classes in AstroFWHM.h to compute the FWHM
	if (image->size().getPixels() < MAX_FWHM_SIZE * MAX_FWHM_SIZE) {
		fwhm::ComponentDecomposer	decomposer(image, false);
		_fwhm = 2 * decomposer.maxradius();
	} else {
		_fwhm = 0.;
	}
	// set the time
	_when = astro::Timer::gettime();
}

std::string	FocusPoint::toString() const {
	return astro::stringprintf("%d: l1=%f, fwhm=%f, brenner=%g, pos=%hu, when=%f",
		_sequence, _l1norm, _fwhm, _brenner, _position, _when);
}

//////////////////////////////////////////////////////////////////////
// FocusRawPointExtractor
//////////////////////////////////////////////////////////////////////
FocusRawPoint   FocusRawPointExtractor::operator()(const FocusPoint& p) const {
	double	x, y;
	switch (_order) {
	case FocusPointOrder::position:
		x = p.position();
		break;
	case FocusPointOrder::sequence:
		x = p.sequence();
		break;
	case FocusPointOrder::time:
		x = p.when();
		break;
	default:
		throw std::logic_error("unknown order type");
	}
	switch (_measure) {
	case FocusPointMeasure::fwhm:
		y = p.fwhm();
		break;
	case FocusPointMeasure::brenner:
		y = p.brenner();
		break;
	default:
		throw std::logic_error("unknown measure type");
	}
	return FocusRawPoint(x, y);
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add focus point %s",
		f.toString().c_str());
	push_back(f);
}

void	FocusPoints::add(ImagePtr image, long position) {
	add(FocusPoint(image, position));
}

bool	xcomparator(const FocusRawPoint& a, const FocusRawPoint& b) {
	return a.x() < b.x();
}

std::vector<FocusRawPoint>	FocusPoints::sort(const FocusRawPointExtractor& extractor) const {
	std::vector<FocusRawPoint>	result;
	std::for_each(begin(), end(), 
		[&extractor,&result](const FocusPoint& p) {
			result.push_back(extractor(p));
		}
	);
	std::sort(result.begin(), result.end(), xcomparator);
	return result;
}

/*
 * \brief Determineminimum x value of an Extractor
 */
double	FocusPoints::min(const FocusRawValueExtractor& extractor) const {
	double	result = std::numeric_limits<double>::max();
	std::for_each(begin(), end(),
		[&extractor,&result](const FocusPoint& p) mutable {
			double	v = extractor.value(p);
			if (v < result) {
				result = v;
			}
		}
	);
	return result;
}

double	FocusPoints::min(FocusPointOrder::order_t order) const {
	FocusRawXValueExtractor	extractor(order);
	return min(extractor);
}

double	FocusPoints::minwhen() const {
	return min(FocusPointOrder::time);
}
double	FocusPoints::minposition() const {
	return min(FocusPointOrder::position);
}
double	FocusPoints::minsequence() const {
	return min(FocusPointOrder::sequence);
}

double	FocusPoints::min(FocusPointMeasure::measure_t measure) const {
	FocusRawYValueExtractor	extractor(measure);
	return min(extractor);
}

double	FocusPoints::minfwhm() const {
	return min(FocusPointMeasure::fwhm);
}
double	FocusPoints::minbrenner() const {
	return min(FocusPointMeasure::brenner);
}

/*
 * \brief Determinemaximum x value of an Extractor
 */
double	FocusPoints::max(const FocusRawValueExtractor& extractor) const {
	double	result = std::numeric_limits<double>::min();
	std::for_each(begin(), end(),
		[&extractor,&result](const FocusPoint& p) mutable {
			double	v = extractor.value(p);
			if (v > result) {
				result = v;
			}
		}
	);
	return result;
}

double	FocusPoints::max(FocusPointOrder::order_t order) const {
	FocusRawXValueExtractor	extractor(order);
	return max(extractor);
}

double	FocusPoints::maxwhen() const {
	return max(FocusPointOrder::time);
}
double	FocusPoints::maxposition() const {
	return max(FocusPointOrder::position);
}
double	FocusPoints::maxsequence() const {
	return max(FocusPointOrder::sequence);
}

double	FocusPoints::max(FocusPointMeasure::measure_t measure) const {
	FocusRawYValueExtractor	extractor(measure);
	return max(extractor);
}

double	FocusPoints::maxfwhm() const {
	return max(FocusPointMeasure::fwhm);
}
double	FocusPoints::maxbrenner() const {
	return max(FocusPointMeasure::brenner);
}

} // namespace snowgui
