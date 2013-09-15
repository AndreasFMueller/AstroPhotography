/*
 * AstroImager.h -- Imager definition
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroImager_h
#define _AstroImager_h

#include <AstroImage.h>
#include <AstroCamera.h>

using namespace astro::image;

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

public:
	Imager(CcdPtr ccd = CcdPtr());

	// Image processing
	void	operator()(ImagePtr image);

	// camera interface
	void	startExposure(const Exposure& exposure);
	ImagePtr	getImage();
};

} // namespace camera
} // namespace astro

#endif /* _AstroImager_h */
