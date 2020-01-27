/*
 * SAOTest.cpp -- tests for the SAO catalog 
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "../SAO.h"
#include <AstroDebug.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

using namespace astro::catalog;

namespace astro {
namespace test {

class SAOTest : public CppUnit::TestFixture {
private:
	SAO	*catalog;
public:
	void	setUp();
	void	tearDown();
	void	testAccess();
	void	testIterator();
	void	testWindow();
	void	testWindowIterator();
	void	testAll();

	CPPUNIT_TEST_SUITE(SAOTest);
	CPPUNIT_TEST(testAccess);
	CPPUNIT_TEST(testIterator);
	CPPUNIT_TEST(testWindow);
	CPPUNIT_TEST(testWindowIterator);
	CPPUNIT_TEST(testAll);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SAOTest);

void	SAOTest::setUp() {
	catalog = new SAO("/usr/local/starcatalogs/sao/sao.dat");
}

void	SAOTest::tearDown() {
	delete catalog;
}

void	SAOTest::testAccess() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAccess() begin");

	SAOStar	firststar = catalog->find(1);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", firststar.toString().c_str());
	CPPUNIT_ASSERT(fabs(firststar.ra().hours()
				- 161.65/3600) < 1e-7);
	CPPUNIT_ASSERT(fabs(firststar.dec().degrees()
				- (82 + 58./60 + 23.52/3600)) < 1e-6);

	SAOStar	laststar = catalog->find(4711);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", laststar.toString().c_str());
	CPPUNIT_ASSERT(fabs(laststar.ra().hours()
				- ( 2 + 44./60 + 33.919/3600)) < 1e-6);
	CPPUNIT_ASSERT(fabs(laststar.dec().degrees()
				- (80 + 8./60 + 29.95/3600)) < 1e-6);

	Star	star = catalog->find(std::string("SAO111111"));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", star.toString().c_str());
	CPPUNIT_ASSERT(fabs(star.ra().hours()
				- ( 3 + 18./60 + 24.115/3600)) < 1e-7);
	CPPUNIT_ASSERT(fabs(star.dec().degrees()
				- ( 5 + 56./60 + 40.44/3600)) < 1e-7);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAccess() end");
}

void	SAOTest::testIterator() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testIterator() begin");
	CatalogIterator	i;
	unsigned long long	counter = 0;
	for (i = catalog->begin(); i != catalog->end(); ++i) {
		counter++;
		if (counter == 4711) {
			Star	s = *i;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "star name: %s",
				s.longname().c_str());
			CPPUNIT_ASSERT(s.name() == std::string("SAO004711"));
		}
	}
	CPPUNIT_ASSERT(counter == catalog->numberOfStars());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testIterator() end");
}

void	SAOTest::testWindow() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWindow() begin");
	RaDec	center(0, 0);
	center.ra().hours(6.75247702777777777777);
	center.dec().degrees(-16.71611583333333333333);
	Angle	width; width.hours(1);
	Angle	height; height.degrees(15);
	SkyWindow	window(center, width, height);
	Catalog::starsetptr	stars = catalog->find(window,
					MagnitudeRange(-30, 4.5));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d stars", stars->size());
	CPPUNIT_ASSERT(stars->size() == 10);
	Catalog::starset::const_iterator	s;
	for (s = stars->begin(); s != stars->end(); s++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", s->toString().c_str());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWindow() end");
}

void	SAOTest::testWindowIterator() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWindowIterator() begin");
	RaDec	center(0, 0);
	center.ra().hours(6.75247702777777777777);
	center.dec().degrees(-16.71611583333333333333);
	Angle	width; width.hours(1);
	Angle	height; height.degrees(15);
	SkyWindow	window(center, width, height);
	CatalogIterator	i = catalog->findIter(window,
					MagnitudeRange(-30, 4.5));
	unsigned long long	counter = 0;
	for (; !i.isEnd(); ++i) {
		counter++;
		Star	s = *i;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", s.toString().c_str());
	}
	CPPUNIT_ASSERT(counter == 10);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWindowIterator() end");
}

void	SAOTest::testAll() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAll() begin");
	SAO::starsetptr	stars = catalog->find(SkyWindow::all,
		MagnitudeRange(-30, 6));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stars with mag<=6: %u", stars->size());
	CPPUNIT_ASSERT(5103 == stars->size());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAll() end");
}

} // namespace test
} // namespace astro
