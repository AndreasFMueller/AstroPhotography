/*
 * PGCTest.cpp -- tests for the PGC catalog 
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "../PGC.h"
#include <AstroDebug.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

using namespace astro::catalog;

namespace astro {
namespace test {

class PGCTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testConstructor();
	void	testAccess();
	void	testWindow();

	CPPUNIT_TEST_SUITE(PGCTest);
	CPPUNIT_TEST(testConstructor);
	CPPUNIT_TEST(testAccess);
	CPPUNIT_TEST(testWindow);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(PGCTest);

void	PGCTest::setUp() {
}

void	PGCTest::tearDown() {
}

void	PGCTest::testConstructor() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() begin");
	PGC	catalog("/usr/local/starcatalogs/pgc");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() end");
}

void	PGCTest::testAccess() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAccess() begin");
	PGC	catalog("/usr/local/starcatalogs/pgc");
	DeepSkyObject	object1 = catalog.find("PGC0237909");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", object1.toString().c_str());
	DeepSkyObject	object2 = catalog.find("PGC2801111");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", object2.toString().c_str());
	DeepSkyObject	object3 = catalog.find("NGC2403");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", object3.toString().c_str());
	DeepSkyObject	object4 = catalog.find("NGC224");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", object4.toString().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAccess() end");
}

void	PGCTest::testWindow() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWindow() begin");
	PGC	catalog("/usr/local/starcatalogs/pgc");
	RaDec	center(0, 0);
	center.ra().hours(6.75247702777777777777);
	center.dec().degrees(-16.71611583333333333333);
	Angle	width; width.hours(1);
	Angle	height; height.degrees(15);
	SkyWindow	window(center, width, height);
	PGC::objectsetptr	objects = catalog.find(window);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d objects", objects->size());
	CPPUNIT_ASSERT(objects->size() == 1431);
	PGC::objectset::const_iterator	o;
	for (o = objects->begin(); o != objects->end(); o++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", o->toString().c_str());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWindow() end");
}

} // namespace test
} // namespace astro
