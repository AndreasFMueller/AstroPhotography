/*
 * Tycho2Test.cpp -- tests for the Tycho2 catalog 
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Tycho2.h>
#include <AstroDebug.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

using namespace astro::catalog;

namespace astro {
namespace test {

class Tycho2Test : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testConstructor();
	void	testAccess();
	void	testWindow();

	CPPUNIT_TEST_SUITE(Tycho2Test);
	CPPUNIT_TEST(testConstructor);
	CPPUNIT_TEST(testAccess);
	CPPUNIT_TEST(testWindow);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Tycho2Test);

void	Tycho2Test::setUp() {
}

void	Tycho2Test::tearDown() {
}

void	Tycho2Test::testConstructor() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() begin");
	Tycho2	catalog("/usr/local/starcatalogs/tycho2/tyc2.dat");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() end");
}

void	Tycho2Test::testAccess() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAccess() begin");
	Tycho2	catalog("/usr/local/starcatalogs/tycho2/tyc2.dat");
	Tycho2Star	firststar = catalog.find(0);
	Tycho2Star	laststar = catalog.find(catalog.nstars() - 1);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAccess() end");
}

void	Tycho2Test::testWindow() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWindow() begin");
	Tycho2	catalog("/usr/local/starcatalogs/tycho2/tyc2.dat");
	RaDec	center(0, 0);
	center.ra().hours(6.75247702777777777777);
	center.dec().degrees(-16.71611583333333333333);
	Angle	width; width.hours(1);
	Angle	height; height.degrees(15);
	SkyWindow	window(center, width, height);
	std::set<Tycho2Star>	stars = catalog.find(window,
						MagnitudeRange(-30., 4.5));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d stars", stars.size());
	std::set<Tycho2Star>::const_iterator	s;
	for (s = stars.begin(); s != stars.end(); s++) {
		std::cout << s->toString() << std::endl;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWindow() end");
}

} // namespace test
} // namespace astro
