/*
 * Ucac4Test.cpp -- tests for the UCAC4 catalog 
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "../Ucac4.h"
#include <AstroDebug.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <unistd.h>

using namespace astro::catalog;

namespace astro {
namespace test {

class Ucac4Test : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testConstructor();
	void	testNumber();
	void	testAccess();
	void	testIterator();
	void	testWindow();

	CPPUNIT_TEST_SUITE(Ucac4Test);
	CPPUNIT_TEST(testConstructor);
	CPPUNIT_TEST(testNumber);
	CPPUNIT_TEST(testAccess);
	CPPUNIT_TEST(testIterator);
	CPPUNIT_TEST(testWindow);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Ucac4Test);

void	Ucac4Test::setUp() {
}

void	Ucac4Test::tearDown() {
}

void	Ucac4Test::testConstructor() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() begin");
	Ucac4	catalog("/usr/local/starcatalogs/u4");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() end");
}

void	Ucac4Test::testNumber() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testNumber() begin");
	Ucac4StarNumber	name1(47, 11);
	CPPUNIT_ASSERT(name1.toString() == std::string("UCAC4-047-000011"));
	Ucac4StarNumber	name2("UCAC4-047-000011");
	CPPUNIT_ASSERT(name1.toString() == name2.toString());
	CPPUNIT_ASSERT(name1 == name2);
	Ucac4StarNumber	name3("UCAC4-047-001100");
	CPPUNIT_ASSERT(name1 != name3);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testNumber() end");
}

void	Ucac4Test::testAccess() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAccess() begin");
	Ucac4	catalog("/usr/local/starcatalogs/u4");
	Ucac4StarNumber	name(47, 11);
	Ucac4Star	star = catalog.find(name);
	CPPUNIT_ASSERT(star.number == name);
	Star	star1 = catalog.find("UCAC4-391-012345");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", star1.toString().c_str());
	CPPUNIT_ASSERT(star1.name() == std::string("UCAC4-391-012345"));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAccess() end");
}

void	Ucac4Test::testIterator() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testIterator() begin");
	Ucac4	catalog("/usr/local/starcatalogs/u4");
	CatalogIterator	i;
	unsigned long long	counter = 0;
	try {
		for (i = catalog.begin(); i != catalog.end(); ++i) {
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
	unsigned long long	n = catalog.numberOfStars();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "iterations: %llu, stars: %llu",
		counter, n);
	CPPUNIT_ASSERT(counter == catalog.numberOfStars());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testIterator() end");
}

void	Ucac4Test::testWindow() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWindow() begin");
	Ucac4	catalog("/usr/local/starcatalogs/u4");
	RaDec   center(0, 0);
	center.ra().hours(6.75247702777777777777);
	center.dec().degrees(-16.71611583333333333333);
	Angle	width; width.hours(1);
	Angle	height; height.degrees(15);
	SkyWindow	window(center, width, height);
	CatalogIterator	i = catalog.findIter(window, MagnitudeRange(-30., 7.5));
	unsigned long long	counter = 0;
	for (; !i.isEnd(); ++i) {
		counter++;
		Star	s = *i;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", s.toString().c_str());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%llu stars", counter);
	CPPUNIT_ASSERT(counter == 88);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWindow() end");
}

} // namespace test
} // namespace astro
