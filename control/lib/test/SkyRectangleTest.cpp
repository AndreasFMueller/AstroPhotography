/*
 * SkyRectangleTest.cpp -- test for the SkyRectangle class
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */

#include <AstroChart.h>
#include <AstroDebug.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

using namespace astro::catalog;

namespace astro {
namespace test {

class SkyRectangleTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testConstructor();
	void	testContains();

	CPPUNIT_TEST_SUITE(SkyRectangleTest);
	CPPUNIT_TEST(testConstructor);
	CPPUNIT_TEST(testContains);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SkyRectangleTest);

void	SkyRectangleTest::setUp() {
}

void	SkyRectangleTest::tearDown() {
}

void	SkyRectangleTest::testConstructor() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() begin");
	RaDec	center;
	center.ra().hours(6);
	center.dec().degrees(45);
	Angle	width; width.hours(1);
	Angle	height; height.degrees(15);
	SkyWindow	window(center, width, height);
	SkyRectangle	rect(window);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() end");
}

void	SkyRectangleTest::testContains() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testContains() begin");
	RaDec	center;
	center.ra().hours(6);
	center.dec().degrees(45);
	Angle	width; width.hours(1);
	Angle	height; height.degrees(15);
	SkyWindow	window(center, width, height);
	SkyRectangle	rect(window);
	CPPUNIT_ASSERT(rect.contains(center));
	center.dec().degrees(52);
	CPPUNIT_ASSERT(rect.contains(center));
	center.dec().degrees(38);
	CPPUNIT_ASSERT(rect.contains(center));
	SkyWindow	w2 = rect.containedin();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "contained in %s",
		w2.toString().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testContains() end");
}

} // namespace test
} // namespace astro
