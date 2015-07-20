/*
 * TrimTest.cpp -- template for tests
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroUtils.h>

namespace astro {
namespace test {

class TrimTest: public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testRtrim();
	void	testLtrim();
	void	testTrim();

	CPPUNIT_TEST_SUITE(TrimTest);
	CPPUNIT_TEST(testRtrim);
	CPPUNIT_TEST(testLtrim);
	CPPUNIT_TEST(testTrim);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TrimTest);

void	TrimTest::setUp() {
}

void	TrimTest::tearDown() {
}

void	TrimTest::testTrim() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testTrim() begin");
	std::string	s("\n\t blubber \t\n");
	CPPUNIT_ASSERT(trim(s) == "blubber");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testTrim() end");
}

void	TrimTest::testRtrim() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRtrim() begin");
	std::string	s("\n\t blubber \t\n");
	CPPUNIT_ASSERT(rtrim(s) == "\n\t blubber");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRtrim() end");
}

void	TrimTest::testLtrim() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testLtrim() begin");
	std::string	s("\n\t blubber \t\n");
	CPPUNIT_ASSERT(ltrim(s) == "blubber \t\n");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testLtrim() end");
}

} // namespace test
} // namespace astro
