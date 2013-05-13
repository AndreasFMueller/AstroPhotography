/*
 * sbigtest.cpp -- class containing unit tests for the SBIG driver
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */

#include <AstroIO.h>
#include <SbigLocator.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <ostream>
#include <debug.h>

using namespace astro::camera::sbig;
using namespace astro::image;
using namespace astro::io;

namespace astro {
namespace camera {
namespace sbig {
namespace test {

class sbigtest : public CppUnit::TestFixture {
	SbigCameraLocator	*locator;
public:
	void	setUp();
	void	tearDown();
	void	testList();
	void	testCamera();
	void	testCcd();
	void	testExposure();
	void	testFilterwheel();
	void	testCooler();
	void	testGuiderport();

	CPPUNIT_TEST_SUITE(sbigtest);
	CPPUNIT_TEST(testList);
	CPPUNIT_TEST(testCamera);
	CPPUNIT_TEST(testCcd);
	//CPPUNIT_TEST(testExposure);
	//CPPUNIT_TEST(testFilterwheel);
	//CPPUNIT_TEST(testCooler);
	CPPUNIT_TEST(testGuiderport);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(sbigtest);

void	sbigtest::setUp() {
	locator = new SbigCameraLocator();
}

void	sbigtest::tearDown() {
	delete locator;
	locator = NULL;
}

void	sbigtest::testList() {
	std::vector<std::string>	cameras = locator->getCameralist();
	int	counter = 0;
	std::vector<std::string>::const_iterator	i;
	for (i = cameras.begin(); i != cameras.end(); i++) {
		std::cout << "camera[" << ++counter << "]: '" << *i << "'" << std::endl;
	}
}

void	sbigtest::testCamera() {
	CameraPtr	camera = locator->getCamera(0);
}

void	sbigtest::testCcd() {
	CameraPtr	camera = locator->getCamera(0);
	CcdPtr	ccd = camera->getCcd(0);
	std::cout << ccd->getInfo() << std::endl;
}

void	sbigtest::testExposure() {
	CameraPtr	camera = locator->getCamera(0);
	CcdPtr	ccd = camera->getCcd(0);
	//Exposure	exposure(ImageRectangle(ImagePoint(80, 50),
	//	ImageSize(500, 400)), 0.02);
	//Exposure	exposure(ImageRectangle(ImagePoint(0, 0),
	//	ImageSize(648, 486)), 0.02);
	//Exposure	exposure(ImageRectangle(ImagePoint(80, 200),
	//	ImageSize(500, 286)), 0.02);
	Exposure	exposure(ImageRectangle(ImagePoint(0, 0),
		ImageSize(4096, 4096)), 0.12);
	std::cout << exposure << std::endl;
	ccd->startExposure(exposure);
	ccd->exposureStatus();
	ImagePtr	image = ccd->getImage();
	// write the image to a file
	unlink("test.fits");
	FITSout	file("test.fits");
	file.write(image);
}

void	sbigtest::testFilterwheel() {
	CameraPtr	camera = locator->getCamera(0);
	CcdPtr	ccd = camera->getCcd(0);
	FilterWheelPtr	filterwheel = camera->getFilterWheel();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filter wheel has %hu positions",
		filterwheel->nFilters());
	unsigned int	currentindex = filterwheel->currentPosition();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "current: %u", currentindex);
	filterwheel->select(currentindex);
	for (unsigned int filterindex = 0;
		filterindex < 4 /* filterwheel->nFilters() */; filterindex++) {
		filterwheel->select(filterindex);
		Exposure	exposure(ImageRectangle(ImagePoint(1500,1500),
			ImageSize(1000,1000)), 0.1);
		ccd->startExposure(exposure);
		ccd->exposureStatus();
		ImagePtr	image = ccd->getImage();
		char	name[128];
		snprintf(name, sizeof(name), "test-pos%02d.fits", filterindex);
		unlink(name);
		FITSout	file(name);
		file.write(image);
	}
}

void	sbigtest::testCooler() {
	CameraPtr	camera = locator->getCamera(0);
	CcdPtr	ccd = camera->getCcd(0);
	CoolerPtr	cooler = ccd->getCooler();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "current temperature: %f",
		cooler->getActualTemperature() - 273.1);
	cooler->setTemperature(273.1);
	cooler->setOn(true);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "current set temperature: %f, "
		"enabled: %s", cooler->getSetTemperature(),
		(cooler->isOn()) ? "YES" : "NO");
	for (int t = 0; t < 60; t++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "current temperature: %f",
			cooler->getActualTemperature() - 273.1);
	}
	cooler->setOn(false);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "current set temperature: %f, "
		"enabled: %s", cooler->getSetTemperature(),
		(cooler->isOn()) ? "YES" : "NO");
	for (int t = 0; t < 60; t++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "current temperature: %f",
			cooler->getActualTemperature() - 273.1);
	}
}

void	sbigtest::testGuiderport() {
	CameraPtr	camera = locator->getCamera(0);
	GuiderPortPtr	guiderport = camera->getGuiderPort();
	guiderport->activate(3, 0, 0, 0);
	for (int t = 0; t < 5; t++) {
		uint8_t	port = guiderport->active();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "active: %02x", port);
		sleep(1);
	}
	guiderport->activate(0, 3, 0, 0);
	for (int t = 0; t < 5; t++) {
		uint8_t	port = guiderport->active();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "active: %02x", port);
		sleep(1);
	}
	guiderport->activate(0, 0, 3, 0);
	for (int t = 0; t < 5; t++) {
		uint8_t	port = guiderport->active();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "active: %02x", port);
		sleep(1);
	}
	guiderport->activate(0, 0, 0, 3);
	for (int t = 0; t < 5; t++) {
		uint8_t	port = guiderport->active();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "active: %02x", port);
		sleep(1);
	}
}

} // namespace test
} // namespace sbig
} // namespace camera
} // namespace astro
