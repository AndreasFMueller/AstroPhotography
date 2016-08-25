/*
 * aotest.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <includes.h>
#include <AstroDebug.h>
#include <AstroUtils.h>
#include "SxAO.h"

using namespace astro::camera::sx;

namespace astro {

/**
 * \brief Inverse error function
 */
double	inverf(double y) {
	if ((y < -1) || (y > 1)) {
		throw std::range_error("impossible erf(x) value");
	}
	double	xneu, x = y;
	double	delta;
	int	counter = 0;
	do {
#define	C	(2 / sqrt(M_PI))
		xneu = x - (erf(x) - y) / (C * exp(-x * x));
		delta = fabs(x - xneu);
		x = xneu;
	} while ((delta > 0.00000001) && (counter++ < 100));
	if (counter >= 100) {
		throw std::runtime_error("inverf did not converge");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "erf^{-1}(%f) = %f", y, xneu);
	return xneu;
}

double	Finv(double y) {
	return sqrt(2) * inverf(2 * y - 1);
}

/**
 * \brief Get a normally distributed random number between -1 and 1
 *
 * This method uses erf defined in math.h and newtons algorithm to find
 * the inverse of the cumulative distribution function (cdf) of the
 * normal distribution with 
 */
static double	randcoord(double s = 1) {
	double	x = 0;
	do {
		double	y = random() / 2147483648.;
debug(LOG_DEBUG, DEBUG_LOG, 0, "y = %f", y);
		x = s * sqrt(2) * inverf(2 * y - 1);
	} while ((x < -1) || (1 < x));
	return x;
}

int	aotest_main(int argc, char *argv[]) {
	debugtimeprecision = 3;
	debugthreads = 1;
	int	c;
	bool	randpattern = false;
	bool	grid = false;
	while (EOF != (c = getopt(argc, argv, "dgr")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'r':
			randpattern = true;
			break;
		case 'g':
			grid = true;
			break;
		}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adaptive optics test started");

	// create the object
	std::string	sname("adaptiveoptics:sx/0");
	DeviceName	devicename(sname);
	SxAO	ao(devicename);

	// try positioning on a grid
	double	elapsed = 0;
	int	operations = 0;

	if (grid) {
		float	x, y;
		for (x = -1; x <= 1; x += 0.25) {
			for (y = -1; y <= 1; y += 0.25) {
				Timer	t;
				t.start();
				ao.set(Point(x, y));
				t.end();
				elapsed += t.elapsed();
				debug(LOG_DEBUG, DEBUG_LOG, 0, "elapsed: %f",
					t.elapsed());
				operations++;
				usleep(100000);
			}
		}
	}

	if (randpattern) {
		float	x, y;
		for (int i = 0; i < 100; i++) {
			x = randcoord(0.1);
			y = randcoord(0.1);
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"random point %d: (%f,%f)", i, x, y);
			Timer	t;
			t.start();
			ao.set(Point(x, y));
			t.end();
			elapsed += t.elapsed();
			debug(LOG_DEBUG, DEBUG_LOG, 0, "elapsed: %f",
				t.elapsed());
			operations++;
			usleep(100000);
		}
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "average operation time: %f",
		elapsed / operations);
	ao.set(Point(0, 0));

	return EXIT_SUCCESS;
}

} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::aotest_main(argc, argv);
	} catch (std::exception& x) {
		std::cerr << "aotest failed: " << x.what() << std::endl;
	}
}
