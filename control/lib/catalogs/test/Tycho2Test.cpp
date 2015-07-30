/*
 * Tycho2Test.cpp -- tests for the Tycho2 catalog 
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "../Tycho2.h"
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
	void	testIterator();
	void	testWindow();

	CPPUNIT_TEST_SUITE(Tycho2Test);
	CPPUNIT_TEST(testConstructor);
	CPPUNIT_TEST(testAccess);
	CPPUNIT_TEST(testIterator);
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

	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieving record 0");
	Tycho2Star	firststar = catalog.find((unsigned int)0);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found first star: %s",
		firststar.name().c_str());
	CPPUNIT_ASSERT(firststar.name() == std::string("T0001 00008 1"));

	debug(LOG_DEBUG, DEBUG_LOG, 0, "getting last star");
	Tycho2Star	laststar = catalog.find(catalog.nstars() - 1);
	CPPUNIT_ASSERT(laststar.name() == std::string("T9537 00387 1"));

	debug(LOG_DEBUG, DEBUG_LOG, 0, "found last star: %s",
		laststar.name().c_str());

	Star	star1 = catalog.find("T9522 00134 1");
	CPPUNIT_ASSERT(star1.longname() == std::string("T9522 00134 1"));
	CPPUNIT_ASSERT(fabs(star1.ra().degrees() - 244.18369114) < 1e-7);
	CPPUNIT_ASSERT(fabs(star1.dec().degrees() - (-82.77286513)) < 1e-7);

	Star	star2 = catalog.find("T4711 00015 1");
	CPPUNIT_ASSERT(star2.longname() == std::string("T4711 00015 1"));
	CPPUNIT_ASSERT(fabs(star2.ra().degrees() - 47.70148132) < 1e-7);
	CPPUNIT_ASSERT(fabs(star2.dec().degrees() - (-2.70543639)) < 1e-7);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAccess() end");
}

void	Tycho2Test::testIterator() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testIterator() begin");
	Tycho2	catalog("/usr/local/starcatalogs/tycho2/tyc2.dat");
	CatalogIterator	i;
	int	counter = 0;
	for (i = catalog.begin(); i != catalog.end(); ++i) {
		counter++;
		if (counter == 100) {
			Star	s = *i;
			CPPUNIT_ASSERT(s.longname()
				== std::string("T0001 00779 1"));
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "iterations: %d", counter);
	CPPUNIT_ASSERT(counter == catalog.numberOfStars());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testIterator() end");
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
	Tycho2::starsetptr	stars = catalog.find(window,
					MagnitudeRange(-30., 4.5));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d stars", stars->size());
	Tycho2::starset::const_iterator	s;
	for (s = stars->begin(); s != stars->end(); s++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", s->toString().c_str());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWindow() end");
}

} // namespace test
} // namespace astro
