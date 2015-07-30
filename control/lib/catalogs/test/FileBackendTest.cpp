/*
 * FileBackendTest.cpp -- tests for the UCAC4 catalog 
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

class FileBackendTest : public CppUnit::TestFixture {
private:
	astro::catalog::FileBackend	*catalog;
public:
	void	setUp();
	void	tearDown();
	void	testConstructor();
	void	testAccess();
	void	testIterator();

	CPPUNIT_TEST_SUITE(FileBackendTest);
	CPPUNIT_TEST(testConstructor);
	CPPUNIT_TEST(testAccess);
	CPPUNIT_TEST(testIterator);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(FileBackendTest);

void	FileBackendTest::setUp() {
	catalog = new astro::catalog::FileBackend("/usr/local/starcatalogs");	
}

void	FileBackendTest::tearDown() {
	delete catalog;
	catalog = NULL;
}

void	FileBackendTest::testConstructor() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() begin");
	unsigned long long	n = catalog->numberOfStars();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "catalog has %llu stars", n);
	CPPUNIT_ASSERT(n == 116447057);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() end");
}

void	FileBackendTest::testAccess() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAccess() begin");

	astro::catalog::Star	star1 = catalog->find("BSC1234");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "star1 = '%s'",
		star1.longname().c_str());
	CPPUNIT_ASSERT(star1.catalog() == 'B');
	CPPUNIT_ASSERT(star1.longname() == std::string("BSC1234"));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%llu", star1.catalognumber());
	CPPUNIT_ASSERT(star1.catalognumber() == 1234);

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

void	FileBackendTest::testIterator() {
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

} // namespace test
} // namespace astro
