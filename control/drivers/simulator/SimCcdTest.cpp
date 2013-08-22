/*
 * SimCcdTest.cpp -- test the simulated camera
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SimCcd.h>
#include <SimUtil.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace astro::image;

namespace astro {
namespace camera {
namespace simulator {
namespace test {

class SimCcdTest : public CppUnit::TestFixture {
	SimLocator	*locator;
	CameraPtr	camera;
	CcdPtr	ccd;
public:
	void	setUp();
	void	tearDown();
	void	testConfig();
	void	testImage();

	CPPUNIT_TEST_SUITE(SimCcdTest);
	CPPUNIT_TEST(testConfig);
	CPPUNIT_TEST(testImage);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SimCcdTest);

void	SimCcdTest::setUp() {
	SimLocator	*locator = new SimLocator();
	camera = locator->getCamera("sim-camera");
	ccd = camera->getCcd(0);
}

void	SimCcdTest::tearDown() {
	delete	locator;
}

void	SimCcdTest::testConfig() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start Ccd config test");
	CcdInfo	info = ccd->getInfo();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got ccd info: %s", info.name().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got ccd info: %s",
		info.toString().c_str());
	CPPUNIT_ASSERT(info.size() == ImageSize(640, 480));
	CPPUNIT_ASSERT(info.name() == "ccd");
	CPPUNIT_ASSERT(info.modes().permits(Binning(1,1)));
	CPPUNIT_ASSERT(info.modes().permits(Binning(2,2)));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "end Ccd config test");
}

void	SimCcdTest::testImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start image test");
	Exposure	exposure;
	exposure.exposuretime = 1;
	ccd->startExposure(exposure);
	ccd->wait();
	ImagePtr	image = ccd->getImage();
	CPPUNIT_ASSERT(image->size() == ImageSize(640, 480));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "end image test");
}

} // namespace test
} // namespace simulator
} // namespace camera
} // namespace astro
