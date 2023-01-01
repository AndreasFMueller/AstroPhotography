/*
 * AstroImager.h -- Imager definition
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroImager_h
#define _AstroImager_h

#include <AstroImage.h>
#include <AstroCamera.h>
#include <AstroUtils.h>
#include <AstroCallback.h>

using namespace astro::image;
using namespace astro::callback;

namespace astro {
namespace camera {

class Imager {
private:
	ImagePtr	_dark;
public:
	ImagePtr	dark() const { return _dark; }
	void	dark(ImagePtr dark);
	bool	hasDark() const { return (_dark) ? true : false; }

private:
	bool	_darksubtract;
public:
	bool	darksubtract() const { return _darksubtract; }
	void	darksubtract(bool darksubtract) { _darksubtract = darksubtract; }

private:
	ImagePtr	_flat;
public:
	ImagePtr	flat() const { return _flat; }
	void	flat(ImagePtr flat);
	bool	hasFlat() const { return (_flat) ? true : false; }

private:
	bool	_flatdivide;
public:
	bool	flatdivide() const { return _flatdivide; }
	void	flatdivide(bool flatdivide) { _flatdivide = flatdivide; }

private:
	bool	_interpolate;
public:
	bool	interpolate() const { return _interpolate; }
	void	interpolate(bool interpolate) { _interpolate = interpolate; }

private:
	CcdPtr	_ccd;
public:
	CcdPtr	ccd() { return _ccd; }
	CcdPtr	ccd() const { return _ccd; }

public:
	Imager(CcdPtr ccd = CcdPtr());
	~Imager();

	// Image processing
	void	operator()(ImagePtr image);

	// camera interface
	void	startExposure(const Exposure& exposure);
	bool	wait();
	ImagePtr	getImage(bool raw = false);

	// assume control of the interface
	void	controlling(device::Device::controlState_t cs);
	void	release();
};

typedef std::shared_ptr<Imager>	ImagerPtr;

/**
 * \brief Calibration image progress
 */
class CalibrationImageProgress {
public:
	int	imagecount;
	int	imageno;
};
typedef CallbackDataEnvelope<CalibrationImageProgress>	CalibrationImageProgressData;

/**
 * \brief common base class for the work done for darks/flats for the imager
 */
class CalimageWork {
private:
	int	_imagecount;
public:
	int	imagecount() const { return _imagecount; }
	void	imagecount(int n) { _imagecount = n; }
protected:
	int	_imageno;
private:
	CallbackPtr	_callback;
protected:
	void	end();
	void	update();
public:
	void	callback(CallbackPtr e) { _callback = e; }

	CalimageWork() : _imagecount(0) { }
};

/**
 * \brief Thread class to extract a dark image for a CCD
 */
class DarkWork : public CalimageWork {
	double	_exposuretime;
public:
	double	exposuretime() const { return _exposuretime; }
	void	exposuretime(double e) { _exposuretime = e; }
private:
	double	_badpixellimit; // number of std devs for bad pixels
public:
	double	badpixellimit() const { return _badpixellimit; }
	void	badpixellimit(double b) { _badpixellimit = b; }
private:
	ImagePtr	_darkimage;
public:
	ImagePtr	darkimage() const { return _darkimage; }
private:
	CcdPtr	_ccd;
public:
	DarkWork(CcdPtr ccd);
protected:
	ImagePtr	common(astro::thread::ThreadBase& thread);
	void	main(astro::thread::Thread<DarkWork>& thread);
friend class astro::thread::Thread<DarkWork>;
};
typedef std::shared_ptr<DarkWork>	DarkWorkPtr;

typedef astro::thread::Thread<DarkWork>	DarkWorkThread;
typedef std::shared_ptr<DarkWorkThread>	DarkWorkThreadPtr;

/**
 * \brief Thread class to extract a dark image for an Imager and install it
 */
