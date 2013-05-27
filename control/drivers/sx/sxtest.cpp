/*
 * sxtest.cpp -- tests for the SX driver
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SxLocator.h>
#include <AstroIO.h>
#include <AstroFilter.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <ostream>
#include <debug.h>
#include <includes.h>
#include <AstroDemosaic.h>

using namespace astro::image;
using namespace astro::image::filter;
using namespace astro::io;

namespace astro {
namespace camera {
namespace sx {
namespace test {

class sxtest : public CppUnit::TestFixture {
	static SxCameraLocator	*locator;
public:
	void	setUp();
	void	tearDown();
	void	testList();
	void	testCooler();
	void	testCamera();
	void	testFullimage();
	void	testSubimage();
	void	testGuiderport();
	void	testGuiderport2();

	CPPUNIT_TEST_SUITE(sxtest);
	//CPPUNIT_TEST(testList);
	//CPPUNIT_TEST(testCooler);
	//CPPUNIT_TEST(testCamera);
	//CPPUNIT_TEST(testFullimage);
	//CPPUNIT_TEST(testSubimage);
	//CPPUNIT_TEST(testGuiderport);
	CPPUNIT_TEST(testGuiderport2);
	CPPUNIT_TEST_SUITE_END();
};

SxCameraLocator	*sxtest::locator = NULL;

CPPUNIT_TEST_SUITE_REGISTRATION(sxtest);

void	sxtest::setUp() {
	if (NULL == locator) {
		locator = new SxCameraLocator();
	}
}

void	sxtest::tearDown() {
}

void	sxtest::testList() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "devices");
	std::vector<std::string>	cameras = locator->getCameralist();
	int	counter = 0;
	std::vector<std::string>::const_iterator	i;
	for (i = cameras.begin(); i != cameras.end(); i++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "camera[%d]: %s", ++counter,
			i->c_str());
	}
}

void	sxtest::testCooler() {
	std::vector<std::string>	cameras = locator->getCameralist();
	CameraPtr	camera = locator->getCamera(*cameras.begin());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get ccd");
	CcdPtr	ccd = camera->getCcd(0);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got CCD");
	try {
		CoolerPtr	cooler = ccd->getCooler();
		CPPUNIT_ASSERT(cooler->getActualTemperature() > 250);
		float	temp = cooler->getActualTemperature();
		float	newtemp = cooler->getActualTemperature();
		float	targettemperature = 283.1;
		cooler->setTemperature(targettemperature);
		cooler->setOn(true);
		CPPUNIT_ASSERT(cooler->getSetTemperature() == targettemperature);
		for (int time = 0; time < 60; time++) {
			sleep(1);
			float	newtemp = cooler->getActualTemperature();
			debug(LOG_DEBUG, DEBUG_LOG, 0, "target: %.1f, actual: %.1f",
				targettemperature, newtemp);
			if ((fabs(newtemp - targettemperature)) < 0.3) {
				return;
			}
		}
		CPPUNIT_ASSERT(newtemp < temp - 9);
	} catch (std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no cooler? cause: %s", x.what());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cooler test complete");
}

void	sxtest::testCamera() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "devices");
	std::vector<std::string>	cameras = locator->getCameralist();
	int	counter = 0;
	std::vector<std::string>::const_iterator	i;
	for (i = cameras.begin(); i != cameras.end(); i++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "camera[%d]: %s", ++counter,
			i->c_str());
	}
	CameraPtr	camera = locator->getCamera(*cameras.begin());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera has %d ccds", camera->nCcds());
	CcdPtr	ccd = camera->getCcd(0);
	std::cout << ccd->getInfo() << std::endl;
	
	Exposure	exposure(ImageRectangle(ImagePoint(176, 0),
		ImageSize(1040, 1040)), 0.019);
	//exposure.limit = 62000;
	exposure.mode = Binning(1,1);
	ccd->startExposure(exposure);
	ImagePtr	image = ccd->getImage();
	Image<unsigned short>	*shortimage
		= dynamic_cast<Image<unsigned short> *>(&*image);
	CPPUNIT_ASSERT(NULL != shortimage);
	Median<unsigned short>	median;
	unsigned short	m = median(*shortimage);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "median: %hu", m);

	unlink("test.fits");
	FITSout	file("test.fits");
	file.write(image);
}

void	sxtest::testFullimage() {
	CameraPtr	camera = locator->getCamera(0);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera has %d ccds", camera->nCcds());
	CcdPtr	ccd = camera->getCcd(0);
	std::cout << ccd->getInfo() << std::endl;
	
	Exposure	exposure(ccd->getInfo().getFrame(), 3600);
	//exposure.limit = 62000;
	exposure.mode = Binning(1,1);
	ccd->startExposure(exposure);
	ImagePtr	image = ccd->getImage();
	Image<unsigned short>	*shortimage
		= dynamic_cast<Image<unsigned short> *>(&*image);

	unlink("test.fits");
	FITSout	file("test.fits");
	file.write(image);

	// find average value of all pixel planes
	MeanR<unsigned short, double>	R;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "R = %f", R.mean(*shortimage));
	MeanGr<unsigned short, double>	Gr;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Gr = %f", Gr.mean(*shortimage));
	MeanB<unsigned short, double>	B;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "B = %f", B.mean(*shortimage));
	MeanGb<unsigned short, double>	Gb;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Gb = %f", Gb.mean(*shortimage));

	// demosaic the image
	DemosaicBilinear<unsigned short>	demosaicer;
	Image<RGB<unsigned short> >	*demosaiced = demosaicer(*shortimage);
	ImagePtr	demosaicedptr(demosaiced);
	unlink("test-demosaiced.fits");
	FITSout	demosaicedfile("test-demosaiced.fits");
	demosaicedfile.write(demosaicedptr);
}

void	sxtest::testSubimage() {
	CameraPtr	camera = locator->getCamera(0);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera has %d ccds", camera->nCcds());
	CcdPtr	ccd = camera->getCcd(0);
	std::cout << ccd->getInfo() << std::endl;
	
	Exposure	exposure(ImageRectangle(ImagePoint(1450, 808),
		ImageSize(1000, 1000)), 0.04);
	//exposure.limit = 62000;
	exposure.mode = Binning(1,1);
	ccd->startExposure(exposure);
	ImagePtr	image = ccd->getImage();

	unlink("test.fits");
	FITSout	file("test.fits");
	file.write(image);
}


void	sxtest::testGuiderport() {
	CameraPtr	camera = locator->getCamera(0);
	GuiderPortPtr	guiderport = camera->getGuiderPort();
	guiderport->activate(1,0,0,0);
	sleep(2);
	guiderport->activate(1,0,0,0);
	sleep(2);
	guiderport->activate(1,0,0,0);
	sleep(2);
	guiderport->activate(1,0,0,0);
	sleep(2);
	guiderport->activate(0,1,0,0);
	sleep(2);
	guiderport->activate(0,1,0,0);
	sleep(2);
	guiderport->activate(0,1,0,0);
	sleep(2);
	guiderport->activate(0,0,1,0);
	sleep(2);
	guiderport->activate(0,0,1,0);
	sleep(2);
	guiderport->activate(0,0,0,1);
	sleep(3);
}

void	sxtest::testGuiderport2() {
	CameraPtr	camera = locator->getCamera(0);
	GuiderPortPtr	guiderport = camera->getGuiderPort();
	int	counter = 100;
	int	flags = 0;
	float	delta = 0.2;
	while (counter--) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%01x", flags);
		flags = (flags << 1) % 16;
		if (0 == flags) {
			flags = 1;
		}
		float	raplus = delta * ((0x1 & flags) ? 1 : 0);
		float	decplus = delta * ((0x2 & flags) ? 1 : 0);
		float	decminus = delta * ((0x4 & flags) ? 1 : 0);
		float	raminus = delta * ((0x8 & flags) ? 1 : 0);
		guiderport->activate(raplus, raminus, decplus, decminus);
		usleep(2 * delta * 1000000);
	}
}


} // namespace test
} // namespace sx
} // namespace camera
} // namespace astro
