/*
 * SimGuidePortTest.cpp -- test the simulated camera
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SimGuidePort.h>
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

class SimGuidePortTest : public CppUnit::TestFixture {
	SimLocator	*locator;
public:
	void	setUp();
	void	tearDown();
	void	testConfig();
	void	testTranslation();
	void	testRotation();

	CPPUNIT_TEST_SUITE(SimGuidePortTest);
	CPPUNIT_TEST(testConfig);
	CPPUNIT_TEST(testTranslation);
	CPPUNIT_TEST(testRotation);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SimGuidePortTest);

void	SimGuidePortTest::setUp() {
	// ensure a consistent state of the random number generator
	srandom(0);
	locator = new SimLocator();
}

void	SimGuidePortTest::tearDown() {
	delete	locator;
}

void	SimGuidePortTest::testConfig() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start GuidePort config test");
	SimGuidePort	*simguideport = locator->simguideport();
	simguideport->drift(Point(1,2));
	CPPUNIT_ASSERT(simguideport->drift() == Point(1,2));
	simguideport->omega(5);
	CPPUNIT_ASSERT(simguideport->omega() == 5);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "end GuidePort config test");
}

void	SimGuidePortTest::testTranslation() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start GuidePort translation test");
	// get and configure the guider port
	SimGuidePort	*simguideport = locator->simguideport();
	simguideport->drift(Point(1,2));

	// get camera and ccd and retrieve an image
	CameraPtr	camera = locator->getCamera("camera:simulator/camera");
	CcdPtr	ccd = camera->getCcd(0);
	Exposure	exposure;
	exposure.exposuretime(1);
	ccd->startExposure(exposure);
	ccd->wait();
	ImagePtr	image = ccd->getImage();
	{
		FITSout	out("guideporttest.fits");
		out.setPrecious(false);
		out.write(image);
	}

	// advance the time and take another image
	simtime_advance(10);
	ccd->startExposure(exposure);
	ccd->wait();
	image = ccd->getImage();
	{
		FITSout	out("guideporttest-translated.fits");
		out.setPrecious(false);
		out.write(image);
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "end GuidePort translation test");
}

void	SimGuidePortTest::testRotation() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start GuidePort rotation test");
	// get and configure the guider port
	SimGuidePort	*simguideport = locator->simguideport();
	simguideport->omega(0.01);

	// get camera and ccd and retrieve an image
	CameraPtr	camera = locator->getCamera("camera:simulator/camera");
	CcdPtr	ccd = camera->getCcd(0);
	Exposure	exposure;
	exposure.exposuretime(1);
	ccd->startExposure(exposure);
	ccd->wait();
	ImagePtr	image = ccd->getImage();

	// advance the time and take another image
	simtime_advance(10);
	ccd->startExposure(exposure);
	ccd->wait();
	image = ccd->getImage();
	{
		FITSout	out("guideporttest-rotated.fits");
		out.setPrecious(false);
		out.write(image);
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "end GuidePort rotation test");
}

} // namespace test
} // namespace simulator
} // namespace camera
} // namespace astro
