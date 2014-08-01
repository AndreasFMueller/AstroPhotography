/*
 * BSCTest.cpp -- tests for the BSC catalog 
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <BSC.h>
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
	void	testWindow();

	CPPUNIT_TEST_SUITE(BSCTest);
	CPPUNIT_TEST(testConstructor);
	CPPUNIT_TEST(testAccess);
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
	BSC	catalog("/usr/local/bsc/catalog", "/usr/local/bsc/notes");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() end");
}

void	BSCTest::testAccess() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAccess() begin");
	BSC	catalog("/usr/local/bsc/catalog", "/usr/local/bsc/notes");
	BSCStar	firststar = catalog.find(1);
	std::cout << firststar.toString() << std::endl;
	BSCStar	laststar = catalog.find(9110);
	std::cout << laststar.toString() << std::endl;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAccess() end");
}

void	BSCTest::testWindow() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWindow() begin");
	BSC	catalog("/usr/local/bsc/catalog", "/usr/local/bsc/notes");
	RaDec	center(0, 0);
	center.ra().hours(6.75247702777777777777);
	center.dec().degrees(-16.71611583333333333333);
	Angle	width; width.hours(1);
	Angle	height; width.degrees(15);
	SkyWindow	window(center, width, height);
#if 0
	std::set<BSCStar>	stars = catalog.find(window, 6);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d stars", stars.size());
#endif
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWindow() end");
}

} // namespace test
} // namespace astro
