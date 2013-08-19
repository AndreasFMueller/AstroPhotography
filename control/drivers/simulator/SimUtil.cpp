/*
 * SimUtil.cpp -- utilites for the simulator
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SimUtil.h>
#include <includes.h>

namespace astro {
namespace camera {
namespace simulator {

double	simtiem() {
        struct timeval  tv;
        gettimeofday(&tv, NULL);
        double  result = tv.tv_sec;
        result += 0.0000001 * tv.tv_usec;
        return result;
}

} // namespace simulator
} // namespace camera
} // namespace astro
