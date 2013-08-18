//
// Sun.h -- class to compute sunrise and sunset times for a given longitude,
//          latitude and elevation above (or below, if negative) the horizon
//
// (c) 2004 Dr. Andreas Mueller, Beratung und Entwicklung
// $Id: Sun.h 1533 2006-05-07 20:34:33Z afm $
//
#ifndef _Sun_h
#define _Sun_h

#include <time.h>

namespace astro {

class Sun {
	double	longitude, latitude, elevation;
	int	lastday, lastmonth, lastyear;
	double	rise, set;
public:
	// construction/destruction
	Sun(double lon = 0.0, double lat = 0.0, double ele = 0.0);
	~Sun(void);

	// compute sunrise and sunset for a given day
	time_t	sunrise(time_t when);
	time_t	sunset(time_t when);
private:
	void	compute(time_t when);
};

} // namespace astro

#endif /* _Sun_h */
