/*
 * SimCameraTest.cpp -- test the simulated camera
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SimCamera.h>
#include <SimUtil.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace astro::image;

namespace astro {
namespace camera {
namespace simulator {
namespace test {

class SimCameraTest : public CppUnit::TestFixture {
	SimLocator	*locator;
public:
	void	setUp();
	void	tearDown();
	void	testName();
	void	testConfig();

	CPPUNIT_TEST_SUITE(SimCameraTest);
	CPPUNIT_TEST(testName);
	CPPUNIT_TEST(testConfig);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SimCameraTest);

void	SimCameraTest::setUp() {
	locator = new SimLocator();
}

void	SimCameraTest::tearDown() {
	delete locator;
}

void	SimCameraTest::testName() {
	CameraPtr	camera = locator->getCamera("sim-camera");
	CPPUNIT_ASSERT(camera->getName() == "sim-camera");
}

void	SimCameraTest::testConfig() {
	CameraPtr	camera = locator->getCamera("sim-camera");
	CPPUNIT_ASSERT(camera->nCcds() == 1);
	CcdInfo	info = camera->getCcdInfo(0);
	CPPUNIT_ASSERT(info.size() == ImageSize(640, 480));
	CPPUNIT_ASSERT(info.name() == "ccd");
	CPPUNIT_ASSERT(info.modes().permits(Binning(1,1)));
	CPPUNIT_ASSERT(info.modes().permits(Binning(2,2)));
}

} // namespace test
} // namespace simulator
} // namespace camera
} // namespace astro
