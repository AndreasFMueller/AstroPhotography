/*
 * AstroCalibration.h -- generating and applying calibration frames
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroCalibration_h
#define _AstroCalibration_h

#include <AstroImage.h>

namespace astro {
namespace calibration {

/**
 * \brief Base class for calibration frame factories
 */
class CalibrationFrameFactory {
protected:
public:
astro::image::ImagePtr	operator()(const astro::image::ImageSequence& images) const;
};

/**
 * \brief Factory for dark frames.
 *
 * 
 */
class DarkFrameFactory : public CalibrationFrameFactory {
public:
astro::image::ImagePtr	operator()(const astro::image::ImageSequence& images) const;
};

/**
 * \brief Factory for flat frames.
 *
 *
 */
class FlatFrameFactory : public CalibrationFrameFactory {
public:
astro::image::ImagePtr	operator()(const astro::image::ImageSequence& images,
	const astro::image::ImagePtr& darkimage) const;
};

/**
 * \brief Calibrate using a dark frame and a flat frame
 */
class Calibrator {
	const astro::image::ImagePtr&	dark;
	const astro::image::ImagePtr&	flat;
public:
	Calibrator(const astro::image::ImagePtr& dark,
		const astro::image::ImagePtr& flat);
	astro::image::ImagePtr	operator()(const astro::image::ImagePtr& image) const;
};

/**
 * \brief Using a dark image, interpolate bad pixelsin an image
 */
class Interpolator {
	const astro::image::ImagePtr& dark;
	astro::image::Image<float>	*floatdark;
	astro::image::Image<double>	*doubledark;
public:
	Interpolator(const astro::image::ImagePtr& dark);
	void	interpolate(astro::image::ImagePtr& image);
	astro::image::ImagePtr	operator()(const astro::image::ImagePtr& image);
};

} // namespace calibration
} // namespace astro

#endif /* _AstroCalibration_h */
