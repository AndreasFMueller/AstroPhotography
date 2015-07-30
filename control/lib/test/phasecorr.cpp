/*
 * phasecorr.cpp -- perform phase correlation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDebug.h>
#include <AstroTransform.h>
#include <includes.h>
#include <AstroCatalog.h>
#include <AstroChart.h>
#include <AstroIO.h>
#include <stdexcept>

using namespace astro::catalog;
using namespace astro::image::transform;
using namespace astro::io;

namespace astro {

int	main(int argc, char *argv[]) {
	int	c;
	while (EOF != (c = getopt(argc, argv, "d")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		}

	// create a Chart factory
	CatalogPtr 	catalog = CatalogFactory::get(CatalogFactory::Combined,
					"/usr/local/starcatalogs");
	TurbulencePointSpreadFunction   psf(2);
	ChartFactory    factory(catalog, psf, 14, 100);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "chart factory created");

	// create an Image Normalizer
	ImageNormalizer normalizer(factory);

	// prepare the initial transformation
	Projection      projection(M_PI * 162 / 180, Point(838, 182), 0.98);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "projection: %s",
	projection.toString().c_str());

	// get the image from the input file
	FITSin  in("andromeda-base.fits");
	ImagePtr        imageptr = in.read();

	// apply the normalizer to the 
	debug(LOG_DEBUG, DEBUG_LOG, 0, "apply normalizer");
	RaDec   center = normalizer(imageptr, projection);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "true center: %s",
	center.toString().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "transformation: %s",
	projection.toString().c_str());

	return EXIT_SUCCESS;
}

} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::main(argc, argv);
	} catch (std::exception& x) {
		std::cerr << "terminate by exception: " << x.what() << std::endl;
	} catch (...) {
		std::cerr << "terminate by unknown exception" << std::endl;
	}
}
