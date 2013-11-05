/*
 * SimGuiderPortTest.cpp -- test the simulated camera
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SimGuiderPort.h>
#include <SimUtil.h>
#include <SimLocator.h>
#include <AstroIO.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace astro::image;
using namespace astro::io;

namespace astro {
namespace camera {
namespace simulator {
namespace test {

class SimGuiderPortTest : public CppUnit::TestFixture {
	SimLocator	*locator;
public:
	void	setUp();
	void	tearDown();
	void	testConfig();
	void	testTranslation();
	void	testRotation();

	CPPUNIT_TEST_SUITE(SimGuiderPortTest);
	CPPUNIT_TEST(testConfig);
	CPPUNIT_TEST(testTranslation);
	CPPUNIT_TEST(testRotation);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SimGuiderPortTest);

void	SimGuiderPortTest::setUp() {
	// ensure a consistent state of the random number generator
	srandom(0);
	locator = new SimLocator();
}

void	SimGuiderPortTest::tearDown() {
	delete	locator;
}

void	SimGuiderPortTest::testConfig() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start GuiderPort config test");
	SimGuiderPort	*simguiderport = locator->simguiderport();
	simguiderport->drift(Point(1,2));
	CPPUNIT_ASSERT(simguiderport->drift() == Point(1,2));
	simguiderport->omega(5);
	CPPUNIT_ASSERT(simguiderport->omega() == 5);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "end GuiderPort config test");
}

void	SimGuiderPortTest::testTranslation() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start GuiderPort translation test");
	// get and configure the guider port
	SimGuiderPort	*simguiderport = locator->simguiderport();
	simguiderport->drift(Point(1,2));

	// get camera and ccd and retrieve an image
	CameraPtr	camera = locator->getCamera("camera:simulator/camera");
	CcdPtr	ccd = camera->getCcd(0);
	Exposure	exposure;
	exposure.exposuretime = 1;
	ccd->startExposure(exposure);
	ccd->wait();
	ImagePtr	image = ccd->getImage();
	{
		FITSout	out("guiderporttest.fits");
		out.setPrecious(false);
		out.write(image);
	}

	// advance the time and take another image
	simtime_advance(10);
	ccd->startExposure(exposure);
	ccd->wait();
	image = ccd->getImage();
	{
		FITSout	out("guiderporttest-translated.fits");
		out.setPrecious(false);
		out.write(image);
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "end GuiderPort translation test");
}

void	SimGuiderPortTest::testRotation() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start GuiderPort rotation test");
	// get and configure the guider port
	SimGuiderPort	*simguiderport = locator->simguiderport();
	simguiderport->omega(0.01);

	// get camera and ccd and retrieve an image
	CameraPtr	camera = locator->getCamera("camera:simulator/camera");
	CcdPtr	ccd = camera->getCcd(0);
	Exposure	exposure;
	exposure.exposuretime = 1;
	ccd->startExposure(exposure);
	ccd->wait();
	ImagePtr	image = ccd->getImage();

	// advance the time and take another image
	simtime_advance(10);
	ccd->startExposure(exposure);
	ccd->wait();
	image = ccd->getImage();
	{
		FITSout	out("guiderporttest-rotated.fits");
		out.setPrecious(false);
		out.write(image);
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "end GuiderPort rotation test");
}

} // namespace test
} // namespace simulator
} // namespace camera
} // namespace astro
