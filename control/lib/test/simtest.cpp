/*
 * simtest.cpp -- simulator test
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>
#include <AstroLoader.h>
#include <AstroIO.h>
#include <Simulator.h>
#include <AstroFormat.h>
#include <AstroDebug.h>

using namespace astro::camera::sim;
using namespace astro::io;
using namespace astro::image;

namespace astro {

static int	counter;

void	writeimage(ImagePtr image) {
	std::string	filename = stringprintf("sim%03d.fits", counter);
	unlink(filename.c_str());
	FITSout	out(filename);
	out.write(image);
	counter++;
}

int	main(int argc, char *argv[]) {
	// parse command line arguments
	int	c;
	while (EOF != (c = getopt(argc, argv, "d")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		}

	// create a camera instance
	SimCamera	camera;
	CcdPtr	ccd = camera.getCcd(0);
	GuidePortPtr	guideport = camera.getGuidePort();

	// we will always use 1 sec exposures
	Exposure	exposure(ImageRectangle(ImagePoint(160,120),
		ImageSize(320, 240)), 1);

	// make 10 images 1 second appart (should give small drift)
	counter = 0;
	while (counter < 10) {
		ccd->startExposure(exposure);
		ImagePtr	image = ccd->getImage();
		writeimage(image);
	}

	// now move for 5 seconds
	guideport->activate(5, 0, 0, 0);
	sleep(5);
	ccd->startExposure(exposure);
	writeimage(ccd->getImage());

	guideport->activate(0, 5, 0, 0);
	sleep(5);
	ccd->startExposure(exposure);
	writeimage(ccd->getImage());
	
	guideport->activate(0, 0, 5, 0);
	sleep(5);
	ccd->startExposure(exposure);
	writeimage(ccd->getImage());
	
	guideport->activate(0, 0, 0, 5);
	sleep(5);
	ccd->startExposure(exposure);
	writeimage(ccd->getImage());
	

	return EXIT_SUCCESS;
}

} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::main(argc, argv);
	} catch (std::exception& x) {
		std::cerr << "simtest terminated by exception: " << x.what()
			<< std::endl;
	}
}
