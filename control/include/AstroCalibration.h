/*
 * AstroCalibration.h -- generating and applying calibration frames
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroCalibration_h
#define _AstroCalibration_h

#include <AstroImage.h>
#include <AstroAdapter.h>
#include <AstroCamera.h>

namespace astro {
namespace calibration {

/**
 * \brief Class to perform Interpolation for calibration images
 */
class	CalibrationInterpolation {
	bool	_mosaic;
public:
	bool	mosaic() const { return _mosaic; }
	void	mosaic(bool m) { _mosaic = m; }
private:
	template<typename ImagePixelType>
	ImagePixelType	pixel(ConstImageAdapter<ImagePixelType>& image,
				int x, int y, int interpolation_distance) const;
	template<typename ImagePixelType, typename BadPixelType>
	size_t	private_interpolate(ImageAdapter<ImagePixelType>& image,
			ConstImageAdapter<BadPixelType>& badpixels) const;
public:
	CalibrationInterpolation(bool mosaic = false) : _mosaic(mosaic) { }
	size_t	operator()(ImagePtr image, ImagePtr badpixels);

	template<typename ImagePixelType, typename BadPixelType>
	size_t	interpolate(ImageAdapter<ImagePixelType>& image,
			ConstImageAdapter<BadPixelType>& badpixels) const;
};

/**
 * \brief Base class for calibration frame factories
 */
class CalibrationFrameFactory {
	ImagePtr	_report;
public:
	ImagePtr	report() const { return _report; }
protected:
	void	copyMetadata(astro::image::ImagePtr calframe,
			const astro::image::ImageSequence& images,
			const std::string& purpose) const;
public:
	astro::image::ImagePtr	operator()(
		const astro::image::ImageSequence& images) const;
};

template<typename T>
class ImageMean;

/**
 * \brief Factory for dark frames.
 *
 * The dark frame factory creates dark frames, i.e. frames that contain
 * information about the dark current and the reliability of indiviual
 * pixels. It does not address pixel values in relation to neighbouring
 * pixels, that is the domain of the flat frames.
 *
 * A dark frame is an image with a floating point value that is typically
 * the average of the values measurered in a sequence of images handed
 * over to the operator() as an image sequence. If bad pixeldetection 
 * is enabled, then bad pixel values are set to NaN, indicating that
 * this pixel does not have a reliable value.
 */
class DarkFrameFactory : public CalibrationFrameFactory {
	double	_badpixellimitstddevs;
public:
	double	badpixellimitstddevs() const { return _badpixellimitstddevs; }
	void	badpixellimitstddevs(double b) { _badpixellimitstddevs = b; }
private:
	int	_absolute;
public:
	int	absolute() const { return _absolute; }
	void	absolute(int a) { _absolute = a; }
private:
	bool	_interpolate;
public:
	bool	interpolate() const { return _interpolate; }
	void	interpolate(bool i) { _interpolate = i; }
private:
	bool	_detect_bad_pixels;
public:
	bool	detect_bad_pixels() const { return _detect_bad_pixels; }
	void	detect_bad_pixels(bool d) { _detect_bad_pixels = d; }
private:
	template<typename DarkPixelType>
	ImagePtr	dark(const ImageSequence& images) const;

	template<typename DarkPixelType>
	ImagePtr	dark(const ImageSequence& images,
				bool gridded) const;

	template<typename DarkPixelType>
	size_t	subdark(ImageMean<DarkPixelType>& im,
			const Subgrid grid) const;
public:
	// Constructor
	DarkFrameFactory();

	astro::image::ImagePtr	operator()(
		const astro::image::ImageSequence& images) const;
};

/**
 * \brief Factory for flat frames.
 *
 * Flat frames are images of a floating point type with values <= 1
 * that can be used to fix sensitivity differences of a sensor.
 * In contrast to the dark image, the flat image contains information
 * on how pixels relate to other pixels of the sensor. So for a 
 * complete calibration, the dark pixel values need to be subtracted,
 * ignoring the nans in the dark, and then the values need to be
 * devided by the flat value, except for the nan values, where interpolation
 * is needed.
 */
class FlatFrameFactory : public CalibrationFrameFactory {
	bool	_mosaic;
public:
	bool	mosaic() const { return _mosaic; }
	void	mosaic(bool m) { _mosaic = m; }
private:
	bool	_interpolate;
public:
	bool	interpolate() const { return _interpolate; }
	void	interpolate(bool i) { _interpolate = i; }
private:
	ImagePtr	flat(const astro::image::ImageSequence& images) const;

	template<typename FlatPixelType>
	ImagePtr	flat(const astro::image::ImageSequence& images,
		const Image<FlatPixelType>& bias) const;
public:
	FlatFrameFactory(bool mosaic = false, bool interpolate = false);

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
 *
 * The corrector class is used to correct an image based on a calibration
 * image. Derived classes implement the correction methods for dark frames
 * or for flat frames.
 *
 * The correction is done by the virtual operator() method. It performs
 * whatever
 * 
 */
class Corrector {
	size_t	_badpixels;
protected:
	astro::image::ImagePtr	calibrationimage;
	astro::image::ImageRectangle	rectangle;
public:
	size_t	badpixels() const { return _badpixels; }
	Corrector(astro::image::ImagePtr calibrationimage,
		const astro::image::ImageRectangle rectangle
			= astro::image::ImageRectangle());
	virtual void	operator()(astro::image::ImagePtr image,
				const int interpolation_distance = 0) const;
};

/**
 * \brief Correct dark correction
 *
 * The dark correction subtracts the values of the dark frame from the
 * image. If there are NaNs in the dark frame, then the corrector will
 * interpolate pixels based on the interpolation_distance argument,
 * this can be turned off by setting the interpolation_distance to 0.
 */
class DarkCorrector : public Corrector {
public:
	DarkCorrector(astro::image::ImagePtr dark,
		const astro::image::ImageRectangle rectangle
			= astro::image::ImageRectangle());
	virtual void	operator()(astro::image::ImagePtr image,
				const int interpolation_distance = 0) const;
};

/**
 * \brief Perform flat correction
 */
class FlatCorrector : public Corrector {
public:
	FlatCorrector(astro::image::ImagePtr flat,
		const astro::image::ImageRectangle rectangle
			= astro::image::ImageRectangle());
	virtual void	operator()(astro::image::ImagePtr image,
				const int interpolation_distance = 0) const;
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
