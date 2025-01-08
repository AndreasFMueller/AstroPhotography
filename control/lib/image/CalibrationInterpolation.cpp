/**
 * CalibrationInterpolation.cpp -- calibration interpolation class
 *
 * (c) 2025 Prof Dr Andreas Müller
 */

#include <AstroCalibration.h>

namespace astro {
namespace calibration {

/**
 * \brief Template method to perform interpolation in an image
 *
 * If the pixel type is a floating point type, then all NaN values
 * are ignored during interpolation.
 *
 * \param image		the image to interpolate
 * \param x		the x coordinate of the pixel to interpolate
 * \param y 		the y coordinate of the pixel to interpolate
 * \return		the interpolated value
 */
template<typename ImagePixelType>
ImagePixelType	CalibrationInterpolation::pixel(
			ConstImageAdapter<ImagePixelType>& image,
			int x, int y, int interpolation_distance) const {
	ImagePixelType	sum = 0;
	int	counter = 0;
	for (int xi = -interpolation_distance;
		xi <= interpolation_distance; xi++) {
		for (int yi = -interpolation_distance;
			yi <= interpolation_distance; yi++) {
			if ((xi == 0) && (yi == 0))
				continue;
			int	X = x + xi;
			int	Y = y + yi;
			if (!image.getSize().contains(X, Y))
				continue;
			ImagePixelType	v = image.pixel(X, Y);
			if (v == v) {
				sum += v;
				counter++;
			}
		}
	}
	if (counter > 0) {
		sum = (1. / counter) * sum;
	}
	return sum;
}

// ensure that the pixel methods are emitted: explizit template instantiation
template
unsigned char	CalibrationInterpolation::pixel(
			ConstImageAdapter<unsigned char>& image,
			int x, int y, int interpolation_distance) const;
template
unsigned short	CalibrationInterpolation::pixel(
			ConstImageAdapter<unsigned short>& image,
			int x, int y, int interpolation_distance) const;
template
unsigned int	CalibrationInterpolation::pixel(
			ConstImageAdapter<unsigned int>& image,
			int x, int y, int interpolation_distance) const;
template
unsigned long	CalibrationInterpolation::pixel(
			ConstImageAdapter<unsigned long>& image,
			int x, int y, int interpolation_distance) const;
template
float	CalibrationInterpolation::pixel(ConstImageAdapter<float>& image,
			int x, int y, int interpolation_distance) const;
template
double	CalibrationInterpolation::pixel(ConstImageAdapter<double>& image,
			int x, int y, int interpolation_distance) const;


#define	interpolation_typed(ImagePixelType, BadPixelType)		\
{									\
	ImageAdapter<ImagePixelType>	*ipa				\
		= dynamic_cast<ImageAdapter<ImagePixelType>*>(&*image);	\
	ConstImageAdapter<BadPixelType>	*bpa				\
		= dynamic_cast<ConstImageAdapter<BadPixelType>*>(&*badpixels);\
	if ((NULL != ipa) && (NULL != bpa)) {				\
		return interpolate<ImagePixelType, BadPixelType>(*ipa,	\
			*bpa);						\
	}								\
}

size_t	CalibrationInterpolation::operator()(ImagePtr image,
		ImagePtr badpixels) {
	interpolation_typed(unsigned char, float)
	interpolation_typed(unsigned short, float)
	interpolation_typed(unsigned int, float)
	interpolation_typed(unsigned long, float)
	interpolation_typed(float, float)
	interpolation_typed(double, float)
	interpolation_typed(unsigned char, double)
	interpolation_typed(unsigned short, double)
	interpolation_typed(unsigned int, double)
	interpolation_typed(unsigned long, double)
	interpolation_typed(float, double)
	interpolation_typed(double, double)
	// throw an exception
	std::string	msg = astro::stringprintf("unknown type combination "
		"for interpolation: %s, %s",
		image->info().c_str(), badpixels->info().c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

/**
 * \brief Interpolate all bad pixels of an image
 *
 * This private method interpolates the pixels in the first argument image
 * for all pixels that are indicated as bad by the third argument image.
 * It does not respect the _mosaic variable. The public interpolate method
 * takes care of a Bayer grid by calling this method on a subgrid.
 *
 * \param image		the image to interpolate in
 * \param badpixels	an image containing bad pixel information
 */
template<typename ImagePixelType, typename BadPixelType>
size_t	CalibrationInterpolation::private_interpolate(
		ImageAdapter<ImagePixelType>& image,
		ConstImageAdapter<BadPixelType>& badpixels) const {
	ImageSize	size = image.getSize();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "interpolate %s image",
		size.toString().c_str());
	int	interpolation_counter = 0;
	for (int x = 0; x < size.width(); x++) {
		for (int y = 0; y < size.height(); y++) {
			BadPixelType	b = badpixels.pixel(x, y);
			if (b == b)
				continue;
			ImagePixelType	v = pixel(image, x, y, 1);
			image.writablepixel(x, y) = v;
			interpolation_counter++;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d interpolated pixels",
		interpolation_counter);
	return interpolation_counter;
}

// ensure that the pixel methods are emitted: explizit template instantiation
template
size_t	CalibrationInterpolation::private_interpolate(
		ImageAdapter<unsigned char>& image,
		ConstImageAdapter<float>& badpixels) const;
template
size_t	CalibrationInterpolation::private_interpolate(
		ImageAdapter<unsigned short>& image,
		ConstImageAdapter<float>& badpixels) const;
template
size_t	CalibrationInterpolation::private_interpolate(
		ImageAdapter<unsigned int>& image,
		ConstImageAdapter<float>& badpixels) const;
template
size_t	CalibrationInterpolation::private_interpolate(
		ImageAdapter<unsigned long>& image,
		ConstImageAdapter<float>& badpixels) const;
template
size_t	CalibrationInterpolation::private_interpolate(
		ImageAdapter<float>& image,
		ConstImageAdapter<float>& badpixels) const;
template
size_t	CalibrationInterpolation::private_interpolate(
		ImageAdapter<double>& image,
		ConstImageAdapter<float>& badpixels) const;
template
size_t	CalibrationInterpolation::private_interpolate(
		ImageAdapter<unsigned char>& image,
		ConstImageAdapter<double>& badpixels) const;
template
size_t	CalibrationInterpolation::private_interpolate(
		ImageAdapter<unsigned short>& image,
		ConstImageAdapter<double>& badpixels) const;
template
size_t	CalibrationInterpolation::private_interpolate(
		ImageAdapter<unsigned int>& image,
		ConstImageAdapter<double>& badpixels) const;
template
size_t	CalibrationInterpolation::private_interpolate(
		ImageAdapter<unsigned long>& image,
		ConstImageAdapter<double>& badpixels) const;
template
size_t	CalibrationInterpolation::private_interpolate(
		ImageAdapter<float>& image,
		ConstImageAdapter<double>& badpixels) const;
template
size_t	CalibrationInterpolation::private_interpolate(
		ImageAdapter<double>& image,
		ConstImageAdapter<double>& badpixels) const;

/**
 * \brief Interpolate all bad pixels of an image
 *
 * This function interpolates the pixels in the first argument image
 * for all pixels that are indicated as bad by the third argument image.
 * If mosaic is true, then we assume that we have to interpolate values
 * of a Bayer grid.
 *
 * \param image		the image to interpolate in
 * \param badpixels	an image containing bad pixel information
 */
template<typename ImagePixelType, typename BadPixelType>
size_t	CalibrationInterpolation::interpolate(
		ImageAdapter<ImagePixelType>& image, 
		ConstImageAdapter<BadPixelType>& badpixels) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"interpolate in %smosaic image of size %s/%s",
		(mosaic()) ? "" : "non-", image.getSize().toString().c_str(),
		badpixels.getSize().toString().c_str());
	// do the interplation for an image that is not a mosaic
	if (!mosaic()) {
		return private_interpolate(image, badpixels);
	}

	// handle the case of a mosaic
	size_t	interpolation_counter = 0;
	for (int x = 0; x <= 1; x++) {
		for (int y = 0; y <= 1; y++) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"interpolate on (%d,%d) subgrid", x, y);
			image::Subgrid	s(ImagePoint(x, y), ImageSize(2, 2));
			adapter::SubgridAdapter<ImagePixelType>	sa(image, s);
			adapter::ConstSubgridAdapter<BadPixelType>	ba(badpixels, s);
			interpolation_counter += private_interpolate(sa, ba);
		}
	}
	return interpolation_counter;
}

