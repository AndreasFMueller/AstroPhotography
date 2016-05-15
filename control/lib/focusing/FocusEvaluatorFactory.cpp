/*
 * FocusEvaluatorFactory.cpp -- implementation of focus evaluator factory
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroTypes.h>
#include <AstroFocus.h>
#include <AstroDebug.h>
#include "FWHM2Evaluator.h"
#include "MeasureEvaluator.h"
#include "BrennerEvaluator.h"

namespace astro {
namespace focusing {

FocusEvaluatorPtr	FocusEvaluatorFactory::get(FocusEvaluatorType type) {
	ImageRectangle	rectangle;
	return get(type, rectangle);
}

FocusEvaluatorPtr	FocusEvaluatorFactory::get(FocusEvaluatorType type,
				const ImageRectangle& rectangle) {
	FocusEvaluator	*evaluator = NULL;
	switch (type) {
	case BrennerHorizontal:
		evaluator = new BrennerHorizontalEvaluator(rectangle);
		break;
	case BrennerVertical:
		evaluator = new BrennerVerticalEvaluator(rectangle);
		break;
	case BrennerOmni:
		evaluator = new BrennerOmniEvaluator(rectangle);
		break;
	case FWHM:
		evaluator = new FWHM2Evaluator(rectangle);
		break;
	case MEASURE:
		evaluator = new MeasureEvaluator(rectangle);
		break;
	}
	if (NULL == evaluator) {
		debug(LOG_ERR, DEBUG_LOG, 0, "unknown evaluator type %d", type);
		throw std::runtime_error("unknown evaluator type");
	}
	return FocusEvaluatorPtr(evaluator);
}

} // namespace focusing
} // namespace astro

