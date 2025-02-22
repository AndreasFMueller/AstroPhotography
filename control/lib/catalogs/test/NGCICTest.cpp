/*
 * NGCICTest.cpp -- tests for the NGCIC catalog 
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "../NGCIC.h"
#include <AstroDebug.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

using namespace astro::catalog;

namespace astro {
namespace test {

class NGCICTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testConstructor();
	void	testAccess();
	void	testWindow();

	CPPUNIT_TEST_SUITE(NGCICTest);
	CPPUNIT_TEST(testConstructor);
	CPPUNIT_TEST(testAccess);
	CPPUNIT_TEST(testWindow);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(NGCICTest);

void	NGCICTest::setUp() {
}

void	NGCICTest::tearDown() {
}

void	NGCICTest::testConstructor() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() begin");
	NGCIC	catalog("/usr/local/starcatalogs/ngcic/ngc2000.dat");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() end");
}

void	NGCICTest::testAccess() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAccess() begin");
	NGCIC	catalog("/usr/local/starcatalogs/ngcic/ngc2000.dat");
	DeepSkyObject	object = catalog.find("NGC7000");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", object.toString().c_str());
	CPPUNIT_ASSERT(object.constellation == "Cyg");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAccess() end");
}

void	NGCICTest::testWindow() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWindow() begin");
	NGCIC	catalog("/usr/local/starcatalogs/ngcic/ngc2000.dat");
	RaDec	center(0, 0);
	center.ra().hours(6.75247702777777777777);
	center.dec().degrees(-16.71611583333333333333);
	Angle	width; width.hours(1);
	Angle	height; height.degrees(15);
	SkyWindow	window(center, width, height);
	DeepSkyObjectSetPtr	objects = catalog.find(window);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d objects", objects->size());
	CPPUNIT_ASSERT(objects->size() == 27);
	DeepSkyObjectSet::const_iterator	o;
	for (o = objects->begin(); o != objects->end(); o++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", o->toString().c_str());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWindow() end");
}

} // namespace test
} // namespace astro
