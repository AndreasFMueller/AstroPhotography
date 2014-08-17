/*
 * StacktraceTest.cpp -- Test the stacktrace handler
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <stacktrace.h>

namespace astro {
namespace test {

class StacktraceTest : public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testStacktrace();

	CPPUNIT_TEST_SUITE(StacktraceTest);
	CPPUNIT_TEST(testStacktrace);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(StacktraceTest);

void	StacktraceTest::setUp() {
}

void	StacktraceTest::tearDown() {
}

void	StacktraceTest::testStacktrace() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testStacktrace() begin");
	stderr_stacktrace(0);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testStacktrace() end");
}

} // namespace test
} // namespace astro
