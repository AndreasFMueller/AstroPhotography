/*
 * FWHMEvaluator.h -- FWHM evaluator
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _FWHMEvaluator_h
#define _FWHMEvaluator_h

#include <AstroFocus.h>
#include "FocusEvaluatorImplementation.h"

namespace astro {
namespace focusing {

class FWHMEvaluator : public FocusEvaluatorImplementation {
protected:
	virtual double	evaluate(FocusableImageimage);
public:
	FWHMEvaluator(const ImageRectangle& rectangle);
};

} // namespace focusing
} // namespace astro

#endif /* _FWHMEvaulator_h */
