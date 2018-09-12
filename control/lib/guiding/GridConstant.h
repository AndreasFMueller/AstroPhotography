/*
 * GridConstant.h
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _GridConstant_h
#define _GridConstant_h

/**
 * \brief grid computation tools
 *
 * This class contains all the necessary conversions from displacments
 * in pixels and angles (radians and arc seconds) to activation times for
 * the guide port.
 */
namespace astro {
namespace guiding {

class GridConstant {
	double	_focallength;	// [m]
	double	_pixelsize;	// [m]
	double	_guiderate;	// guiding speed relative to siderial rate
public:
	void	focallength(double f) { _focallength = f; }
	double	focallength() const { return _focallength; }

	void	pixelsize(double p) { _pixelsize = p; }
	double	pixelsize() const { return _pixelsize; }

	void	guiderate(double g) { _guiderate = g; }
	double	guiderate() const { return _guiderate; }

	double	angle_per_pixel() const;	// [radians/pixel]
	double	angle_per_second() const;	// [radians/s]
	double	pixels_per_angle() const; 	// [pixels/radians]
	double	pixels_per_second() const;	// [pixels/s]
	double	arcsec_per_pixel() const;
	double	arcsec_per_second() const;

	GridConstant(double focallength, double pixelsize);
	static double	pixelsize_from_arcsec(double focallength,
		double arcsec);
	static double	pixelsize_from_angle(double focallength,
		double angle);

	double	suggested_arcsec(double arcseconds) const;	// [s]
	double	suggested_pixel(double pixels) const;		// [s]
	double	operator()(double pixels) const;		// [s]
};

} // namespace guiding
} // namespace astro

#endif /* _GridConstant_h */
