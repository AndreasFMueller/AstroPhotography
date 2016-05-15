/*
 * FWHM2Evaluator.h
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _FWHM2Evaluator_h
#define _FWHM2Evaluator_h

#include <AstroFocus.h>

namespace astro {
namespace focusing {

class FWHM2Evaluator : public FocusEvaluator {
	ImagePoint	_center;
	double	_radius;
public:
	FWHM2Evaluator(const ImagePoint& center, double radius = 20);
	FWHM2Evaluator();
	FWHM2Evaluator(const ImageRectangle& rectangle);
	virtual double	operator()(const ImagePtr image);
};

} // namespace focusing
} // namespace astro

#endif /* _FWHM2Evaluator_h */
