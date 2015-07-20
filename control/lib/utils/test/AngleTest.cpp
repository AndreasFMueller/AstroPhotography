/*
 * AngleTest.cpp -- template for tests
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroCoordinates.h>

namespace astro {
namespace test {

class AngleTest: public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testHours();
	void	testDegrees();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(AngleTest);
	CPPUNIT_TEST(testHours);
	CPPUNIT_TEST(testDegrees);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(AngleTest);

void	AngleTest::setUp() {
}

void	AngleTest::tearDown() {
}

void	AngleTest::testHours() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testHours() begin");
	Angle	a;
	a.hours(17.5);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "hours %f", 
		Angle::hms_to_angle("17.5").hours());
	CPPUNIT_ASSERT(Angle::hms_to_angle("17.5") == a);
	CPPUNIT_ASSERT(Angle::hms_to_angle("17:30") == a);
	a.hours(17.5 + 0.5 / 60);
	CPPUNIT_ASSERT(Angle::hms_to_angle("17:30.5") == a);
	CPPUNIT_ASSERT(Angle::hms_to_angle("17:30:30") == a);
	a.hours(17.5 + 0.5 / 60 + 0.5 / 3600);
	CPPUNIT_ASSERT(Angle::hms_to_angle("17:30:30.5") == a);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testHours() end");
}

void	AngleTest::testDegrees() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDegrees() begin");
	Angle	a;
	a.degrees(17.5);
	CPPUNIT_ASSERT(Angle::dms_to_angle("17.5") == a);
	CPPUNIT_ASSERT(Angle::dms_to_angle("17:30") == a);
	a.degrees(17.5 + 0.5 / 60);
	CPPUNIT_ASSERT(Angle::dms_to_angle("17:30.5") == a);
	CPPUNIT_ASSERT(Angle::dms_to_angle("17:30:30") == a);
	a.degrees(17.5 + 0.5 / 60 + 0.5 / 3600);
	CPPUNIT_ASSERT(Angle::dms_to_angle("17:30:30.5") == a);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDegrees() end");
}

#if 0
void	AngleTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
