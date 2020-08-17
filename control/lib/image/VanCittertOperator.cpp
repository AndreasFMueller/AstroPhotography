/*
 * VanCittertOperator.cpp -- implementation 
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroConvolve.h>
#include <AstroAdapter.h>
#include <AstroDebug.h>
#include <AstroIO.h>
#include <AstroImageops.h>

namespace astro {
namespace image {

/**
 * \brief Construct a VanCittert operator
 *
 * The constructor takes the point spread function image an turns it into
 * a luminance only image with double pixels.
 *
 * \param psf	the point spread function
 */
VanCittertOperator::VanCittertOperator(ImagePtr psf)
	: DeconvolutionOperator(psf) {
	// ensure that the sum of all elements of the PSF is 1
	double	sum = 0;
	for (int x = 0; x < _psf.getSize().width(); x++) {
		for (int y = 0; y < _psf.getSize().height(); y++) {
			double	v = _psf.pixel(x, y);
			if (v < 0) {
				debug(LOG_DEBUG, DEBUG_LOG, 0, "negative pixel "
					"%f at (%d,%d), replaced", v, x, y);
				v = 0;
				_psf.pixel(x, y) = 0;
			}
			sum += v;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "the L^1-norm is %f", sum);
	for (int x = 0; x < _psf.getSize().width(); x++) {
		for (int y = 0; y < _psf.getSize().height(); y++) {
			_psf.pixel(x, y) = -_psf.pixel(x, y) / sum;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "normalized");
	ImagePoint	center = _psf.getSize().center();
	_psf.pixel(center) = 1 + _psf.pixel(center);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "van Cittert kernel ready");
}

template<typename Pixel>
class SumAdapter : public ConstImageAdapter<double> {
	const ConstImageAdapter<Pixel>&		_op1;
	const ConstImageAdapter<double>&	_op2;
public:
	SumAdapter(const ConstImageAdapter<Pixel>& op1,
		const ConstImageAdapter<double>& op2)
		: ConstImageAdapter<double>(op1.getSize()),
		  _op1(op1), _op2(op2) {
		if (op1.getSize() != op2.getSize()) {
			std::string	msg = stringprintf("%s and %s differ "
				"in size", op1.getSize().toString().c_str(),
				op2.getSize().toString().c_str());
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
	}
	virtual double	pixel(int x, int y) const {
		double	v = _op1.pixel(x, y);
		v += _op2.pixel(x, y);
		return v;
	}
};

#define summands_typed(Pixel) 						\
{									\
	Image<Pixel >	*img1 = dynamic_cast<Image<Pixel >*>(&*a1);	\
	if (NULL != img1) {						\
		a1type = demangle_string(*img1);			\
		debug(LOG_DEBUG, DEBUG_LOG, 0, "image1: %s",		\
			a1type.c_str());				\
	}								\
	Image<double >	*img2 = dynamic_cast<Image<double >*>(&*a2);	\
	if (NULL != img2) {						\
		a2type = demangle_string(*img2);			\
		debug(LOG_DEBUG, DEBUG_LOG, 0, "image2: %s",		\
			a2type.c_str());				\
	}								\
	if ((NULL != img1) && (NULL != img2)) {				\
		SumAdapter<Pixel>	sa(*img1, *img2);		\
		return ImagePtr(new Image<Pixel>(sa));			\
	}								\
}

/**
 * \brief Addition operator needed for Vancittert operation
 *
 * We assume here that both images are of the same size and type
 *
 * \param a1	the original image, can have any type
 * \param a2	the convoluted image, must have type Image<double>
 */
ImagePtr	VanCittertOperator::add(ImagePtr a1, ImagePtr a2) const {
	std::string	a1type;
	std::string	a2type;
	summands_typed(unsigned char)
	summands_typed(unsigned short)
	summands_typed(unsigned int)
	summands_typed(unsigned long)
	summands_typed(float)
	summands_typed(double)
	summands_typed(RGB<unsigned char>)
	summands_typed(RGB<unsigned short>)
	summands_typed(RGB<unsigned int>)
	summands_typed(RGB<unsigned long>)
	summands_typed(RGB<float>)
	summands_typed(RGB<double>)
	summands_typed(YUYV<unsigned char>)
	summands_typed(YUYV<unsigned short>)
	summands_typed(YUYV<unsigned int>)
	summands_typed(YUYV<unsigned long>)
	summands_typed(YUYV<float>)
	summands_typed(YUYV<double>)
	// The add adapter allows to add the two images
	std::string	msg = stringprintf("type mismatch: %s vs %s",
		a1type.c_str(), a2type.c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

/**
 * \brief Deconvolve an image using the Van Cittert deconvolution algorithm
 *
 * \param image		The image to deconvolve
 */
ImagePtr	VanCittertOperator::operator()(ImagePtr image) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "deconvolving %s image in %d iterations",
		image->size().toString().c_str(), _iterations);
	// start with the input image
	ImagePtr	g = image;
	int	i = _iterations;
	while (i--) {
		int	number = _iterations - i;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "iteration %d", number);
		g = add(image, smallConvolve(_psf, g));
		if (_constrained) {
			ops::positive(g);
		}
		if (prefix().size() > 0) {
			std::string	filename = stringprintf("%s-%02d.fits",
				prefix().c_str(), number);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "writing %s image to %s",
				g->size().toString().c_str(), filename.c_str());
			io::FITSout	out(filename);
			out.setPrecious(false);
			out.write(g);
		}
	}
	// done
	return g;
}

} // namespace astro
} // namespace image
