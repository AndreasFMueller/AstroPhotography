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

/**
 * \brief Get an Evaluator by type
 *
 * \param type		evaluator type
 */
FocusEvaluatorPtr	FocusEvaluatorFactory::get(FocusEvaluatorType type) {
	ImageRectangle	rectangle;
	return get(type, rectangle);
}

/**
 * \brief Get an Evaluator by type
 *
 * \param type		evaluator type
 * \param rectangle	rectangle of interest
 */
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

/**
 * \brief Get an evaluator specified by name
 *
 * Valid names are returned by the evaluatornames method
 *
 * \param name	name of the evaluator
 */
FocusEvaluatorPtr	FocusEvaluatorFactory::get(const std::string& type) {
	return get(type, ImageRectangle());
}

/**
 * \brief Get a focus evaluator by name restricted to a rectangle
 *
 * \param type		name of the focus evaluator
 * \param rectangle	rectangle of interest
 */
FocusEvaluatorPtr	FocusEvaluatorFactory::get(const std::string& type,
				const ImageRectangle& rectangle) {
	FocusEvaluator	*evaluator = NULL;
	if (type == "BrennerHorizontal") {
		evaluator = new BrennerHorizontalEvaluator(rectangle);
	}
	if (type == "BrennerHorizontal") {
		evaluator = new BrennerVerticalEvaluator(rectangle);
	}
	if (type == "BrennerOmni") {
		evaluator = new BrennerOmniEvaluator(rectangle);
	}
	if (type == "fwhm") {
		evaluator = new FWHM2Evaluator(rectangle);
	}
	if (type == "measure") {
		evaluator = new MeasureEvaluator(rectangle);
	}
	if (NULL == evaluator) {
	}
	return FocusEvaluatorPtr(evaluator);
}

/**
 * \brief Construct a list of valid evaluators
 */
std::list<std::string>	FocusEvaluatorFactory::evaluatornames() {
	std::list<std::string>	names;
	names.push_back(std::string("BrennerHorizontal"));
	names.push_back(std::string("BrennerVertical"));
	names.push_back(std::string("BrennerOmni"));
	names.push_back(std::string("fwhm"));
	names.push_back(std::string("measure"));
	return names;
}

} // namespace focusing
} // namespace astro

