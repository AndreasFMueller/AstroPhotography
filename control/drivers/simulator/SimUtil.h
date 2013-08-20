/*
 * SimUtil.h -- utilites for the simulator
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SimUtil_h
#define _SimUtil_h

namespace astro {
namespace camera {
namespace simulator {

double	simtime();
void	simtime_advance(double delta);

} // namespace simulator
} // namespace camera
} // namespace astro

#endif /* _SimUtil_h */
