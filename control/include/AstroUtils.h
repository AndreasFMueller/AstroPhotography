/*
 * AstroUtils.h -- some utility classes
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroUtils_h
#define _AstroUtils_h

namespace astro {

/**
 * \brief Timer class
 *
 * Some processes, in particular the SX driver, need to know exactly how long
 * a given process takes. In the SX driver for the M26C camera this is used
 * to correct the two fields for exposure differences.
 */
class Timer {
	double	startTime;
	double	endTime;
	double	gettime();
public:
	Timer();
	void	start();
	void	end();
	double	elapsed();
};

} // namespace astro

#endif /* _AstroUtils_h */
