/*
 * MeasureEvaluator.h -- evaluator using the squaredgradient filter
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _MeasureEvaluator_h
#define _MeasureEvaluator_h

#include <AstroFocus.h>
#include "FocusEvaluatorImplementation.h"

namespace astro {
namespace focusing {

class MeasureEvaluator : public FocusEvaluatorImplementation {
public:
	MeasureEvaluator();
	MeasureEvaluator(const ImageRectangle& rectangle);
protected:
	virtual double	evaluate(FocusableImage image);
};

} // namespace focusing
} // namespace astro

#endif /* _MeasureEvaluator_h */
