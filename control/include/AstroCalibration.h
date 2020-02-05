/*
 * AstroCalibration.h -- generating and applying calibration frames
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroCalibration_h
#define _AstroCalibration_h

#include <AstroImage.h>
#include <AstroCamera.h>

namespace astro {
namespace calibration {

/**
 * \brief Base class for calibration frame factories
 */
class CalibrationFrameFactory {
	ImagePtr	_report;
protected:
	virtual void	copyMetadata(astro::image::ImagePtr calframe,
				astro::image::ImagePtr firstimage) const;
public:
	ImagePtr	report() const { return _report; }
	astro::image::ImagePtr	operator()(
		const astro::image::ImageSequence& images) const;
};

/**
 * \brief Factory for dark frames.
 *
 * 
 */
class DarkFrameFactory : public CalibrationFrameFactory {
	double	_badpixellimit;
protected:
	virtual void	copyMetadata(astro::image::ImagePtr calframe,
				astro::image::ImagePtr firstimage) const;
public:
	double	badpixellimit() const { return _badpixellimit; }
	void	badpixellimit(double b) { _badpixellimit = b; }
	DarkFrameFactory(double badpixellimit = 3)
		: _badpixellimit(badpixellimit) {
	}
	astro::image::ImagePtr	operator()(
		const astro::image::ImageSequence& images) const;
};

/**
 * \brief Factory for flat frames.
 *
 *
 */
class FlatFrameFactory : public CalibrationFrameFactory {
protected:
	virtual void	copyMetadata(astro::image::ImagePtr calframe,
				astro::image::ImagePtr firstimage) const;
public:
	astro::image::ImagePtr	operator()(
				const astro::image::ImageSequence& images,
				const astro::image::ImagePtr biasimage) const;
};

/**
 * \brief Clamp an image
 */
class Clamper {
	double minvalue;
	double maxvalue;
public:
	Clamper(double minvalue, double maxvalue);
	void	operator()(astro::image::ImagePtr image) const;
};

/**
 * \brief Strech an image
 */
class Stretcher {
public:
	Stretcher();
	void	operator()(astro::image::ImagePtr image) const;
};

/**
 * \brief Corrector class
 */
class Corrector {
protected:
	astro::image::ImagePtr	calibrationimage;
	astro::image::ImageRectangle	rectangle;
public:
	Corrector(astro::image::ImagePtr calibrationimage,
		const astro::image::ImageRectangle rectangle
			= astro::image::ImageRectangle());
	virtual void	operator()(astro::image::ImagePtr image) const = 0;
};

/**
 * \brief Correct dark correction
 */
class DarkCorrector : public Corrector {
public:
	DarkCorrector(astro::image::ImagePtr dark,
		const astro::image::ImageRectangle rectangle
			= astro::image::ImageRectangle());
	virtual void	operator()(astro::image::ImagePtr image) const;
};

/**
 * \brief Perform flat correction
 */
class FlatCorrector : public Corrector {
public:
	FlatCorrector(astro::image::ImagePtr flat,
		const astro::image::ImageRectangle rectangle
			= astro::image::ImageRectangle());
	virtual void	operator()(astro::image::ImagePtr image) const;
};

/**
 * \brief Calibrate using a dark frame and a flat frame
 */
class Calibrator {
	astro::image::ImagePtr	dark;
	astro::image::ImagePtr	flat;
	astro::image::ImageRectangle	rectangle;
public:
	Calibrator(astro::image::ImagePtr dark,
		astro::image::ImagePtr flat,
		const astro::image::ImageRectangle rectangle
			= astro::image::ImageRectangle());
	astro::image::ImagePtr	operator()(const astro::image::ImagePtr image) const;
};

/**
 * \brief Class to record and average calibration images
 */
class CalibrationFrameProcess {
protected:
	astro::camera::CcdPtr	ccd;
	astro::camera::Exposure	exposure;
	Temperature	_temperature;
	unsigned int	_nimages;
	void	prepare();
	void	cleanup();
public:
	CalibrationFrameProcess(astro::camera::CcdPtr _ccd) : ccd(_ccd) {
		_temperature = -1;
		_nimages = 3;
	}
	double	exposuretime() const {
		return exposure.exposuretime();
	}
	void	setExposuretime(const float exposuretime) {
		exposure.exposuretime(exposuretime);
	}

	Temperature	temperature() const { return _temperature; }
	void	setTemperature(const Temperature& t) { _temperature = t; }
	unsigned int	nimages() const { return _nimages; }
	void	setNimages(unsigned int nimages) { _nimages = nimages; }

	virtual astro::image::ImagePtr	get() = 0;
};

class DarkFrameProcess : public CalibrationFrameProcess {
public:
	DarkFrameProcess(astro::camera::CcdPtr _ccd)
		: CalibrationFrameProcess(_ccd) { }
	virtual astro::image::ImagePtr	get();
};

class FlatFrameProcess : public CalibrationFrameProcess {
	const astro::image::ImagePtr	dark;
public:
	FlatFrameProcess(astro::camera::CcdPtr _ccd,
		const astro::image::ImagePtr _dark)
		: CalibrationFrameProcess(_ccd), dark(_dark) { }
	virtual astro::image::ImagePtr	get();
};



} // namespace calibration
} // namespace astro

#endif /* _AstroCalibration_h */
