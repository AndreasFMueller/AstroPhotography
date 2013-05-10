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

	CPPUNIT_TEST_SUITE(sxtest);
	CPPUNIT_TEST(testList);
	CPPUNIT_TEST(testCooler);
	CPPUNIT_TEST(testCamera);
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
#if 0
	debug(LOG_DEBUG, DEBUG_LOG, 0, "devices");
	std::vector<std::string>	cameras = locator->getCameralist();
	int	counter = 0;
	std::vector<std::string>::const_iterator	i;
	for (i = cameras.begin(); i != cameras.end(); i++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "camera[%d]: %s", ++counter,
			i->c_str());
	}
#endif
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
	ShortImagePtr	image = ccd->shortImage();
	Median<unsigned short>	median;
	unsigned short	m = median(*image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "median: %hu", m);

	unlink("test.fits");
	FITSoutfile<unsigned short>	file("test.fits");
	file.write(*image);
}

} // namespace test
} // namespace sx
} // namespace camera
} // namespace astro
