/*
 * ConvolutionOperator.cpp -- implementation of the Convolution operator
 *                            base class
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroConvolve.h>

namespace astro {
namespace image {

FourierImagePtr	ConvolutionOperator::operator()(ImagePtr image) const {
	FourierImage	i(image);
	return i * (*_psf);
}

FourierImagePtr	ConvolutionOperator::operator()(FourierImagePtr fourier) const {
	return _psf * fourier;
}

} // namespace image
} // namespace astro
