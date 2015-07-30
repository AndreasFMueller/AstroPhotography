/*
 * BSCTest.cpp -- tests for the BSC catalog 
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "../BSC.h"
#include <AstroDebug.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

using namespace astro::catalog;

namespace astro {
namespace test {

class BSCTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testConstructor();
	void	testAccess();
	void	testIterator();
	void	testWindow();

	CPPUNIT_TEST_SUITE(BSCTest);
	CPPUNIT_TEST(testConstructor);
	CPPUNIT_TEST(testAccess);
	CPPUNIT_TEST(testIterator);
	CPPUNIT_TEST(testWindow);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BSCTest);

void	BSCTest::setUp() {
}

void	BSCTest::tearDown() {
}

void	BSCTest::testConstructor() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() begin");
	BSC	catalog("/usr/local/starcatalogs/bsc/catalog",
			"/usr/local/starcatalogs/bsc/notes");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stars in BSC: %u",
		catalog.numberOfStars());
	CPPUNIT_ASSERT(catalog.numberOfStars() == 9096);
	BSC	catalog2("/usr/local/starcatalogs/bsc");
	CPPUNIT_ASSERT(catalog2.numberOfStars() == 9096);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() end");
}

void	BSCTest::testAccess() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAccess() begin");
	BSC	catalog("/usr/local/starcatalogs/bsc/catalog",
			"/usr/local/starcatalogs/bsc/notes");

	BSCStar	star1 = catalog.find(3);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s has long name '%s'",
		star1.name().c_str(), star1.longname().c_str());
	CPPUNIT_ASSERT(star1.longname() == "33    Psc");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", star1.toString().c_str());

	BSCStar	star2 = catalog.find(9103);
	CPPUNIT_ASSERT(star2.longname() == "3    Cet");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", star2.toString().c_str());

	Star	star3 = catalog.find(std::string("BSC4450"));
	CPPUNIT_ASSERT(star3.longname() == "Xi  Hya");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", star3.toString().c_str());

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAccess() end");
}

void	BSCTest::testIterator() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testIterator() begin");
	BSC	catalog("/usr/local/starcatalogs/bsc/catalog",
			"/usr/local/starcatalogs/bsc/notes");
	int	counter = 0;
	CatalogIterator	i;
	for (i = catalog.begin(); i != catalog.end(); ++i) {
		counter++;
		if (counter == 15) {
			Star	s = *i;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "star[15] = %s",
				s.toString().c_str());
			CPPUNIT_ASSERT(s.longname() == std::string("21Alp And"));
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "iterations: %d", counter);
	CPPUNIT_ASSERT(counter == catalog.numberOfStars());
	
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testIterator() end");
}

void	BSCTest::testWindow() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWindow() begin");
	BSC	catalog("/usr/local/starcatalogs/bsc/catalog",
			"/usr/local/starcatalogs/bsc/notes");
	RaDec	center(0, 0);
	center.ra().hours(6.75247702777777777777);
	center.dec().degrees(-16.71611583333333333333);
	Angle	width; width.hours(1);
	Angle	height; height.degrees(15);
	SkyWindow	window(center, width, height);
	BSC::starsetptr	stars = catalog.find(window,
					MagnitudeRange(-30, 4.5));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d stars", stars->size());
	CPPUNIT_ASSERT(stars->size() == 10);
	std::set<Star>::const_iterator	s;
	for (s = stars->begin(); s != stars->end(); s++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", s->toString().c_str());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWindow() end");
}

} // namespace test
} // namespace astro
