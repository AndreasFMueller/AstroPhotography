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
	void	testIterator();
	void	testWindow();
	void	testWindowIterator();
	void	testAll();

	CPPUNIT_TEST_SUITE(HipparcosTest);
	CPPUNIT_TEST(testAccess);
	CPPUNIT_TEST(testIterator);
	CPPUNIT_TEST(testWindow);
	CPPUNIT_TEST(testWindowIterator);
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", firststar.toString().c_str());
	CPPUNIT_ASSERT(fabs(firststar.ra().hours()
				- 0.22/3600) < 1e-7);
	CPPUNIT_ASSERT(fabs(firststar.dec().degrees()
				- (1 + 5./60 + 20.4/3600)) < 1e-6);

	HipparcosStar	laststar = catalog->find(4711);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", laststar.toString().c_str());
	CPPUNIT_ASSERT(fabs(laststar.ra().hours()
				- (1 + 31.63/3600)) < 1e-6);
	CPPUNIT_ASSERT(fabs(laststar.dec().degrees()
				- (-(17 + 4./60 + 36.3/3600))) < 1e-6);

	Star	star = catalog->find(std::string("HIP118322"));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", star.toString().c_str());
	CPPUNIT_ASSERT(fabs(star.ra().hours()
				- (23 + 59./60 + 54.91/3600)) < 1e-7);
	CPPUNIT_ASSERT(fabs(star.dec().degrees()
				- (-(65 + 34./60 + 37.5/3600))) < 1e-7);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAccess() end");
}

void	HipparcosTest::testIterator() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testIterator() begin");
	CatalogIterator	i;
	unsigned long long	counter = 0;
	for (i = catalog->begin(); i != catalog->end(); ++i) {
		counter++;
		if (counter == 4711) {
			Star	s = *i;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "star name: %s",
				s.longname().c_str());
			CPPUNIT_ASSERT(s.longname() == std::string("HIP004721"));
		}
	}
	CPPUNIT_ASSERT(counter == catalog->numberOfStars());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testIterator() end");
}

void	HipparcosTest::testWindow() {
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

void	HipparcosTest::testWindowIterator() {
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