class DarkWorkImager : public DarkWork {
	Imager&	_imager;
public:
	DarkWorkImager(Imager& imager)
		: DarkWork(imager.ccd()), _imager(imager) { }
protected:
	void	main(astro::thread::Thread<DarkWorkImager>& thread);
friend class astro::thread::Thread<DarkWorkImager>;
};
typedef std::shared_ptr<DarkWorkImager>	DarkWorkImagerPtr;

typedef astro::thread::Thread<DarkWorkImager>	DarkWorkImagerThread;
typedef std::shared_ptr<DarkWorkImagerThread>	DarkWorkImagerThreadPtr;

/**
 * \brief Thread class to extract a flat image for a CCD
 */
class FlatWork : public CalimageWork {
	double	_exposuretime;
public:
	double	exposuretime() const { return _exposuretime; }
	void	exposuretime(double e) { _exposuretime = e; }
private:
	ImagePtr	_darkimage;
public:
	ImagePtr	darkimage() const { return _darkimage; }
	void	darkimage(ImagePtr d) { _darkimage = d; }
private:
	ImagePtr	_flatimage;
public:
	ImagePtr	flatimage() const { return _flatimage; }
private:
	CcdPtr	_ccd;
public:
	FlatWork(CcdPtr ccd);
protected:
	ImagePtr	common(astro::thread::ThreadBase& thread);
	void	main(astro::thread::Thread<FlatWork>& thread);
friend class astro::thread::Thread<FlatWork>;
};
typedef std::shared_ptr<FlatWork>	FlatWorkPtr;

typedef astro::thread::Thread<FlatWork>	FlatWorkThread;
typedef std::shared_ptr<FlatWorkThread>	FlatWorkThreadPtr;

/**
 * \brief Thread class to extract a flat image for an Imager and install it
 */
class FlatWorkImager : public FlatWork {
	Imager&	_imager;
public:
	FlatWorkImager(Imager& imager)
		: FlatWork(imager.ccd()), _imager(imager) { }
protected:
	void	main(astro::thread::Thread<FlatWorkImager>& thread);
friend class astro::thread::Thread<FlatWorkImager>;
};
typedef std::shared_ptr<FlatWorkImager>	FlatWorkImagerPtr;

typedef astro::thread::Thread<FlatWorkImager>	FlatWorkImagerThread;
typedef std::shared_ptr<FlatWorkImagerThread>	FlatWorkImagerThreadPtr;

/**
 * \brief Work class for image acquisition through a CCD
 */
class ImageWork {
protected:
	CcdPtr	_ccd;
	Exposure	_exposure;
	ImagePtr	_image;
	CallbackPtr	_endcallback;
public:
	ImageWork(CcdPtr ccd, const Exposure& exposure);
	ImagePtr	image() const { return _image; }
	void	endcallback(CallbackPtr e) { _endcallback = e; }
protected:
	void	main(astro::thread::Thread<ImageWork>& thread);
friend class astro::thread::Thread<ImageWork>;
};
typedef std::shared_ptr<ImageWork>	ImageWorkPtr;
typedef astro::thread::Thread<ImageWork>	ImageWorkThread;
typedef std::shared_ptr<ImageWorkThread>	ImageWorkThreadPtr;

/**
 * \brief Work class for image acquisition through an Imager
 */
class ImageWorkImager {
	Imager&		_imager;
	Exposure	_exposure;
	ImagePtr	_image;
	CallbackPtr	_endcallback;
public:
	ImageWorkImager(Imager& imager, const Exposure& exposure);
	ImagePtr	image() const { return _image; }
	void	endcallback(CallbackPtr e) { _endcallback = e; }
protected:
	void	main(astro::thread::Thread<ImageWorkImager>& thread);
friend class astro::thread::Thread<ImageWorkImager>;
};
typedef std::shared_ptr<ImageWorkImager>	ImageWorkImagerPtr;
typedef astro::thread::Thread<ImageWorkImager>	ImageWorkImagerThread;
typedef std::shared_ptr<ImageWorkImagerThread>	ImageWorkImagerThreadPtr;

} // namespace camera
} // namespace astro

#endif /* _AstroImager_h */