// explizit instantiation of the interpolate
template
size_t  CalibrationInterpolation::interpolate(
                ImageAdapter<unsigned char>& image,
                ConstImageAdapter<float>& badpixels) const;
template
size_t  CalibrationInterpolation::interpolate(
                ImageAdapter<unsigned short>& image,
                ConstImageAdapter<float>& badpixels) const;
template
size_t  CalibrationInterpolation::interpolate(
                ImageAdapter<unsigned int>& image,
                ConstImageAdapter<float>& badpixels) const;
template
size_t  CalibrationInterpolation::interpolate(
                ImageAdapter<unsigned long>& image,
                ConstImageAdapter<float>& badpixels) const;
template
size_t  CalibrationInterpolation::interpolate(
                ImageAdapter<float>& image,
                ConstImageAdapter<float>& badpixels) const;
template
size_t  CalibrationInterpolation::interpolate(
                ImageAdapter<double>& image,
                ConstImageAdapter<float>& badpixels) const;
template
size_t  CalibrationInterpolation::interpolate(
                ImageAdapter<unsigned char>& image,
                ConstImageAdapter<double>& badpixels) const;
template
size_t  CalibrationInterpolation::interpolate(
                ImageAdapter<unsigned short>& image,
                ConstImageAdapter<double>& badpixels) const;
template
size_t  CalibrationInterpolation::interpolate(
                ImageAdapter<unsigned int>& image,
                ConstImageAdapter<double>& badpixels) const;
template
size_t  CalibrationInterpolation::interpolate(
                ImageAdapter<unsigned long>& image,
                ConstImageAdapter<double>& badpixels) const;
template
size_t  CalibrationInterpolation::interpolate(
                ImageAdapter<float>& image,
                ConstImageAdapter<double>& badpixels) const;
template
size_t  CalibrationInterpolation::interpolate(
                ImageAdapter<double>& image,
                ConstImageAdapter<double>& badpixels) const;

} // namespace calibration
} // namespace astro
