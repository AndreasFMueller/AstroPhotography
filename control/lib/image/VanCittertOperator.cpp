/*
 * VanCittertOperator.cpp -- implementation 
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroConvolve.h>
#include <AstroAdapter.h>
#include <AstroDebug.h>

namespace astro {
namespace image {

#define psf_typed(psf, Pixel)						\
{									\
	Image<Pixel >	*img = dynamic_cast<Image<Pixel > *>(&*psf);	\
	if (NULL != img) {						\
		adapter::LuminanceAdapter<Pixel, double> la(*img);	\
		_psf = la;						\
		copied = true;						\
	}								\
}

VanCittertOperator::VanCittertOperator(ImagePtr psf)
	: _psf(psf->size()) {
	bool	copied = false;
	// copy the data from the argument psf, this depends on the type
	psf_typed(psf, unsigned char)
	psf_typed(psf, unsigned short)
	psf_typed(psf, unsigned int)
	psf_typed(psf, unsigned long)
	psf_typed(psf, float)
	psf_typed(psf, double)
	psf_typed(psf, RGB<unsigned char>)
	psf_typed(psf, RGB<unsigned short>)
	psf_typed(psf, RGB<unsigned int>)
	psf_typed(psf, RGB<unsigned long>)
	psf_typed(psf, RGB<float>)
	psf_typed(psf, RGB<double>)
	psf_typed(psf, YUYV<unsigned char>)
	psf_typed(psf, YUYV<unsigned short>)
	psf_typed(psf, YUYV<unsigned int>)
	psf_typed(psf, YUYV<unsigned long>)
	psf_typed(psf, YUYV<float>)
	psf_typed(psf, YUYV<double>)
	if (!copied) {
		throw std::runtime_error("no acceptable pixel type");
	}

	// ensure that the sum of all elements of the PSF is 1
	double	sum = 0;
	for (int x = 0; x < _psf.getSize().width(); x++) {
		for (int y = 0; y < _psf.getSize().height(); y++) {
			sum += _psf.pixel(x, y);
		}
	}
	for (int x = 0; x < _psf.getSize().width(); x++) {
		for (int y = 0; y < _psf.getSize().height(); y++) {
			_psf.pixel(x, y) = -_psf.pixel(x, y) / sum;
		}
	}
	int	w2 = _psf.getSize().width() / 2;
	int	h2 = _psf.getSize().height() / 2;
	_psf.pixel(w2, h2) = 1 + _psf.pixel(w2, h2);
}

#define summand_typed(Pixel) 						\
{									\
	Image<Pixel >	*img = dynamic_cast<Image<Pixel >*>(&*a1);	\
	if (NULL != img) {						\
		a = new adapter::LuminanceAdapter<Pixel, double>(*img);	\
	}								\
}

static ImagePtr	operator+(ImagePtr a1, ImagePtr a2) {
	Image<double>	*b = dynamic_cast<Image<double>*>(&*a2);
	if (NULL == b) {
		throw std::runtime_error("bad pixel type in second operand");
	}
	ConstImageAdapter<double>	*a = NULL;
	summand_typed(unsigned char)
	summand_typed(unsigned short)
	summand_typed(unsigned int)
	summand_typed(unsigned long)
	summand_typed(float)
	summand_typed(double)
	summand_typed(RGB<unsigned char>)
	summand_typed(RGB<unsigned short>)
	summand_typed(RGB<unsigned int>)
	summand_typed(RGB<unsigned long>)
	summand_typed(RGB<float>)
	summand_typed(RGB<double>)
	summand_typed(YUYV<unsigned char>)
	summand_typed(YUYV<unsigned short>)
	summand_typed(YUYV<unsigned int>)
	summand_typed(YUYV<unsigned long>)
	summand_typed(YUYV<float>)
	summand_typed(YUYV<double>)
	if (NULL == a) {
		throw std::runtime_error("bad pixel type in first operand");
	}
	adapter::AddAdapter<double>	aa(*b, *a);
	ImagePtr	result = ImagePtr(new Image<double>(aa));
	delete a;
	return result;
}

ImagePtr	VanCittertOperator::operator()(ImagePtr image) const {
// implementation currently missing
#if 0
	Image<double>	*psf = dynamic_cast<Image<double> *>(*&_psf);
	ImagePtr	g = f;
	int	i = 0;
	while (i--) {
		g = image + smallConvolve(*psf, g);
	}
	return g;
#endif
	return image;
}

} // namespace astro
} // namespace image
