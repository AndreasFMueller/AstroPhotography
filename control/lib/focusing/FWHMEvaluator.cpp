/*
 * FWHMEvaluator.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "FWHMEvaluator.h"

namespace astro {
namespace focusing {

FWHMEvaulator::FWHMEvaluator(const ImageRectangle& rectangle)
	: FocusEvaluatorImplementation(rectangle) {
}

double	FWHMEvaluator::evaluate(FocusableImage image) {
	return 1;
}

} // namespace focusing
} // namespace astro
