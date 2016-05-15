/*
 * FocusEvaluator.h -- implementation of focus evaluators 
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroTypes.h>
#include <AstroFocus.h>
#include <AstroDebug.h>
#include "FWHM2Evaluator.h"
#include "MeasureEvaluator.h"

namespace astro {
namespace focusing {

/**
 * \brief Implementation base class for Focus evaluators
 */
class FocusEvaluatorImplementation : public FocusEvaluator {
protected:
	ImageRectangle	_rectangle;
	FocusableImage	extractimage(ImagePtr image);
public:
	FocusEvaluatorImplementation(const ImageRectangle& rectangle);
	virtual double	operator()(const ImagePtr image) = 0;
};

} // namespace focusing
} // namespace astro

