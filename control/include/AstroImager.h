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
	ImagePtr	_dark;
	bool	_darksubtract;
	ImagePtr	_flat;
	bool	_flatdivide;
	bool	_interpolate;
	CcdPtr	ccd;
public:
	Imager(CcdPtr _ccd = CcdPtr());

	// accessors
	ImagePtr	dark() const { return _dark; }
	void	setDark(ImagePtr dark) { _dark = dark; }

	bool	darksubtract() const { return _darksubtract; }
	void	setDarksubtract(bool darksubtract) {
		_darksubtract = darksubtract;
	}

	ImagePtr	flat() const { return _flat; }
	void	setFlat(ImagePtr flat) { _flat = flat; }

	bool	flatdivide() const { return _flatdivide; }
	void	setFlatdivide(bool flatdivide) {
		_flatdivide = flatdivide;
	}

	bool	interpolate() const { return _interpolate; }
	void	setInterpolate(bool interpolate) {
		_interpolate = interpolate;
	}

	// Image processing
	void	operator()(ImagePtr image);

	// camera interface
	void	startExposure(const Exposure& exposure);
	ImagePtr	getImage();
};

} // namespace camera
} // namespace astro

#endif /* _AstroImager_h */
