/*
 * FWHMTest.cpp -- test pixel conversions
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroImage.h>
#include <AstroFWHM.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <AstroDebug.h>

using namespace astro::image;
using namespace astro::image::filter;

namespace astro {
namespace test {

class FWHMTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testFWHM();

	CPPUNIT_TEST_SUITE(FWHMTest);
	CPPUNIT_TEST(testFWHM);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(FWHMTest);

void	FWHMTest::setUp() {
}

void	FWHMTest::tearDown() {
}

void	FWHMTest::testFWHM() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testFWHM() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMin() end");
}


} // namespace test
} // namespace astro
