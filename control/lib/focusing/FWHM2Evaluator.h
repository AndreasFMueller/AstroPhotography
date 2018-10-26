/*
 * FWHM2Evaluator.h
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _FWHM2Evaluator_h
#define _FWHM2Evaluator_h

#include <AstroFocus.h>
#include "FocusEvaluatorImplementation.h"

namespace astro {
namespace focusing {

class FWHM2Evaluator : public FocusEvaluatorImplementation {
	ImagePoint	_center;
	double	_radius;
public:
	FWHM2Evaluator(const ImagePoint& center, double radius = 20);
	FWHM2Evaluator();
	FWHM2Evaluator(const ImageRectangle& rectangle);
protected:
	virtual double	evaluate(FocusableImage image);
};

} // namespace focusing
} // namespace astro

#endif /* _FWHM2Evaluator_h */
