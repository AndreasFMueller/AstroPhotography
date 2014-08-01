/*
 * SkyWindowTest.cpp -- tests for the SkyWindow catalog 
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCatalog.h>
#include <AstroDebug.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

using namespace astro::catalog;

namespace astro {
namespace test {

class SkyWindowTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testConstructor();
	void	testContains();

	CPPUNIT_TEST_SUITE(SkyWindowTest);
	CPPUNIT_TEST(testConstructor);
	CPPUNIT_TEST(testContains);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SkyWindowTest);

void	SkyWindowTest::setUp() {
}

void	SkyWindowTest::tearDown() {
}

void	SkyWindowTest::testConstructor() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() begin");
	RaDec	center(0, 0);
	center.ra().hours(1);
	center.dec().degrees(60);
	Angle	width; width.hours(3);
	Angle	height; height.degrees(40);
	SkyWindow	window(center, width, height);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() end");
}

void	SkyWindowTest::testContains() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testContains() begin");
	RaDec	center(0, 0);
	center.ra().hours(1);
	center.dec().degrees(60);
	Angle	width; width.hours(3);
	Angle	height; height.degrees(40);
	SkyWindow	window(center, width, height);
	CPPUNIT_ASSERT(window.contains(center));
	RaDec	test;
	test.ra().hours(-0.49); test.dec().degrees(79);
	CPPUNIT_ASSERT(window.contains(test));
	test.ra().hours(-0.49); test.dec().degrees(41);
	CPPUNIT_ASSERT(window.contains(test));
	test.ra().hours(2.49); test.dec().degrees(79);
	CPPUNIT_ASSERT(window.contains(test));
	test.ra().hours(2.49); test.dec().degrees(41);
	CPPUNIT_ASSERT(window.contains(test));
	test.ra().hours(-0.51); test.dec().degrees(81);
	CPPUNIT_ASSERT(!window.contains(test));
	test.ra().hours(-0.51); test.dec().degrees(39);
	CPPUNIT_ASSERT(!window.contains(test));
	test.ra().hours(2.51); test.dec().degrees(81);
	CPPUNIT_ASSERT(!window.contains(test));
	test.ra().hours(2.51); test.dec().degrees(39);
	CPPUNIT_ASSERT(!window.contains(test));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testContains() end");
}

} // namespace test
} // namespace astro
