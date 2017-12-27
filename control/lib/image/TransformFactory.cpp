/*
 * TransformFactory.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroTransform.h>
#include "TransformBuilder.h"

namespace astro {
namespace image {
namespace transform {

/**
 * \brief Construct a TransformFactory
 */
TransformFactory::TransformFactory(bool rigid) : _rigid(rigid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rigid = %s", (_rigid) ? "yes" : "no");
}

/**
 * \brief Build a transform from a set of residuals
 */
Transform	TransformFactory::operator()(const std::vector<Residual>& residuals) {
	if (residuals.size() == 0) {
		throw std::runtime_error("need at least one residual to "
			"extract translation");
	}
	Transform	t;

	// make sure we have enough points
	if (residuals.size() < 3) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "not enough data for full "
			"transform, extracting translation only");
		// compute the average of all translations
		Point	sum;
		for (auto r = residuals.begin(); r != residuals.end(); r++) {
			sum = sum + r->offset();
		}
		sum = (1. / residuals.size()) * sum;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "average translation: %s",
			std::string(sum).c_str());

		// the average translation becomes the constant part
		// of the transform
		t[2] = sum.x();
		t[5] = sum.y();
		return t;
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "determine best transformation between "
		"two sets of %d points", residuals.size());
	std::vector<Point>	from;
	std::vector<Point>	to;
	std::vector<double>	weights;
	for (auto ptr = residuals.begin(); ptr != residuals.end(); ptr++) {
		Point	f = ptr->from();
		from.push_back(f);
		to.push_back(f + ptr->offset());
		weights.push_back(ptr->weight());
	}
	return build(from, to, weights);
}

/**
 * \brief Build a transform from a set of point pairs
 */
Transform	TransformFactory::operator()(const std::vector<Point>& from,
	const std::vector<Point>& to) {
	std::vector<double>	weights(from.size(), 1.);
	return build(from, to, weights);
}

/**
 * \brief Build a transform from a set of point pairs with weights
 */
Transform	TransformFactory::operator()(const std::vector<Point>& from,
	const std::vector<Point>& to, const std::vector<double>& weights) {
	return build(from, to, weights);
}

/**
 * \brief Build a transform from a set of point pairs with weights
 */
Transform	TransformFactory::build(const std::vector<Point>& from,
	const std::vector<Point>& to, const std::vector<double>& weights) {
	if (rigid()) {
		RigidTransformBuilder	rtb;
		return rtb.build(from, to, weights);
	} else {
		AffineTransformBuilder	atb;
		return atb.build(from, to, weights);
	}
}

} // namespace transform
} // namespace image
} // namespace astro
