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
 * \brief Clamp an image
 */
class Clamper {
	double minvalue;
	double maxvalue;
public:
	Clamper(double minvalue, double maxvalue);
	void	operator()(astro::image::ImagePtr& image) const;
};

/**
 * \brief Strech an image
 */
class Stretcher {
public:
	Stretcher();
	void	operator()(astro::image::ImagePtr& image) const;
};

/**
 * \brief Correct dark correction
 */
class DarkCorrector  {
	const astro::image::ImagePtr&	dark;
public:
	DarkCorrector(const astro::image::ImagePtr& dark);
	void	operator()(astro::image::ImagePtr& image) const;
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

} // namespace calibration
} // namespace astro

#endif /* _AstroCalibration_h */
