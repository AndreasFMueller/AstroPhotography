/*
 * HipparcosTest.cpp -- tests for the Hipparcos catalog 
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "../Hipparcos.h"
#include <AstroDebug.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

using namespace astro::catalog;

namespace astro {
namespace test {

class HipparcosTest : public CppUnit::TestFixture {
private:
	Hipparcos	*catalog;
public:
	void	setUp();
	void	tearDown();
	void	testAccess();
	void	testWindow();
	void	testAll();

	CPPUNIT_TEST_SUITE(HipparcosTest);
	CPPUNIT_TEST(testAccess);
	CPPUNIT_TEST(testWindow);
	CPPUNIT_TEST(testAll);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(HipparcosTest);

void	HipparcosTest::setUp() {
	catalog = new Hipparcos("/usr/local/starcatalogs/hipparcos/hip_main.dat");
}

void	HipparcosTest::tearDown() {
	delete catalog;
}

void	HipparcosTest::testAccess() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAccess() begin");
	HipparcosStar	firststar = catalog->find(1);
	std::cout << firststar.toString() << std::endl;
	HipparcosStar	laststar = catalog->find(9110);
	std::cout << laststar.toString() << std::endl;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAccess() end");
}

void	HipparcosTest::testWindow() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWindow() begin");
	RaDec	center(0, 0);
	center.ra().hours(6.75247702777777777777);
	center.dec().degrees(-16.71611583333333333333);
	Angle	width; width.hours(1);
	Angle	height; height.degrees(15);
	SkyWindow	window(center, width, height);
	Hipparcos::starsetptr	stars = catalog->find(window,
					MagnitudeRange(-30, 4.5));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d stars", stars->size());
	CPPUNIT_ASSERT(stars->size() == 10);
	std::set<HipparcosStar>::const_iterator	s;
	for (s = stars->begin(); s != stars->end(); s++) {
		std::cout << s->toString().c_str() << std::endl;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWindow() end");
}

void	HipparcosTest::testAll() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAll() begin");
	Hipparcos::starsetptr	stars = catalog->find(SkyWindow::all,
		MagnitudeRange(-30, 6));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stars with mag<=6: %u", stars->size());
	CPPUNIT_ASSERT(5041 == stars->size());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAll() end");
}

} // namespace test
} // namespace astro
