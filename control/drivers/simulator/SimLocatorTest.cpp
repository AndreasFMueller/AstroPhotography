/*
 * SimLocatorTest.cpp --  a test class for the SimLocator
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SimLocator.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <SimCamera.h>
#include <SimGuiderPort.h>
#include <SimCooler.h>
#include <SimFilterWheel.h>

using namespace astro::device;

namespace astro {
namespace camera {
namespace simulator {
namespace test {

class SimLocatorTest : public CppUnit::TestFixture {
	SimLocator	*locator;
public:
	void	setUp();
	void	tearDown();
	void	testName();
	void	testVersion();
	void	testCameraName();
	void	testFilterWheelName();
	void	testGuiderPortName();
	void	testCoolerName();
	void	testCamera();
	void	testFilterWheel();
	void	testGuiderPort();
	void	testCooler();

	CPPUNIT_TEST_SUITE(SimLocatorTest);
	CPPUNIT_TEST(testName);
	CPPUNIT_TEST(testVersion);
	CPPUNIT_TEST(testCameraName);
	CPPUNIT_TEST(testFilterWheelName);
	CPPUNIT_TEST(testGuiderPortName);
	CPPUNIT_TEST(testCoolerName);
	CPPUNIT_TEST(testCamera);
	CPPUNIT_TEST(testFilterWheel);
	CPPUNIT_TEST(testGuiderPort);
	CPPUNIT_TEST(testCooler);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SimLocatorTest);

void	SimLocatorTest::setUp() {
	locator = new SimLocator();
}

void	SimLocatorTest::tearDown() {
	delete locator;
}

void	SimLocatorTest::testName() {
	CPPUNIT_ASSERT(locator->getName() == std::string("simulator"));
}

void	SimLocatorTest::testVersion() {
	CPPUNIT_ASSERT(locator->getVersion() == VERSION);
}

void	SimLocatorTest::testCameraName() {
	std::vector<std::string>	names
		= locator->getDevicelist(DeviceLocator::CAMERA);
	CPPUNIT_ASSERT(names.size() == 1);
	CPPUNIT_ASSERT(std::string("camera:simulator/camera") == *names.begin());
}

void	SimLocatorTest::testFilterWheelName() {
	std::vector<std::string>	names
		= locator->getDevicelist(DeviceLocator::FILTERWHEEL);
	CPPUNIT_ASSERT(names.size() == 1);
	CPPUNIT_ASSERT(std::string("filterwheel:simulator/filterwheel") == *names.begin());
}

void	SimLocatorTest::testGuiderPortName() {
	std::vector<std::string>	names
		= locator->getDevicelist(DeviceLocator::GUIDERPORT);
	CPPUNIT_ASSERT(names.size() == 1);
	CPPUNIT_ASSERT(std::string("guiderport:simulator/guiderport") == *names.begin());
}

void	SimLocatorTest::testCoolerName() {
	std::vector<std::string>	names
		= locator->getDevicelist(DeviceLocator::COOLER);
	CPPUNIT_ASSERT(names.size() == 1);
	CPPUNIT_ASSERT(std::string("cooler:simulator/cooler") == *names.begin());
}

void	SimLocatorTest::testCamera() {
	SimCamera	*camera
		= dynamic_cast<SimCamera *>(&*(locator->getCamera("camera:simulator/camera")));
	CPPUNIT_ASSERT(NULL != camera);
}

void	SimLocatorTest::testFilterWheel() {
	SimFilterWheel	*filterwheel
		= dynamic_cast<SimFilterWheel*>(&*(locator->getFilterWheel("filterwheel:simulator/filterwheel")));
	CPPUNIT_ASSERT(NULL != filterwheel);
}

void	SimLocatorTest::testGuiderPort() {
	SimGuiderPort	*guiderport
		= dynamic_cast<SimGuiderPort*>(&*(locator->getGuiderPort("guiderport:simulator/guiderport")));
	CPPUNIT_ASSERT(NULL != guiderport);
}

void	SimLocatorTest::testCooler() {
	SimCooler	*cooler
		= dynamic_cast<SimCooler *>(&*(locator->getCooler("cooler:simulator/cooler")));
	CPPUNIT_ASSERT(NULL != cooler);
}

} // namespace test
} // namespace simulator
} // namespace camera
} // namespace astro
