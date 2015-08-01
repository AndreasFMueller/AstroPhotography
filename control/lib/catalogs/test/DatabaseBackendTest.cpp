/*
 * DatabaseBackendTest.cpp -- tests for the UCAC4 catalog 
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "../CatalogBackend.h"
#include <AstroDebug.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <unistd.h>

using namespace astro::catalog;

namespace astro {
namespace test {

class DatabaseBackendTest : public CppUnit::TestFixture {
private:
	astro::catalog::DatabaseBackend	*catalog;
public:
	void	setUp();
	void	tearDown();
	void	testConstructor();
	void	testAccess();
	void	testIterator();
	void	testWindow();

	CPPUNIT_TEST_SUITE(DatabaseBackendTest);
	CPPUNIT_TEST(testConstructor);
	CPPUNIT_TEST(testAccess);
	CPPUNIT_TEST(testIterator);
//	CPPUNIT_TEST(testWindow);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(DatabaseBackendTest);

void	DatabaseBackendTest::setUp() {
	catalog = new astro::catalog::DatabaseBackend("stars.db");	
}

void	DatabaseBackendTest::tearDown() {
	delete catalog;
	catalog = NULL;
}

void	DatabaseBackendTest::testConstructor() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() begin");
	unsigned long long	n = catalog->numberOfStars();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "catalog has %llu stars", n);
	//CPPUNIT_ASSERT(n == 116447057);
	CPPUNIT_ASSERT(n == 2557499);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() end");
}

void	DatabaseBackendTest::testAccess() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAccess() begin");

	astro::catalog::Star	star1 = catalog->find("BSC1231");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "star1 = '%s'",
		star1.longname().c_str());
	CPPUNIT_ASSERT(star1.catalog() == 'B');
	CPPUNIT_ASSERT(star1.longname() == std::string("34Gam Eri"));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%llu", star1.catalognumber());
	CPPUNIT_ASSERT(star1.catalognumber() == 1231);

	astro::catalog::Star	star2 = catalog->find("HIP004711");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "star2 = '%s'",
		star2.longname().c_str());
	CPPUNIT_ASSERT(star2.catalog() == 'H');
	CPPUNIT_ASSERT(star2.longname() == std::string("HIP004711"));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "star2 = %llu", star2.catalognumber());
	CPPUNIT_ASSERT(star2.catalognumber() == 4711);

	astro::catalog::Star	star3 = catalog->find("T4711 01111 1");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "star3 = '%s'",
		star3.longname().c_str());
	CPPUNIT_ASSERT(star3.catalog() == 'T');
	CPPUNIT_ASSERT(star3.longname() == std::string("T4711 01111 1"));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "star3 = %llu", star3.catalognumber());
	CPPUNIT_ASSERT(star3.catalognumber() == 4711011111);

	astro::catalog::Star	star4 = catalog->find("UCAC4-391-012345");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "star4 = '%s'",
		star4.longname().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", star4.toString().c_str());
	CPPUNIT_ASSERT(star4.longname() == std::string("UCAC4-391-012345"));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "star4 = %llu", star4.catalognumber());
	CPPUNIT_ASSERT(star4.catalognumber() == 445306681);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAccess() end");
}

void	DatabaseBackendTest::testIterator() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testIterator() begin");
	CatalogIterator	i;
	unsigned long long	counter = 0;
	try {
		for (i = catalog->begin(); i != catalog->end(); ++i) {
			counter++;
			if (0 == (counter % 1000000)) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"number of stars at %s: %llu",
					i.toString().c_str(), counter);
			}
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "final position: %s", 
			i.toString().c_str());
	} catch (const std::exception &x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "exception: %s", x.what());
	}
	unsigned long long	n = catalog->numberOfStars();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "iterations: %llu, stars: %llu",
		counter, n);
	CPPUNIT_ASSERT(counter == catalog->numberOfStars());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testIterator() end");
}

void	DatabaseBackendTest::testWindow() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWindow() begin");
	RaDec	center(0, 0);
	center.ra().hours(6.75247702777777777777);
	center.dec().degrees(-16.71611583333333333333);
	Angle	width; width.hours(1);
	Angle	height; height.degrees(15);
	SkyWindow	window(center, width, height);
	CatalogIterator	i
		= catalog->findIter(window, MagnitudeRange(-30., 4.5));
	unsigned long long	counter = 0;
	for (; !i.isEnd(); ++i) {
		counter++;
		Star    s = *i;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", s.toString().c_str());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%llu stars", counter);
        CPPUNIT_ASSERT(counter == 27);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWindow() end");
}

} // namespace test
} // namespace astro
