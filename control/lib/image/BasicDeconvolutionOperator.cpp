/*
 * BasicDeconvolutionOperator.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroConvolve.h>

namespace astro {
namespace image {

BasicDeconvolutionOperator::BasicDeconvolutionOperator(ImagePtr image) {
	_psf = FourierImagePtr(new FourierImage(image));
}

BasicDeconvolutionOperator::BasicDeconvolutionOperator(const ConstImageAdapter<double>& image) {
	_psf =
	_psf = FourierImagePtr(new FourierImage(image));
}

ImagePtr	BasicDeconvolutionOperator::operator()(ImagePtr image) const {
	FourierImage	*fimage = new FourierImage(image);
	FourierImagePtr	fimageptr = FourierImagePtr(fimage);
	return (fimageptr / _psf)->inverse();
}

} // namespace image
} // namespace astro
