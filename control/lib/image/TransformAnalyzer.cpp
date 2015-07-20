/*
 * TransformAnalzer.cpp -- TransformAnalyzer implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTransform.h>
#include <AstroAdapter.h>

using namespace astro::adapter;

namespace astro {
namespace image {
namespace transform {

/**
 * \brief Get the transform from the base image to the argument image
 */
Transform	TransformAnalyzer::transform(
			const ConstImageAdapter<double>& image) const {
	std::vector<Residual>	residuals = (*this)(image);

	return Transform(residuals);
}

} // namespace transform
} // namepace image
} // namespace astro
