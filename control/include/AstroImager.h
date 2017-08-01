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
	void	dark(ImagePtr dark) { _dark = dark; }

private:
	bool	_darksubtract;
public:
	bool	darksubtract() const { return _darksubtract; }
	void	darksubtract(bool darksubtract) { _darksubtract = darksubtract; }

private:
	ImagePtr	_flat;
public:
	ImagePtr	flat() const { return _flat; }
	void	flat(ImagePtr flat) { _flat = flat; }

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

	// Image processing
	void	operator()(ImagePtr image);

	// camera interface
	void	startExposure(const Exposure& exposure);
	bool	wait();
	ImagePtr	getImage(bool raw = false);
};

typedef std::shared_ptr<Imager>	ImagerPtr;

/**
 * \brief Thread class to extract a dark image for a CCD
 */
class DarkWork {
	double	_exposuretime;
public:
	double	exposuretime() const { return _exposuretime; }
	void	exposuretime(double e) { _exposuretime = e; }
private:
	int	_imagecount;
public:
	int	imagecount() const { return _imagecount; }
	void	imagecount(int n) { _imagecount = n; }
private:
	CallbackPtr	_endCallback;
protected:
	void	end();
public:
	void	endCallback(CallbackPtr e) { _endCallback = e; }
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
