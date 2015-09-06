/*
 * AstroConvolve.h -- methods and classes to convolve images e.g. with
 *                    point spread functions
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroConvolve_h
#define _AstroConvolve_h

#include <AstroImage.h>
#include <AstroTypes.h>
#include <fftw3.h>

namespace astro {
namespace image {

class FourierImage;
typedef std::shared_ptr<FourierImage>	FourierImagePtr;

/**
 * \brief Fourier Image
 *
 * This class contains the fourier transform of an image
 */
class FourierImage : public Image<double> {
	ImageSize	_orig;
public:
	const ImageSize&	orig() const { return _orig; }
private:
	void	fourier(const Image<double>& image);
public:
static ImageSize	fsize(const ImageSize&);
	FourierImage(const ImageSize& size);
	FourierImage(const Image<double>& image);
	FourierImage(const ImagePtr image);
	ImagePtr	inverse() const;
	ImagePtr	abs() const;
	ImagePtr	phase() const;
	ImagePtr	color() const;
};

/**
 * \brief multiply two fourier transforms
 */
FourierImagePtr	operator*(const FourierImage& a, const FourierImage& b);
FourierImagePtr	operator*(const FourierImagePtr a, const FourierImagePtr b);

class ConvolutionResult;
typedef std::shared_ptr<ConvolutionResult>	ConvolutionResultPtr;

/**
 * \brief class to encode convolution results
 */
class ConvolutionResult : public FourierImage {
	Point	_center;
public:
	const Point	center() const { return _center; }
	void	center(const Point& c) { _center = c; }
public:
	ConvolutionResult(const ImageSize& size, const Point& center);
	ConvolutionResult(const Image<double>& image, const Point& center);
	ConvolutionResult(const ImagePtr image, const Point& center);
	ImagePtr	image() const;
};

ConvolutionResultPtr	operator*(const ConvolutionResult& a,
				const ConvolutionResult& b);
ConvolutionResultPtr	operator*(const ConvolutionResultPtr a,
				const ConvolutionResultPtr b);
ConvolutionResultPtr	operator*(const ConvolutionResultPtr a,
				const ConvolutionResult b);
ConvolutionResultPtr	operator*(const ConvolutionResult a,
				const ConvolutionResultPtr b);

/**
 * \brief Base class for convolution operators
 */
class ConvolutionOperator {
	Point	_center;
	ConvolutionResultPtr	_psf;
public:
	const Point	center() const { return _center; }
	void	center(const Point& c) { _center = c; }
public:
	FourierImagePtr	operator()(ImagePtr image) const;
	FourierImagePtr	operator()(FourierImagePtr fourier) const;
};

/**
 * \brief Airy disk image
 */
class AiryImage : public ConstImageAdapter<double> {
	ImagePoint	_center;
	double	_a;
	double	_k;
	double	_angularpixelsize; //
public:
	AiryImage(const ImageSize& size, const ImagePoint& center, double a,
		double angularpixelsize,
		double lambda = 550e-9 /* wavelength in meters */);
	virtual double	pixel(int x, int y) const;
};

/**
 * \brief Gauss disk image
 */
class GaussImage : public ConstImageAdapter<double> {
	ImagePoint	_center;
	double	_sigma;
	double	_angularpixelsize; 
	double	_n;
public:
	GaussImage(const ImageSize& size, const ImagePoint& center,
		double sigma, double angularpixelsize);
	virtual double	pixel(int x, int y) const;
};
 
/**
 * \brief circular disk image
 */
class DiskImage : public ConstImageAdapter<double> {
	ImagePoint	_center;
	double	_angularpixelsize; 
	double	_r;
	double	_interiorvalue;
public:
	DiskImage(const ImageSize& size, const ImagePoint& center,
		double r, double angularpixelsize);
	virtual double	pixel(int x, int y) const;
};

/**
 * \brief ring image
 */
class RingImage : public ConstImageAdapter<double> {
	ImagePoint	_center;
	double	_angularpixelsize; 
	double	_r1;
	double	_r2;
	double	_interiorvalue;
public:
	RingImage(const ImageSize& size, const ImagePoint& center,
		double r1, double r2, double angularpixelsize);
	virtual double	pixel(int x, int y) const;
};

} // namespace image
} // namespace astro

#endif /* _AstroConvolve_h */
