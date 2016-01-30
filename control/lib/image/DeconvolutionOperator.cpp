/*
 * DeconvolutionOperator.cpp -- Base class implementation for deconvolution
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroConvolve.h>

namespace astro {
namespace image {

ImagePtr	DeconvolutionOperator::operator()(ImagePtr image) const {
	FourierImagePtr	f = FourierImagePtr(new FourierImage(image));
	return this->operator()(f);
}

ImagePtr	DeconvolutionOperator::operator()(FourierImagePtr image) const {
	return this->operator()(image->inverse());
}

} // namespace image
} // namespace astro
