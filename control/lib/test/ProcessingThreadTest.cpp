/*
 * ProcessingThreadTest.cpp -- test to verify workings of the ProcessingThread
 *                             class
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <AstroDebug.h>

using namespace astro::image;
using namespace astro::process;

namespace astro {
namespace test {

class PreviewAdapterTest : public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testXXX();

	CPPUNIT_TEST_SUITE(PreviewAdapterTest);
	CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(PreviewAdapterTest);

void	PreviewAdapterTest::setUp() {
}

void	PreviewAdapterTest::tearDown() {
}

void	PreviewAdapterTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}

} // namespace test
} // namespace astro
