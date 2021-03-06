/*
 * SimCoolerTest.cpp -- test the simulated cooler
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SimCooler.h>
#include <SimUtil.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

namespace astro {
namespace camera {
namespace simulator {
namespace test {

class SimCoolerTest : public CppUnit::TestFixture {
	SimLocator	*locator;
public:
	void	setUp();
	void	tearDown();
	void	testName();
	void	testCooler();

	CPPUNIT_TEST_SUITE(SimCoolerTest);
	CPPUNIT_TEST(testName);
	CPPUNIT_TEST(testCooler);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SimCoolerTest);

void	SimCoolerTest::setUp() {
	locator = new SimLocator();
}

void	SimCoolerTest::tearDown() {
	delete locator;
}

void	SimCoolerTest::testName() {
	CoolerPtr	cooler = locator->getCooler("cooler:simulator/cooler");
	CPPUNIT_ASSERT(cooler->name().unitname() == "cooler");
}

void	SimCoolerTest::testCooler() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Start cooler test");
	CoolerPtr	cooler = locator->getCooler("cooler:simulator/cooler");
	Temperature	ambient(13.2, Temperature::CELSIUS);
	CPPUNIT_ASSERT(fabs(ambient - cooler->getActualTemperature()) < 0.01);
	CPPUNIT_ASSERT(cooler->getActualTemperature()
				== cooler->getSetTemperature());
	CPPUNIT_ASSERT(!cooler->isOn());
	cooler->setTemperature(Temperature(260));
	CPPUNIT_ASSERT(cooler->getSetTemperature() == Temperature(260));
	cooler->setOn(true);
	CPPUNIT_ASSERT(cooler->isOn());
	for (int i = 0; i < 10; i++) {
		simtime_advance(1);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "T(%.1f) = %.3f", simtime(),
			cooler->getActualTemperature());
	}
	double	delta = fabs(cooler->getActualTemperature()
		- cooler->getSetTemperature());
	CPPUNIT_ASSERT(delta < 0.2);
	for (int i = 0; i < 10; i++) {
		simtime_advance(1);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "T(%.1f) = %.3f", simtime(),
			cooler->getActualTemperature());
	}
	delta = fabs(cooler->getActualTemperature()
		- cooler->getSetTemperature());
	CPPUNIT_ASSERT(delta < 0.1);
	cooler->setOn(false);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "turning cooler off");
	for (int i = 0; i < 10; i++) {
		simtime_advance(1);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "T(%.1f) = %.3f", simtime(),
			cooler->getActualTemperature());
	}
	delta = fabs(cooler->getActualTemperature() - ambient);
	CPPUNIT_ASSERT(delta < 0.2);
	for (int i = 0; i < 10; i++) {
		simtime_advance(1);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "T(%.1f) = %.3f", simtime(),
			cooler->getActualTemperature());
	}
	delta = fabs(cooler->getActualTemperature() - ambient);
	CPPUNIT_ASSERT(delta < 0.1);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "End cooler test");
}

} // namespace test
} // namespace simulator
} // namespace camera
} // namespace astro
